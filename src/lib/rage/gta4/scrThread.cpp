#include "scrThread.hpp"
#include "ResourceLoader.hpp"
#include "debugger/VMLogger.hpp"
#include "game/GTA4.hpp"
#include "rage/shared/atArray.hpp"
#include "rage/shared/scrHash.hpp"
#include "rage/shared/scrString.hpp"
#include "scrProgram.hpp"

#if defined(_M_IX86)

namespace rage::gta4
{
    using namespace scrDbgLib;
    using namespace scrDbgShared;

    static int32_t g_NullContainer = 0;

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

        return std::to_string(value.Int);
    }

    scrThread* scrThread::GetByHash(uint32_t hash)
    {
        auto threads = GTA4::GetPointers().ScriptThreads;
        if (!threads)
            return nullptr;

        for (auto& thread : *threads)
        {
            if (thread && thread->m_Context.m_Id != 0 && thread->m_Context.m_ProgramHash == hash)
                return thread;
        }

        return nullptr;
    }

    scrNativeContext::Handler scrThread::GetCommandHandler(uint32_t hash)
    {
        auto handlers = GTA4::GetPointers().CommandHandlers;
        if (!handlers)
            return nullptr;

        return handlers->Lookup(hash);
    }

    uint32_t scrThread::GetCommandHash(scrNativeContext::Handler handler)
    {
        auto handlers = GTA4::GetPointers().CommandHandlers;
        if (!handlers)
            return 0;

        return handlers->LookupHash(handler);
    }

    scrThread::State scrThread::OnException(uint32_t pc, scrOpcode op, const char* fmt, ...)
    {
        char message[512];

        va_list args;
        va_start(args, fmt);
        std::vsnprintf(message, sizeof(message), fmt, args);
        va_end(args);

        char fullMessage[600];
        std::snprintf(fullMessage, sizeof(fullMessage), "Exception in %s at 0x%X (%s)!\nReason: %s",
            m_ScriptName,
            pc,
            static_cast<uint8_t>(op) < 256 ? g_scrOpcodeNames[static_cast<uint8_t>(op)] : "???",
            message);
        MessageBoxA(0, fullMessage, "Exception", MB_OK | MB_ICONERROR);

        m_KillReason = fmt; // = message;
        return m_Context.m_State = State::KILLED;
    }

    scrThread::State __fastcall scrThread::RunThread(scrThread* _this, void* edx, int32_t insnCount)
    {
        if (_this->m_Context.m_State == State::KILLED || _this->m_Context.m_State == State::PAUSED)
            return _this->m_Context.m_State;

        scrThread** curThread = GTA4::GetPointers().CurrentScriptThread;

        scrThread* prevThread = *curThread;
        *curThread = _this;

        struct ThreadRestore
        {
            scrThread** Current;
            scrThread* Previous;

            ~ThreadRestore()
            {
                *Current = Previous;
            }
        } restore{curThread, prevThread};

        scrProgram* program = scrProgram::GetProgram(_this->m_Context.m_ProgramHash);
        if (!program || program->m_NameHash == "remindermp"_J) // We use hash comparison here which is ~36x faster than R*'s _stricmp check from what I tested.
            return _this->m_Context.m_State = State::KILLED;

        auto frameStart = std::chrono::high_resolution_clock::now();

        Debugger* debugger = g_Game->GetDebugger();
        auto& pointers = GTA4::GetPointers();

        uint8_t* code = program->m_Code;
        scrValue* stack = _this->m_Stack;
        scrValue* globals = *pointers.ScriptGlobals;
        uint8_t* protectedGlobals = *pointers.ProtectedScriptGlobals;
        uint32_t (*getNextSlot)() = pointers.GetNextProtectedScriptSlot;
        uint32_t globalsCount = *pointers.ScriptGlobalsCount;

        uint32_t pc = _this->m_Context.m_Pc;
        scrValue* sp = &_this->m_Stack[_this->m_Context.m_Sp - 1];

        while (insnCount-- > 0)
        {
            _this->m_InsnCount++;

            if (debugger->ProcessBreakpoints(_this->m_Context.m_ProgramHash, pc, (uint32_t*)&_this->m_Context.m_State))
            {
                _this->m_Context.m_Pc = pc;
                _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
                return _this->m_Context.m_State; // If we do not return here, the VM will end up executing opcodes until the next NATIVE call.
            }

            scrOpcode op = static_cast<scrOpcode>(code[pc++]);
            if (debugger->ShouldBreakTracking(static_cast<uint8_t>(op)))
                debugger->BreakTracking();

            switch (op)
            {
            case scrOpcode::NOP:
            {
                break;
            }
            case scrOpcode::IADD:
            {
                --sp;
                debugger->AddFieldOffset(sp[1].Int);
                sp[0].Int += sp[1].Int;
                break;
            }
            case scrOpcode::ISUB:
            {
                --sp;
                sp[0].Int -= sp[1].Int;
                break;
            }
            case scrOpcode::IMUL:
            {
                --sp;
                sp[0].Int *= sp[1].Int;
                break;
            }
            case scrOpcode::IDIV:
            {
                --sp;
                if (sp[1].Int != 0)
                    sp[0].Int /= sp[1].Int;
                break;
            }
            case scrOpcode::IMOD:
            {
                --sp;
                if (sp[1].Int != 0)
                    sp[0].Int %= sp[1].Int;
                break;
            }
            case scrOpcode::INOT:
            {
                sp[0].Int = sp[0].Int == 0;
                break;
            }
            case scrOpcode::INEG:
            {
                sp[0].Int = -sp[0].Int;
                break;
            }
            case scrOpcode::IEQ:
            {
                --sp;
                sp[0].Int = sp[0].Int == sp[1].Int;
                break;
            }
            case scrOpcode::INE:
            {
                --sp;
                sp[0].Int = sp[0].Int != sp[1].Int;
                break;
            }
            case scrOpcode::IGT:
            {
                --sp;
                sp[0].Int = sp[0].Int > sp[1].Int;
                break;
            }
            case scrOpcode::IGE:
            {
                --sp;
                sp[0].Int = sp[0].Int >= sp[1].Int;
                break;
            }
            case scrOpcode::ILT:
            {
                --sp;
                sp[0].Int = sp[0].Int < sp[1].Int;
                break;
            }
            case scrOpcode::ILE:
            {
                --sp;
                sp[0].Int = sp[0].Int <= sp[1].Int;
                break;
            }
            case scrOpcode::FADD:
            {
                --sp;
                sp[0].Float += sp[1].Float;
                break;
            }
            case scrOpcode::FSUB:
            {
                --sp;
                sp[0].Float -= sp[1].Float;
                break;
            }
            case scrOpcode::FMUL:
            {
                --sp;
                sp[0].Float *= sp[1].Float;
                break;
            }
            case scrOpcode::FDIV:
            {
                --sp;
                if (sp[1].Float != 0.0f)
                    sp[0].Float /= sp[1].Float;
                break;
            }
            case scrOpcode::FMOD:
            {
                --sp;
                if (sp[1].Float != 0.0f)
                {
                    float x = sp[0].Float;
                    float y = sp[1].Float;
                    sp[0].Float = y != 0.0f ? x - (static_cast<int32_t>(x / y) * y) : 0.0f;
                }
                break;
            }
            case scrOpcode::FNEG:
            {
                sp[0].Uns ^= 0x80000000;
                break;
            }
            case scrOpcode::FEQ:
            {
                --sp;
                sp[0].Int = sp[0].Float == sp[1].Float;
                break;
            }
            case scrOpcode::FNE:
            {
                --sp;
                sp[0].Int = sp[0].Float != sp[1].Float;
                break;
            }
            case scrOpcode::FGT:
            {
                --sp;
                sp[0].Int = sp[0].Float > sp[1].Float;
                break;
            }
            case scrOpcode::FGE:
            {
                --sp;
                sp[0].Int = sp[0].Float >= sp[1].Float;
                break;
            }
            case scrOpcode::FLT:
            {
                --sp;
                sp[0].Int = sp[0].Float < sp[1].Float;
                break;
            }
            case scrOpcode::FLE:
            {
                --sp;
                sp[0].Int = sp[0].Float <= sp[1].Float;
                break;
            }
            case scrOpcode::VADD:
            {
                sp -= 3;
                sp[-2].Float += sp[1].Float;
                sp[-1].Float += sp[2].Float;
                sp[0].Float += sp[3].Float;
                break;
            }
            case scrOpcode::VSUB:
            {
                sp -= 3;
                sp[-2].Float -= sp[1].Float;
                sp[-1].Float -= sp[2].Float;
                sp[0].Float -= sp[3].Float;
                break;
            }
            case scrOpcode::VMUL:
            {
                sp -= 3;
                sp[-2].Float *= sp[1].Float;
                sp[-1].Float *= sp[2].Float;
                sp[0].Float *= sp[3].Float;
                break;
            }
            case scrOpcode::VDIV:
            {
                sp -= 3;
                sp[-2].Float = sp[1].Float != 0.0f ? sp[-2].Float /= sp[1].Float : 0.0f;
                sp[-1].Float = sp[2].Float != 0.0f ? sp[-1].Float /= sp[2].Float : 0.0f;
                sp[0].Float = sp[3].Float != 0.0f ? sp[0].Float /= sp[3].Float : 0.0f;
                break;
            }
            case scrOpcode::VNEG:
            {
                sp[-2].Uns ^= 0x80000000;
                sp[-1].Uns ^= 0x80000000;
                sp[0].Uns ^= 0x80000000;
                break;
            }
            case scrOpcode::IAND:
            {
                --sp;
                sp[0].Int &= sp[1].Int;
                break;
            }
            case scrOpcode::IOR:
            {
                --sp;
                sp[0].Int |= sp[1].Int;
                break;
            }
            case scrOpcode::IXOR:
            {
                --sp;
                sp[0].Int ^= sp[1].Int;
                break;
            }
            case scrOpcode::J:
            {
                pc = *reinterpret_cast<uint32_t*>(&code[pc]);
                break;
            }
            case scrOpcode::JZ:
            {
                int32_t cond = sp[0].Int;
                --sp;
                if (cond == 0)
                    pc = *reinterpret_cast<uint32_t*>(&code[pc]);
                else
                    pc += 4;
                break;
            }
            case scrOpcode::JNZ:
            {
                int32_t cond = sp[0].Int;
                --sp;
                if (cond != 0)
                    pc = *reinterpret_cast<uint32_t*>(&code[pc]);
                else
                    pc += 4;
                break;
            }
            case scrOpcode::I2F:
            {
                sp[0].Float = static_cast<float>(sp[0].Int);
                break;
            }
            case scrOpcode::F2I:
            {
                sp[0].Int = static_cast<int32_t>(sp[0].Float);
                break;
            }
            case scrOpcode::F2V:
            {
                sp += 2;
                sp[-1].Int = sp[0].Int = sp[-2].Int;
                break;
            }
            case scrOpcode::PUSH_CONST_S16:
            {
                ++sp;
                sp[0].Int = *reinterpret_cast<int16_t*>(&code[pc]);
                pc += 2;
                break;
            }
            case scrOpcode::PUSH_CONST_U32:
            case scrOpcode::PUSH_CONST_F:
            {
                ++sp;
                sp[0].Uns = *reinterpret_cast<uint32_t*>(&code[pc]);
                pc += 4;
                break;
            }
            case scrOpcode::DUP:
            {
                ++sp;
                sp[0].Int = sp[-1].Int;
                break;
            }
            case scrOpcode::DROP:
            {
                --sp;
                break;
            }
            case scrOpcode::NATIVE:
            {
                _this->m_Context.m_Pc = pc - 1;

                uint8_t argCount = code[pc++];
                uint8_t retCount = code[pc++];
                uint32_t nativeAddr = *reinterpret_cast<uint32_t*>(&code[pc]);
                pc += 4;

                scrNativeContext::Handler handler = reinterpret_cast<scrNativeContext::Handler>(nativeAddr);
                if (!handler)
                    return _this->OnException(pc, op, "Missing native command");

                _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);

                scrNativeContext ctx{};
                ctx.m_Args = &stack[_this->m_Context.m_Sp - argCount];
                ctx.m_ArgCount = argCount;
                ctx.m_Rets = retCount ? &stack[_this->m_Context.m_Sp - argCount] : nullptr;
                ctx.m_VectorRefCount = 0;

                uint32_t hash{};
                std::string_view name{};
                std::string argsStr{};
                std::string retsStr{};
                bool shouldLog = VMLogger::ShouldLog(VMLogType::NATIVE_CALLS, _this->m_Context.m_ProgramHash);

                // log args before calling the native because rets will be written to the same stack slot
                if (shouldLog)
                {
                    hash = GetCommandHash(handler);
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
                        VMLogger::Logf("[%s+0x%08X] %s(%s)%s", _this->m_ScriptName, pc - 7, name.data(), argsStr.c_str(), retsStr.c_str());
                    else
                        VMLogger::Logf("[%s+0x%08X] unk_0x%08X(%s)%s", _this->m_ScriptName, pc - 7, hash, argsStr.c_str(), retsStr.c_str());
                }

                if (_this->m_Context.m_State == State::REFRESH)
                {
                    insnCount = 1100000;
                    _this->m_Context.m_State = State::RUNNING;
                }
                else if (_this->m_Context.m_State != State::RUNNING)
                {
                    if (VMLogger::ShouldLog(VMLogType::FRAME_TIME, _this->m_Context.m_ProgramHash))
                    {
                        auto frameEnd = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
                        VMLogger::Logf("[%s] %.3f ms", _this->m_ScriptName, duration);
                    }

                    _this->m_Context.m_Pc = pc - 7;
                    return _this->m_Context.m_State;
                }

                ctx.CopyVectorsOut();

                _this->m_Context.m_Sp += retCount - argCount;
                sp = &stack[_this->m_Context.m_Sp - 1];
                break;
            }
            case scrOpcode::CALL:
            {
                uint32_t retPc = pc + 4;
                pc = *reinterpret_cast<uint32_t*>(&code[pc]);
                ++sp;
                sp[0].Int = retPc;
                break;
            }
            case scrOpcode::ENTER:
            {
                uint8_t argCount = code[pc++];
                uint16_t localCount = *reinterpret_cast<uint16_t*>(&code[pc]);
                pc += 2;

                ++sp;
                sp[0].Int = _this->m_Context.m_Fp;

                uint32_t callsitePc = sp[-1].Uns - 5;

                uint32_t newSp = static_cast<uint32_t>(sp - stack + 1);
                _this->m_Context.m_Sp = newSp;
                _this->m_Context.m_Fp = newSp - argCount - 2;

                for (uint32_t i = 0; i < localCount; i++)
                    stack[_this->m_Context.m_Sp + i].Int = 0;

                _this->m_Context.m_Sp += localCount - argCount;
                sp = &stack[_this->m_Context.m_Sp - 1];

                if (VMLogger::ShouldLog(VMLogType::FUNCTION_CALLS, _this->m_Context.m_ProgramHash))
                {
                    auto funcName = program->GetFuncName(pc - 4);

                    std::string argsStr = "";
                    for (int i = 0; i < argCount; i++)
                    {
                        argsStr += std::to_string(stack[_this->m_Context.m_Fp + 2 + i].Int);
                        if (i != argCount - 1)
                            argsStr += ", ";
                    }

                    VMLogger::Logf("[%s+0x%08X] %s(%s)", _this->m_ScriptName, callsitePc, funcName.c_str(), argsStr.c_str());
                }
                break;
            }
            case scrOpcode::LEAVE:
            {
                uint8_t argCount = code[pc++];
                uint8_t retCount = code[pc++];

                uint32_t fpOffset = argCount + _this->m_Context.m_Fp + 2;
                scrValue* framePtr = &stack[fpOffset - 2];
                uint32_t retPc = framePtr[0].Uns;

                _this->m_Context.m_Sp = fpOffset;
                _this->m_Context.m_Fp = framePtr[1].Uns;

                uint32_t newSp = (framePtr - stack) - argCount;
                _this->m_Context.m_Sp = newSp;

                int32_t src = sp - stack - retCount + 1;
                for (uint8_t i = 0; i < retCount; i++)
                    stack[_this->m_Context.m_Sp++] = stack[src++];

                sp = &stack[_this->m_Context.m_Sp - 1];
                pc = retPc;

                if (pc == 0)
                {
                    _this->m_KillReason = "Reached end of program.";
                    return _this->m_Context.m_State = State::KILLED;
                }
                break;
            }
            case scrOpcode::LOAD:
            {
                sp[0].Int = sp[0].Reference->Int;
                break;
            }
            case scrOpcode::STORE:
            {
                scrValue* addr = sp[0].Reference;
                int32_t val = sp[-1].Int;
                sp -= 2;
                addr->Int = val;
                debugger->FinalizeTracking(_this->m_ScriptName, pc - 1, val, false);
                break;
            }
            case scrOpcode::STORE_REV:
            {
                int32_t val = sp[0].Int;
                scrValue* addr = sp[-1].Reference;
                --sp;
                addr->Int = val;
                break;
            }
            case scrOpcode::ARRAY_LOAD:
            {
                scrValue* addr = sp[0].Reference;
                int32_t count = sp[-1].Int;
                sp -= 2;
                for (int32_t i = 0; i < count; i++)
                    (++sp)[0].Int = addr[i].Int;
                break;
            }
            case scrOpcode::ARRAY_STORE:
            {
                scrValue* addr = sp[0].Reference;
                int32_t count = sp[-1].Int;
                sp -= 2;
                for (int32_t i = count - 1; i >= 0; i--)
                    addr[i].Int = (sp--)[0].Int;
                debugger->FinalizeTracking(_this->m_ScriptName, pc - 1, count, true);
                break;
            }
            case scrOpcode::LOCAL_0:
            case scrOpcode::LOCAL_1:
            case scrOpcode::LOCAL_2:
            case scrOpcode::LOCAL_3:
            case scrOpcode::LOCAL_4:
            case scrOpcode::LOCAL_5:
            case scrOpcode::LOCAL_6:
            case scrOpcode::LOCAL_7:
            {
                (++sp)[0].Reference = &stack[_this->m_Context.m_Fp + (static_cast<uint8_t>(op) - static_cast<uint8_t>(scrOpcode::LOCAL_0))];
                break;
            }
            case scrOpcode::LOCAL:
            {
                sp[0].Reference = &stack[_this->m_Context.m_Fp + sp[0].Int];
                break;
            }
            case scrOpcode::STATIC:
            {
                debugger->BeginTracking(_this->m_Context.m_ProgramHash, sp[0].Int, false);
                sp[0].Reference = &stack[sp[0].Int];
                break;
            }
            case scrOpcode::GLOBAL:
            {
                debugger->BeginTracking(_this->m_Context.m_ProgramHash, sp[0].Int, true);
                sp[0].Reference = &globals[sp[0].Int];
                break;
            }
            case scrOpcode::ARRAY:
            {
                int32_t itemSize = sp[-1].Int;
                scrValue* addr = sp[0].Reference;
                sp -= 2;

                if (sp[0].Int < 0 || sp[0].Int >= addr->Int)
                    return _this->OnException(pc, op, "Array overrun: %u >= %u", sp[0].Int, addr->Int);

                debugger->AddArrayIndex(sp[0].Int, addr->Int);
                sp[0].Reference = &addr[1 + (sp[0].Int * itemSize)];
                break;
            }
            case scrOpcode::SWITCH:
            {
                int32_t val = sp[0].Int;
                uint8_t count = code[pc++];
                --sp;

                uint32_t tablePc = pc;
                uint32_t endPc = pc + count * 8;
                bool found = false;
                for (uint8_t i = 0; i < count; i++)
                {
                    int32_t caseVal = *reinterpret_cast<int32_t*>(&code[tablePc + i * 8]);
                    uint32_t target = *reinterpret_cast<uint32_t*>(&code[tablePc + i * 8 + 4]);
                    if (val == caseVal)
                    {
                        pc = target;
                        found = true;
                        break;
                    }
                }

                if (!found)
                    pc = endPc;
                break;
            }
            case scrOpcode::STRING:
            {
                uint8_t len = code[pc++];
                (++sp)[0].String = reinterpret_cast<const char*>(&code[pc]);
                pc += len;
                break;
            }
            case scrOpcode::_NULL:
            {
                (++sp)[0].Reference = reinterpret_cast<scrValue*>(&g_NullContainer);
                break;
            }
            case scrOpcode::TEXT_LABEL_ASSIGN_STRING:
            {
                char* dest = const_cast<char*>(sp[0].String);
                const char* src = sp[-1].String;
                uint8_t capacity = code[pc++];
                sp -= 2;

                scrStringAssign(dest, capacity, src);
                break;
            }
            case scrOpcode::TEXT_LABEL_ASSIGN_INT:
            {
                char* dest = const_cast<char*>(sp[0].String);
                int32_t val = sp[-1].Int;
                uint8_t capacity = code[pc++];
                sp -= 2;

                char buf[32];
                scrStringItoa(buf, val);
                scrStringAssign(dest, capacity, buf);
                break;
            }
            case scrOpcode::TEXT_LABEL_APPEND_STRING:
            {
                char* dest = const_cast<char*>(sp[0].String);
                const char* src = sp[-1].String;
                uint8_t capacity = code[pc++];
                sp -= 2;

                scrStringAppend(dest, capacity, src);
                break;
            }
            case scrOpcode::TEXT_LABEL_APPEND_INT:
            {
                char* dest = const_cast<char*>(sp[0].String);
                int32_t val = sp[-1].Int;
                uint8_t capacity = code[pc++];
                sp -= 2;

                char buf[32];
                scrStringItoa(buf, val);
                scrStringAppend(dest, capacity, buf);
                break;
            }
            case scrOpcode::CATCH:
            {
                _this->m_Context.m_CatchFp = _this->m_Context.m_Fp;
                _this->m_Context.m_CatchPc = pc;
                _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
                _this->m_Context.m_CatchSp = static_cast<uint32_t>(sp - stack + 1);
                (++sp)[0].Int = -1;
                break;
            }
            case scrOpcode::THROW:
            {
                if (!_this->m_Context.m_CatchPc)
                    return _this->OnException(pc, op, "THROW with no CATCH");

                int32_t val = sp[0].Int;

                pc = _this->m_Context.m_CatchPc;
                _this->m_Context.m_Fp = _this->m_Context.m_CatchFp;
                _this->m_Context.m_Sp = _this->m_Context.m_CatchSp;
                sp = &stack[_this->m_Context.m_Sp];
                sp[0].Int = val;
                break;
            }
            case scrOpcode::TEXT_LABEL_COPY:
            {
                int32_t count = sp[-1].Int;
                uint32_t* dest = reinterpret_cast<uint32_t*>(sp[0].Reference);
                int32_t srcCount = sp[-2].Int;
                sp -= 3;

                if (srcCount > count)
                    srcCount = count;

                for (int32_t i = srcCount - 1; i >= 0; i--)
                    dest[i] = (sp--)[0].Int;

                reinterpret_cast<char*>(dest)[(4 * srcCount) - 1] = 0;
                break;
            }
            case scrOpcode::GETXPROTECT:
            {
                scrValue* addr = sp[0].Reference;
                sp = sp - 1; // will push result back at sp+1 below

                bool isGlobal;
                uint8_t* protectedStorage;
                if (addr < globals || addr >= &globals[globalsCount])
                {
                    isGlobal = false;
                    protectedStorage = _this->m_ProtectedStack;
                }
                else
                {
                    isGlobal = true;
                    protectedStorage = protectedGlobals;
                }

                uint32_t slotIndex = addr->Uns;
                bool freshSlot = false;
                if (slotIndex == 0)
                {
                    freshSlot = true;
                    if (isGlobal)
                    {
                        slotIndex = getNextSlot();
                        addr->Uns = slotIndex;
                    }
                    else
                    {
                        if (_this->m_ProtectedSlotIndex < 2048)
                        {
                            slotIndex = (_this->m_ProtectedSlotIndex + 1) * 4;
                            addr->Uns = slotIndex;
                            _this->m_ProtectedSlotIndex++;
                        }
                        else
                        {
                            slotIndex = 0;
                            return _this->OnException(pc, op, "Protected slot overrun: %d", _this->m_ProtectedSlotIndex);
                        }
                    }
                }

                ++sp;

                if (protectedStorage)
                {
                    if (freshSlot)
                        reinterpret_cast<scrValue*>(protectedStorage + slotIndex)->Int = 0;
                    sp[0].Int = reinterpret_cast<scrValue*>(protectedStorage + slotIndex)->Int;
                }
                else
                {
                    sp[0].Int = 0;
                }
                break;
            }
            case scrOpcode::SETXPROTECT:
            {
                scrValue* addr = sp[0].Reference;
                int32_t value = sp[-1].Int;
                sp -= 2;

                bool isGlobal;
                uint8_t* protectedStorage;
                if (addr < globals || addr >= &globals[globalsCount])
                {
                    isGlobal = false;
                    protectedStorage = _this->m_ProtectedStack;
                }
                else
                {
                    isGlobal = true;
                    protectedStorage = protectedGlobals;
                }

                uint32_t slotIndex = addr->Uns;
                if (slotIndex == 0)
                {
                    if (isGlobal)
                    {
                        slotIndex = getNextSlot();
                        addr->Uns = slotIndex;
                    }
                    else
                    {
                        if (_this->m_ProtectedSlotIndex < 2048)
                        {
                            slotIndex = (_this->m_ProtectedSlotIndex + 1) * 4;
                            addr->Uns = slotIndex;
                            _this->m_ProtectedSlotIndex++;
                        }
                        else
                        {
                            slotIndex = 0;
                            return _this->OnException(pc, op, "Protected slot overrun: %d", _this->m_ProtectedSlotIndex);
                        }
                    }
                }

                if (protectedStorage)
                {
                    reinterpret_cast<scrValue*>(protectedStorage + slotIndex)->Int = value;
                    debugger->FinalizeTracking(_this->m_ScriptName, pc - 1, value);
                }
                break;
            }
            case scrOpcode::REFXPROTECT:
            {
                uint32_t flags = sp[0].Uns;
                int32_t count = sp[-1].Int;
                scrValue* addr = sp[-2].Reference;
                sp -= 3;

                bool isGlobal;
                uint8_t* protectedStorage;
                if (addr < globals || addr >= &globals[globalsCount])
                {
                    isGlobal = false;
                    protectedStorage = _this->m_ProtectedStack;
                }
                else
                {
                    isGlobal = true;
                    protectedStorage = protectedGlobals;
                }

                if ((flags & 1) != 0)
                {
                    bool pushedRef = false;
                    for (int32_t i = 0; i < count; i++)
                    {
                        int32_t resolvedValue = 0;
                        if (i == 0 && (flags & 4) != 0)
                        {
                            resolvedValue = addr->Int;
                        }
                        else
                        {
                            uint32_t* slotPtr = &addr[i].Uns;
                            uint32_t slotIndex = *slotPtr;
                            if (slotIndex == 0)
                            {
                                if (isGlobal)
                                {
                                    slotIndex = getNextSlot();
                                    *slotPtr = slotIndex;
                                }
                                else
                                {
                                    if (_this->m_ProtectedSlotIndex < 2048)
                                    {
                                        slotIndex = (_this->m_ProtectedSlotIndex + 1) * 4;
                                        *slotPtr = slotIndex;
                                        _this->m_ProtectedSlotIndex++;
                                    }
                                    else
                                    {
                                        return _this->OnException(pc, op, "Protected slot overrun: %d", _this->m_ProtectedSlotIndex);
                                    }
                                }
                            }

                            if (protectedStorage)
                                resolvedValue = reinterpret_cast<scrValue*>(protectedStorage + slotIndex)->Int;
                        }

                        _this->m_ProtectedTempStack[++_this->m_Context.m_ProtectedSp].Int = resolvedValue;

                        if (!pushedRef)
                        {
                            ++sp;
                            sp[0].Reference = &_this->m_ProtectedTempStack[_this->m_Context.m_ProtectedSp];
                            pushedRef = true;
                        }
                    }
                }
                else if ((flags & 2) != 0)
                {
                    for (int32_t i = count - 1; i >= 0; i--)
                    {
                        uint32_t protectedSp = _this->m_Context.m_ProtectedSp--;
                        int32_t storedVal = _this->m_ProtectedTempStack[protectedSp].Int;
                        if (i == 0 && (flags & 4) != 0)
                        {
                            addr->Int = storedVal;
                        }
                        else
                        {
                            uint32_t* slotPtr = &addr[i].Uns;
                            uint32_t slotIndex = *slotPtr;
                            if (!slotIndex)
                            {
                                if (isGlobal)
                                {
                                    slotIndex = getNextSlot();
                                    *slotPtr = slotIndex;
                                }
                                else
                                {
                                    if (_this->m_ProtectedSlotIndex < 2048)
                                    {
                                        slotIndex = (_this->m_ProtectedSlotIndex + 1) * 4;
                                        *slotPtr = slotIndex;
                                        _this->m_ProtectedSlotIndex++;
                                    }
                                    else
                                    {
                                        slotIndex = 0;
                                        return _this->OnException(pc, op, "Protected storage overrun: %d", _this->m_ProtectedSlotIndex);
                                    }
                                }
                            }

                            if (protectedStorage)
                                reinterpret_cast<scrValue*>(protectedStorage + slotIndex)->Int = storedVal;
                        }
                    }
                }
                break;
            }
            case scrOpcode::EXIT:
            {
                return _this->m_Context.m_State = State::KILLED;
            }
            default:
            {
                if (op >= scrOpcode::PUSH_CONST_M16 && op <= scrOpcode::PUSH_CONST_159)
                {
                    (++sp)[0].Int = static_cast<uint8_t>(op) - 96;
                }
                else
                {
                    return _this->OnException(pc, op, "Invalid opcode: %02X", op);
                }
                break;
            }
            }
        }

        _this->m_Context.m_Pc = pc;
        _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
        return State::RUNNING;
    }
}

#endif