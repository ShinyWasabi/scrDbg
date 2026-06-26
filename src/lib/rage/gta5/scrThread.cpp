#include "scrThread.hpp"
#include "ResourceLoader.hpp"
#include "debugger/VMLogger.hpp"
#include "game/GTA5.hpp"
#include "rage/shared/atArray.hpp"
#include "rage/shared/scrNativeContext.hpp"
#include "rage/shared/scrString.hpp"
#include "scrNativeRegistration.hpp"
#include "scrProgram.hpp"

#if defined(_M_X64)

namespace rage::gta5
{
    using namespace scrDbgLib;
    using namespace scrDbgShared;

    static std::string FormatNativeTypes(scrValue value, NativesBin::NativeTypes type)
    {
        switch (type)
        {
        case NativesBin::NativeTypes::INT:
            return std::to_string(value.Int);
        case NativesBin::NativeTypes::BOOL:
            return value.Int ? "TRUE" : "FALSE";
        case NativesBin::NativeTypes::FLOAT:
            return std::to_string(value.Float);
        case NativesBin::NativeTypes::STRING:
            return value.String ? "\"" + std::string(value.String) + "\"" : "NULL";
        case NativesBin::NativeTypes::REFERENCE:
            return "0x" + std::to_string(reinterpret_cast<uintptr_t>(value.Reference));
        }

        return std::to_string(value.Any);
    }

    template <typename T>
    T GetTLSInternal()
    {
        T tls{};

        if constexpr (std::is_same_v<T, scrThreadGEN8::TLS>)
        {
            tls.m_CurrentThread = reinterpret_cast<scrThreadGEN8**>(*reinterpret_cast<uintptr_t*>(__readgsqword(0x58)) + 0x2A50);
            tls.m_CurrentThreadActive = reinterpret_cast<bool*>(*reinterpret_cast<uintptr_t*>(__readgsqword(0x58)) + 0x2A58);
        }
        else
        {
            tls.m_CurrentThread = reinterpret_cast<scrThreadGEN9**>(*reinterpret_cast<uintptr_t*>(__readgsqword(0x58)) + 0x7A0);
            tls.m_CurrentThreadActive = reinterpret_cast<bool*>(*reinterpret_cast<uintptr_t*>(__readgsqword(0x58)) + 0x7A8);
        }

        return tls;
    }

    template <typename T>
    T GetByHashInternal(uint32_t hash)
    {
        auto threads = []() {
            if constexpr (std::is_same_v<T, scrThreadGEN8*>)
                return GTA5::GetPointers().ScriptThreadsGEN8;
            else
                return GTA5::GetPointers().ScriptThreadsGEN9;
        }();

        if (!threads)
            return nullptr;

        for (auto& thread : *threads)
        {
            if (thread && thread->m_Context.m_Id != 0 && thread->m_ScriptHash == hash)
                return thread;
        }

        return nullptr;
    }

    template <typename T>
    scrThreadState OnExceptionInternal(uint32_t pc, scrOpcode op, const char* fmt, va_list args)
    {
        auto thread = *T::GetTLS().m_CurrentThread;
        if (!thread)
            return scrThreadState::KILLED;

        char message[512];
        std::vsnprintf(message, sizeof(message), fmt, args);

        char fullMessage[128];
        std::snprintf(fullMessage, sizeof(fullMessage), "Exception in %s at 0x%X (%s)!\nReason: %s",
            thread->m_ScriptName,
            pc,
            static_cast<uint8_t>(op) < 131 ? g_scrOpcodeNames[static_cast<uint8_t>(op)] : "???",
            message);
        MessageBoxA(0, fullMessage, "Exception", MB_OK | MB_ICONERROR);

        return thread->m_Context.m_State = scrThreadState::KILLED;
    }

#define GET_U8 (*++code)
#define GET_U16 ((code += 2), *reinterpret_cast<uint16_t*>(code - 1))
#define GET_S16 ((code += 2), *reinterpret_cast<int16_t*>(code - 1))
#define GET_U24 ((code += 3), *reinterpret_cast<uint32_t*>(code - 3) >> 8)
#define GET_U32 ((code += 4), *reinterpret_cast<uint32_t*>(code - 3))

#define JUMP(idx)                           \
    do                                      \
    {                                       \
        int64_t index = idx;                \
        code = program->GetCode(index) - 1; \
        base = &code[-index];               \
    } while (0)

    template <typename T>
    scrThreadState RunThreadInternal(scrValue* stack, scrValue** globals, scrProgram* program, typename T::Context* context)
    {
        auto frameStart = std::chrono::high_resolution_clock::now();

        char buffer[16];
        uint8_t* code;
        uint8_t* base;

        scrValue* sp = stack + context->m_Sp - 1;
        scrValue* fp = stack + context->m_Fp;

        uint32_t scriptHash = context->m_ProgramHash;
        const char* scriptName = (*T::GetTLS().m_CurrentThread)->m_ScriptName;
        Debugger* debugger = g_Game->GetDebugger();

        JUMP(context->m_Pc);

    NEXT:
        // Update these per opcode start
        context->m_Pc = static_cast<uint32_t>(code - base);
        context->m_Sp = static_cast<int32_t>(sp - stack + 1);
        context->m_Fp = static_cast<int32_t>(fp - stack);

        uint32_t pc = static_cast<uint32_t>(code - base);
        if (debugger->ProcessBreakpoints(scriptHash, pc, (uint32_t*)&context->m_State))
            return context->m_State; // If we do not return here, the VM will end up executing opcodes until the next NATIVE call.

        scrOpcode op = static_cast<scrOpcode>(GET_U8);
        if (debugger->ShouldBreakTracking(static_cast<uint8_t>(op)))
            debugger->BreakTracking();

        switch (op)
        {
        case scrOpcode::NOP:
        {
            JUMP(code - base);
            goto NEXT;
        }
        case scrOpcode::IADD:
        {
            --sp;
            sp[0].Int += sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::ISUB:
        {
            --sp;
            sp[0].Int -= sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::IMUL:
        {
            --sp;
            sp[0].Int *= sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::IDIV:
        {
            --sp;
            if (sp[1].Int)
                sp[0].Int /= sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::IMOD:
        {
            --sp;
            if (sp[1].Int)
                sp[0].Int %= sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::INOT:
        {
            sp[0].Int = sp[0].Int == 0;
            goto NEXT;
        }
        case scrOpcode::INEG:
        {
            sp[0].Int = -sp[0].Int;
            goto NEXT;
        }
        case scrOpcode::IEQ:
        {
            --sp;
            sp[0].Int = sp[0].Int == sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::INE:
        {
            --sp;
            sp[0].Int = sp[0].Int != sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::IGT:
        {
            --sp;
            sp[0].Int = sp[0].Int > sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::IGE:
        {
            --sp;
            sp[0].Int = sp[0].Int >= sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::ILT:
        {
            --sp;
            sp[0].Int = sp[0].Int < sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::ILE:
        {
            --sp;
            sp[0].Int = sp[0].Int <= sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::FADD:
        {
            --sp;
            sp[0].Float += sp[1].Float;
            goto NEXT;
        }
        case scrOpcode::FSUB:
        {
            --sp;
            sp[0].Float -= sp[1].Float;
            goto NEXT;
        }
        case scrOpcode::FMUL:
        {
            --sp;
            sp[0].Float *= sp[1].Float;
            goto NEXT;
        }
        case scrOpcode::FDIV:
        {
            --sp;
            if (sp[1].Int)
                sp[0].Float /= sp[1].Float;
            goto NEXT;
        }
        case scrOpcode::FMOD:
        {
            --sp;
            if (sp[1].Int)
            {
                float x = sp[0].Float;
                float y = sp[1].Float;
                sp[0].Float = y ? x - (static_cast<int32_t>(x / y) * y) : 0.0f;
            }
            goto NEXT;
        }
        case scrOpcode::FNEG:
        {
            sp[0].Uns ^= 0x80000000;
            goto NEXT;
        }
        case scrOpcode::FEQ:
        {
            --sp;
            sp[0].Int = sp[0].Float == sp[1].Float;
            goto NEXT;
        }
        case scrOpcode::FNE:
        {
            --sp;
            sp[0].Int = sp[0].Float != sp[1].Float;
            goto NEXT;
        }
        case scrOpcode::FGT:
        {
            --sp;
            sp[0].Int = sp[0].Float > sp[1].Float;
            goto NEXT;
        }
        case scrOpcode::FGE:
        {
            --sp;
            sp[0].Int = sp[0].Float >= sp[1].Float;
            goto NEXT;
        }
        case scrOpcode::FLT:
        {
            --sp;
            sp[0].Int = sp[0].Float < sp[1].Float;
            goto NEXT;
        }
        case scrOpcode::FLE:
        {
            --sp;
            sp[0].Int = sp[0].Float <= sp[1].Float;
            goto NEXT;
        }
        case scrOpcode::VADD:
        {
            sp -= 3;
            sp[-2].Float += sp[1].Float;
            sp[-1].Float += sp[2].Float;
            sp[0].Float += sp[3].Float;
            goto NEXT;
        }
        case scrOpcode::VSUB:
        {
            sp -= 3;
            sp[-2].Float -= sp[1].Float;
            sp[-1].Float -= sp[2].Float;
            sp[0].Float -= sp[3].Float;
            goto NEXT;
        }
        case scrOpcode::VMUL:
        {
            sp -= 3;
            sp[-2].Float *= sp[1].Float;
            sp[-1].Float *= sp[2].Float;
            sp[0].Float *= sp[3].Float;
            goto NEXT;
        }
        case scrOpcode::VDIV:
        {
            sp -= 3;
            if (sp[1].Int)
                sp[-2].Float /= sp[1].Float;
            if (sp[2].Int)
                sp[-1].Float /= sp[2].Float;
            if (sp[3].Int)
                sp[0].Float /= sp[3].Float;
            goto NEXT;
        }
        case scrOpcode::VNEG:
        {
            sp[-2].Uns ^= 0x80000000;
            sp[-1].Uns ^= 0x80000000;
            sp[0].Uns ^= 0x80000000;
            goto NEXT;
        }
        case scrOpcode::IAND:
        {
            --sp;
            sp[0].Int &= sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::IOR:
        {
            --sp;
            sp[0].Int |= sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::IXOR:
        {
            --sp;
            sp[0].Int ^= sp[1].Int;
            goto NEXT;
        }
        case scrOpcode::I2F:
        {
            sp[0].Float = static_cast<float>(sp[0].Int);
            goto NEXT;
        }
        case scrOpcode::F2I:
        {
            sp[0].Int = static_cast<int32_t>(sp[0].Float);
            goto NEXT;
        }
        case scrOpcode::F2V:
        {
            sp += 2;
            sp[-1].Int = sp[0].Int = sp[-2].Int;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_U8:
        {
            ++sp;
            sp[0].Int = GET_U8;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_U8_U8:
        {
            sp += 2;
            sp[-1].Int = GET_U8;
            sp[0].Int = GET_U8;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_U8_U8_U8:
        {
            sp += 3;
            sp[-2].Int = GET_U8;
            sp[-1].Int = GET_U8;
            sp[0].Int = GET_U8;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_U32:
        case scrOpcode::PUSH_CONST_F:
        {
            ++sp;
            sp[0].Uns = GET_U32;
            goto NEXT;
        }
        case scrOpcode::DUP:
        {
            ++sp;
            sp[0].Any = sp[-1].Any;
            goto NEXT;
        }
        case scrOpcode::DROP:
        {
            --sp;
            goto NEXT;
        }
        case scrOpcode::NATIVE:
        {
            uint8_t native = GET_U8;
            int32_t argCount = (native >> 2) & 0x3F;
            int32_t retCount = native & 3;

            uint8_t high = GET_U8;
            uint8_t low = GET_U8;
            scrNativeContext::Handler handler = reinterpret_cast<scrNativeContext::Handler>(program->m_Natives[(high << 8) | low]);

            context->m_Pc = static_cast<int32_t>(code - base - 4);
            context->m_Sp = static_cast<int32_t>(sp - stack + 1);
            context->m_Fp = static_cast<int32_t>(fp - stack);

            scrNativeContext ctx{};
            ctx.m_Rets = retCount ? &stack[context->m_Sp - argCount] : nullptr;
            ctx.m_ArgCount = argCount;
            ctx.m_Args = &stack[context->m_Sp - argCount];
            ctx.m_VectorRefCount = 0;

            uint64_t hash{};
            std::string_view name{};
            std::string argsStr{};
            std::string retsStr{};
            bool shouldLog = VMLogger::ShouldLog(VMLogType::NATIVE_CALLS, scriptHash);

            // log args before calling the native because rets will be written to the same stack slot
            if (shouldLog)
            {
                // cache these here so we can use them when when logging rets as well
                hash = GTA5::GetPointers().NativeRegistrationTable->GetHashByHandler(handler);
                name = NativesBin::GetNameByHash(hash);

                auto args = NativesBin::GetArgsByHash(hash);
                if (argCount > 0 && ctx.m_Args && args && !args->empty())
                {
                    for (int32_t i = 0; i < argCount; i++)
                    {
                        auto type = (i < args->size()) ? (*args)[i] : NativesBin::NativeTypes::NONE;
                        argsStr += FormatNativeTypes(ctx.m_Args[i], type);

                        if (i < argCount - 1)
                            argsStr += ", ";
                    }
                }
            }

            (*handler)(&ctx);

            if (shouldLog)
            {
                auto rets = NativesBin::GetRetsByHash(hash);
                if (retCount > 0 && ctx.m_Rets && rets && !rets->empty())
                {
                    retsStr += " -> (";
                    for (int32_t i = 0; i < retCount; i++)
                    {
                        auto type = (i < rets->size()) ? (*rets)[i] : NativesBin::NativeTypes::NONE;
                        retsStr += FormatNativeTypes(ctx.m_Rets[i], type);

                        if (i < retCount - 1)
                            retsStr += ", ";
                    }
                    retsStr += ")";
                }

                // now log the entire thing
                if (!name.empty())
                    VMLogger::Logf("[%s+0x%08X] %s(%s)%s", scriptName, pc, name.data(), argsStr.c_str(), retsStr.c_str());
                else
                    VMLogger::Logf("[%s+0x%08X] unk_0x%016llX(%s)%s", scriptName, pc, hash, argsStr.c_str(), retsStr.c_str());
            }

            if (context->m_State != scrThreadState::RUNNING)
            {
                if (VMLogger::ShouldLog(VMLogType::FRAME_TIME, scriptHash))
                {
                    auto frameEnd = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
                    VMLogger::Logf("[%s] %.3f ms", scriptName, duration);
                }

                return context->m_State;
            }

            ctx.CopyVectorsOut();

            sp -= argCount - retCount;
            goto NEXT;
        }
        case scrOpcode::ENTER:
        {
            uint32_t argCount = GET_U8;
            uint32_t frameSize = GET_U16;
            uint32_t nameSize = GET_U8;

            if (context->m_CallDepth < 16)
                context->m_CallStack[context->m_CallDepth] = static_cast<int32_t>(code - base + 2);
            ++context->m_CallDepth;

            code += nameSize;

            uint32_t current = static_cast<uint32_t>(sp - stack);
            uint32_t max = context->m_StackSize - frameSize;
            if (current >= max)
                return T::OnException(pc, op, "Stack overflow: %u >= %u", current, max);

            uint32_t callSitePc = sp[0].Uns - 4;

            (++sp)->Int = static_cast<int32_t>(fp - stack);

            fp = sp - argCount - 1;

            while (frameSize--)
                (++sp)->Any = 0;

            sp -= argCount;

            if (VMLogger::ShouldLog(VMLogType::FUNCTION_CALLS, scriptHash))
            {
                auto funcName = program->GetFuncName(pc, nameSize);

                std::string argsStr = "";
                for (int i = 0; i < argCount; i++)
                {
                    argsStr += std::to_string((fp + 1 + i)->Int);
                    if (i != argCount - 1)
                        argsStr += ", ";
                }

                VMLogger::Logf("[%s+0x%08X] %s(%s)", scriptName, callSitePc, funcName.c_str(), argsStr.c_str());
            }

            goto NEXT;
        }
        case scrOpcode::LEAVE: // TO-DO: Log function returns as well
        {
            --context->m_CallDepth;

            uint32_t argCount = GET_U8;
            uint32_t retCount = GET_U8;

            scrValue* ret = sp - retCount;
            sp = fp + argCount - 1;
            fp = stack + sp[2].Uns;

            uint32_t caller = sp[1].Uns;
            JUMP(caller);
            sp -= argCount;

            while (retCount--)
                *++sp = *++ret;

            // Script reached end of code
            if (!caller)
                return context->m_State = scrThreadState::KILLED;

            goto NEXT;
        }
        case scrOpcode::LOAD:
        {
            sp[0].Any = sp[0].Reference->Any;
            goto NEXT;
        }
        case scrOpcode::STORE:
        {
            sp -= 2;
            debugger->FinalizeTracking(scriptName, pc, sp[1].Int);
            sp[2].Reference->Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::STORE_REV:
        {
            --sp;
            sp[0].Reference->Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::LOAD_N:
        {
            scrValue* data = (sp--)->Reference;
            uint32_t itemCount = (sp--)->Int;
            for (uint32_t i = 0; i < itemCount; i++)
                (++sp)->Any = data[i].Any;
            goto NEXT;
        }
        case scrOpcode::STORE_N:
        {
            scrValue* data = (sp--)->Reference;
            uint32_t itemCount = (sp--)->Int;

            debugger->FinalizeTracking(scriptName, pc, itemCount, true);
            for (uint32_t i = 0; i < itemCount; i++)
                data[itemCount - 1 - i].Any = (sp--)->Any;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U8:
        {
            --sp;
            scrValue* ref = sp[1].Reference;
            uint32_t index = sp[0].Uns;
            if (index >= ref->Uns)
                return T::OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);

            uint8_t size = GET_U8;
            debugger->AddArrayIndex(index, size);

            ref += 1U + index * size;
            sp[0].Reference = ref;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U8_LOAD:
        {
            --sp;
            scrValue* ref = sp[1].Reference;
            uint32_t index = sp[0].Uns;
            if (index >= ref->Uns)
                return T::OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_U8;
            sp[0].Any = ref->Any;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U8_STORE:
        {
            sp -= 3;
            scrValue* ref = sp[3].Reference;
            uint32_t index = sp[2].Uns;
            if (index >= ref->Uns)
                return T::OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);

            uint8_t size = GET_U8;
            debugger->AddArrayIndex(index, size);
            debugger->FinalizeTracking(scriptName, pc, sp[1].Int);

            ref += 1U + index * size;
            ref->Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U8:
        {
            ++sp;
            sp[0].Reference = fp + GET_U8;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U8_LOAD:
        {
            ++sp;
            sp[0].Any = fp[GET_U8].Any;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U8_STORE:
        {
            --sp;
            fp[GET_U8].Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::STATIC_U8:
        {
            ++sp;
            uint8_t offset = GET_U8;
            debugger->BeginTracking(scriptHash, offset, false);
            sp[0].Reference = stack + offset;
            goto NEXT;
        }
        case scrOpcode::STATIC_U8_LOAD:
        {
            ++sp;
            sp[0].Any = stack[GET_U8].Any;
            goto NEXT;
        }
        case scrOpcode::STATIC_U8_STORE:
        {
            --sp;
            uint8_t _static = GET_U8;

            if (VMLogger::ShouldLog(VMLogType::STATIC_WRITES, scriptHash))
                VMLogger::Logf("[%s+0x%08X] Static_%u = %d", scriptName, pc, _static, sp[1].Int);

            stack[_static].Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::IADD_U8:
        {
            sp[0].Int += GET_U8;
            goto NEXT;
        }
        case scrOpcode::IMUL_U8:
        {
            sp[0].Int *= GET_U8;
            goto NEXT;
        }
        case scrOpcode::IOFFSET:
        {
            --sp;
            sp[0].Any += sp[1].Int * sizeof(scrValue);
            goto NEXT;
        }
        case scrOpcode::IOFFSET_U8:
        {
            uint8_t offset = GET_U8;
            debugger->AddFieldOffset(offset);
            sp[0].Any += offset * sizeof(scrValue);
            goto NEXT;
        }
        case scrOpcode::IOFFSET_U8_LOAD:
        {
            sp[0].Any = sp[0].Reference[GET_U8].Any;
            goto NEXT;
        }
        case scrOpcode::IOFFSET_U8_STORE:
        {
            sp -= 2;
            uint8_t offset = GET_U8;
            debugger->AddFieldOffset(offset);
            debugger->FinalizeTracking(scriptName, pc, sp[1].Int);
            sp[2].Reference[offset].Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_S16:
        {
            ++sp;
            int16_t offset = GET_S16;
            debugger->AddFieldOffset(offset);
            sp[0].Int = offset;
            goto NEXT;
        }
        case scrOpcode::IADD_S16:
        {
            sp[0].Int += GET_S16;
            goto NEXT;
        }
        case scrOpcode::IMUL_S16:
        {
            sp[0].Int *= GET_S16;
            goto NEXT;
        }
        case scrOpcode::IOFFSET_S16:
        {
            int16_t offset = GET_S16;
            debugger->AddFieldOffset(offset);
            sp[0].Any += offset * sizeof(scrValue);
            goto NEXT;
        }
        case scrOpcode::IOFFSET_S16_LOAD:
        {
            sp[0].Any = sp[0].Reference[GET_S16].Any;
            goto NEXT;
        }
        case scrOpcode::IOFFSET_S16_STORE:
        {
            sp -= 2;
            int16_t offset = GET_S16;
            debugger->AddFieldOffset(offset);
            debugger->FinalizeTracking(scriptName, pc, sp[1].Int);
            sp[2].Reference[offset].Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U16:
        {
            --sp;
            scrValue* ref = sp[1].Reference;
            uint32_t index = sp[0].Uns;
            if (index >= ref->Uns)
                return T::OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);

            uint16_t size = GET_U16;
            debugger->AddArrayIndex(index, size);

            ref += 1U + index * size;
            sp[0].Reference = ref;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U16_LOAD:
        {
            --sp;
            scrValue* ref = sp[1].Reference;
            uint32_t index = sp[0].Uns;
            if (index >= ref->Uns)
                return T::OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_U16;
            sp[0].Any = ref->Any;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U16_STORE:
        {
            sp -= 3;
            scrValue* ref = sp[3].Reference;
            uint32_t index = sp[2].Uns;
            if (index >= ref->Uns)
                return T::OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);

            uint16_t size = GET_U16;
            debugger->AddArrayIndex(index, size);
            debugger->FinalizeTracking(scriptName, pc, sp[1].Int);

            ref += 1U + index * size;
            ref->Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U16:
        {
            ++sp;
            sp[0].Reference = fp + GET_U16;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U16_LOAD:
        {
            ++sp;
            sp[0].Any = fp[GET_U16].Any;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U16_STORE:
        {
            --sp;
            fp[GET_U16].Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::STATIC_U16:
        {
            ++sp;
            uint16_t offset = GET_U16;
            debugger->BeginTracking(scriptHash, offset, false);
            sp[0].Reference = stack + offset;
            goto NEXT;
        }
        case scrOpcode::STATIC_U16_LOAD:
        {
            ++sp;
            sp[0].Any = stack[GET_U16].Any;
            goto NEXT;
        }
        case scrOpcode::STATIC_U16_STORE:
        {
            --sp;
            uint16_t _static = GET_U16;

            if (VMLogger::ShouldLog(VMLogType::STATIC_WRITES, scriptHash))
                VMLogger::Logf("[%s+0x%08X] Static_%u = %d", scriptName, pc, _static, sp[1].Int);

            stack[_static].Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U16:
        {
            ++sp;
            uint16_t global = GET_U16;
            debugger->BeginTracking(scriptHash, global, true);
            sp[0].Reference = globals[0] + global;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U16_LOAD:
        {
            ++sp;
            sp[0].Any = globals[0][GET_U16].Any;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U16_STORE:
        {
            --sp;
            uint16_t global = GET_U16;

            if (VMLogger::ShouldLog(VMLogType::GLOBAL_WRITES, scriptHash))
                VMLogger::Logf("[%s+0x%08X] Global_%u = %d", scriptName, pc, global, sp[1].Int);

            globals[0][global].Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::J:
        {
            int32_t ofs = GET_S16;
            JUMP(code - base + ofs);
            goto NEXT;
        }
        case scrOpcode::JZ:
        {
            int32_t ofs = GET_S16;
            --sp;
            if (sp[1].Int == 0)
                JUMP(code - base + ofs);
            else
                JUMP(code - base);
            goto NEXT;
        }
        case scrOpcode::IEQ_JZ:
        {
            int32_t ofs = GET_S16;
            sp -= 2;
            if (!(sp[1].Int == sp[2].Int))
                JUMP(code - base + ofs);
            else
                JUMP(code - base);
            goto NEXT;
        }
        case scrOpcode::INE_JZ:
        {
            int32_t ofs = GET_S16;
            sp -= 2;
            if (!(sp[1].Int != sp[2].Int))
                JUMP(code - base + ofs);
            else
                JUMP(code - base);
            goto NEXT;
        }
        case scrOpcode::IGT_JZ:
        {
            int32_t ofs = GET_S16;
            sp -= 2;
            if (!(sp[1].Int > sp[2].Int))
                JUMP(code - base + ofs);
            else
                JUMP(code - base);
            goto NEXT;
        }
        case scrOpcode::IGE_JZ:
        {
            int32_t ofs = GET_S16;
            sp -= 2;
            if (!(sp[1].Int >= sp[2].Int))
                JUMP(code - base + ofs);
            else
                JUMP(code - base);
            goto NEXT;
        }
        case scrOpcode::ILT_JZ:
        {
            int32_t ofs = GET_S16;
            sp -= 2;
            if (!(sp[1].Int < sp[2].Int))
                JUMP(code - base + ofs);
            else
                JUMP(code - base);
            goto NEXT;
        }
        case scrOpcode::ILE_JZ:
        {
            int32_t ofs = GET_S16;
            sp -= 2;
            if (!(sp[1].Int <= sp[2].Int))
                JUMP(code - base + ofs);
            else
                JUMP(code - base);
            goto NEXT;
        }
        case scrOpcode::CALL:
        {
            uint32_t ofs = GET_U24;
            ++sp;
            sp[0].Uns = static_cast<int32_t>(code - base);
            JUMP(ofs);
            goto NEXT;
        }
        case scrOpcode::STATIC_U24:
        {
            ++sp;
            uint32_t offset = GET_U24;
            debugger->BeginTracking(scriptHash, offset, false);
            sp[0].Reference = stack + offset;
            goto NEXT;
        }
        case scrOpcode::STATIC_U24_LOAD:
        {
            ++sp;
            sp[0].Any = stack[GET_U24].Any;
            goto NEXT;
        }
        case scrOpcode::STATIC_U24_STORE:
        {
            --sp;
            uint32_t _static = GET_U24;

            if (VMLogger::ShouldLog(VMLogType::STATIC_WRITES, scriptHash))
                VMLogger::Logf("[%s+0x%08X] Static_%u = %d", scriptName, pc, _static, sp[1].Int);

            stack[_static].Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U24:
        {
            uint32_t global = GET_U24;
            uint32_t block = global >> 0x12U;
            uint32_t index = global & 0x3FFFFU;
            ++sp;
            if (!globals[block])
                return T::OnException(pc, op, "Global block %u (index %u) is not loaded!", block, index);
            else
            {
                debugger->BeginTracking(scriptHash, global, true);
                sp[0].Reference = &globals[block][index];
            }
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U24_LOAD:
        {
            uint32_t global = GET_U24;
            uint32_t block = global >> 0x12U;
            uint32_t index = global & 0x3FFFFU;
            ++sp;
            if (!globals[block])
                return T::OnException(pc, op, "Global block %u (index %u) is not loaded!", block, index);
            else
                sp[0].Any = globals[block][index].Any;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U24_STORE:
        {
            uint32_t global = GET_U24;
            uint32_t block = global >> 0x12U;
            uint32_t index = global & 0x3FFFFU;
            --sp;
            if (!globals[block])
                return T::OnException(pc, op, "Global block %u (index %u) is not loaded!", block, index);
            else
            {
                if (VMLogger::ShouldLog(VMLogType::GLOBAL_WRITES, scriptHash))
                    VMLogger::Logf("[%s+0x%08X] Global_%u = %d", scriptName, pc, global, sp[1].Int);

                globals[block][index].Any = sp[1].Any;
            }
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_U24:
        {
            ++sp;
            uint32_t offset = GET_U24;
            debugger->AddFieldOffset(offset);
            sp[0].Int = offset;
            goto NEXT;
        }
        case scrOpcode::SWITCH:
        {
            --sp;
            uint32_t switchVal = sp[1].Uns;
            uint32_t caseCount = GET_U8;
            JUMP(code - base);
            for (uint32_t i = 0; i < caseCount; i++)
            {
                uint32_t caseVal = GET_U32;
                uint32_t ofs = GET_U16;
                if (switchVal == caseVal)
                {
                    JUMP(code - base + ofs);
                    break;
                }
            }
            JUMP(code - base);
            goto NEXT;
        }
        case scrOpcode::STRING:
        {
            uint32_t ofs = sp[0].Uns;
            sp[0].String = program->GetString(ofs);
            goto NEXT;
        }
        case scrOpcode::STRINGHASH:
        {
            sp[0].Uns = JOAAT(sp[0].String);
            goto NEXT;
        }
        case scrOpcode::TEXT_LABEL_ASSIGN_STRING:
        {
            sp -= 2;
            char* dest = const_cast<char*>(sp[2].String);
            const char* src = sp[1].String;
            scrStringAssign(dest, GET_U8, src);
            goto NEXT;
        }
        case scrOpcode::TEXT_LABEL_ASSIGN_INT:
        {
            sp -= 2;
            char* dest = const_cast<char*>(sp[2].String);
            int32_t value = sp[1].Int;
            scrStringItoa(buffer, value);
            scrStringAssign(dest, GET_U8, buffer);
            goto NEXT;
        }
        case scrOpcode::TEXT_LABEL_APPEND_STRING:
        {
            sp -= 2;
            char* dest = const_cast<char*>(sp[2].String);
            const char* src = sp[1].String;
            scrStringAppend(dest, GET_U8, src);
            goto NEXT;
        }
        case scrOpcode::TEXT_LABEL_APPEND_INT:
        {
            sp -= 2;
            char* dest = const_cast<char*>(sp[2].String);
            int32_t value = sp[1].Int;
            scrStringItoa(buffer, value);
            scrStringAppend(dest, GET_U8, buffer);
            goto NEXT;
        }
        case scrOpcode::TEXT_LABEL_COPY:
        {
            sp -= 3;
            scrValue* dest = sp[3].Reference;
            int32_t destSize = sp[2].Int;
            int32_t srcSize = sp[1].Int;
            if (srcSize > destSize)
            {
                int32_t excess = srcSize - destSize;
                sp -= excess;
                srcSize = destSize;
            }
            scrValue* destPtr = dest + srcSize - 1;
            for (int32_t i = 0; i < srcSize; i++)
                *destPtr-- = *sp--;
            reinterpret_cast<char*>(dest)[srcSize * sizeof(scrValue) - 1] = '\0';
            goto NEXT;
        }
        case scrOpcode::CATCH:
        {
            context->m_CatchPc = static_cast<int32_t>(code - base);
            context->m_CatchFp = static_cast<int32_t>(fp - stack);
            context->m_CatchSp = static_cast<int32_t>(sp - stack + 1);
            ++sp;
            sp[0].Int = -1;
            goto NEXT;
        }
        case scrOpcode::THROW:
        {
            int32_t ofs = sp[0].Int;
            if (!context->m_CatchPc)
                return T::OnException(pc, op, "Catch PC is NULL!");
            else
            {
                JUMP(context->m_CatchPc);
                fp = stack + context->m_CatchFp;
                sp = stack + context->m_CatchSp;
                sp[0].Int = ofs;
            }
            goto NEXT;
        }
        case scrOpcode::CALLINDIRECT:
        {
            uint32_t ofs = sp[0].Uns;
            if (!ofs)
                return T::OnException(pc, op, "Function pointer is NULL!");
            sp[0].Uns = static_cast<int32_t>(code - base);
            JUMP(ofs);
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_M1:
        {
            ++sp;
            sp[0].Int = -1;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_0:
        {
            ++sp;
            sp[0].Any = 0;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_1:
        {
            ++sp;
            sp[0].Int = 1;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_2:
        {
            ++sp;
            sp[0].Int = 2;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_3:
        {
            ++sp;
            sp[0].Int = 3;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_4:
        {
            ++sp;
            sp[0].Int = 4;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_5:
        {
            ++sp;
            sp[0].Int = 5;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_6:
        {
            ++sp;
            sp[0].Int = 6;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_7:
        {
            ++sp;
            sp[0].Int = 7;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_FM1:
        {
            ++sp;
            sp[0].Uns = 0xBF800000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F0:
        {
            ++sp;
            sp[0].Uns = 0x00000000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F1:
        {
            ++sp;
            sp[0].Uns = 0x3F800000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F2:
        {
            ++sp;
            sp[0].Uns = 0x40000000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F3:
        {
            ++sp;
            sp[0].Uns = 0x40400000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F4:
        {
            ++sp;
            sp[0].Uns = 0x40800000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F5:
        {
            ++sp;
            sp[0].Uns = 0x40A00000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F6:
        {
            ++sp;
            sp[0].Uns = 0x40C00000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F7:
        {
            ++sp;
            sp[0].Uns = 0x40E00000;
            goto NEXT;
        }
        case scrOpcode::IS_BIT_SET:
        {
            --sp;
            sp[0].Int = (sp[0].Int & (1 << sp[1].Int)) != 0;
            goto NEXT;
        }
        }

        return T::OnException(pc, op, "Invalid opcode: 0x%02X", static_cast<uint8_t>(op));
    }

    template scrThreadGEN8::TLS GetTLSInternal<scrThreadGEN8::TLS>();
    template scrThreadGEN9::TLS GetTLSInternal<scrThreadGEN9::TLS>();

    template scrThreadGEN8* GetByHashInternal<scrThreadGEN8*>(uint32_t);
    template scrThreadGEN9* GetByHashInternal<scrThreadGEN9*>(uint32_t);

    template scrThreadState OnExceptionInternal<scrThreadGEN8>(uint32_t, scrOpcode, const char*, va_list);
    template scrThreadState OnExceptionInternal<scrThreadGEN9>(uint32_t, scrOpcode, const char*, va_list);

    template scrThreadState RunThreadInternal<scrThreadGEN8>(scrValue*, scrValue**, scrProgram*, scrThreadGEN8::Context*);
    template scrThreadState RunThreadInternal<scrThreadGEN9>(scrValue*, scrValue**, scrProgram*, scrThreadGEN9::Context*);
}

#endif