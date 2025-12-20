#include "scrThread.hpp"
#include "Joaat.hpp"
#include "Pointers.hpp"
#include "ResourceLoader.hpp"
#include "atArray.hpp"
#include "core/Memory.hpp"
#include "debug/ScriptBreakpoint.hpp"
#include "debug/ScriptFunctionNames.hpp"
#include "debug/ScriptLogger.hpp"
#include "debug/ScriptVariableWriteTracker.hpp"
#include "scrNativeRegistration.hpp"
#include "scrOpcode.hpp"
#include "scrProgram.hpp"
#include "scrString.hpp"

namespace rage
{
    scrThread* scrThread::GetCurrentThread()
    {
        return *reinterpret_cast<scrThread**>(*reinterpret_cast<uintptr_t*>(__readgsqword(0x58)) + (scrDbgLib::g_IsEnhanced ? 0x7A0 : 0x2A50));
    }

    scrThread* scrThread::GetThread(uint32_t hash)
    {
        for (auto& thread : *scrDbgLib::g_Pointers.ScriptThreads)
        {
            if (thread && *thread->Context()->Id() && *thread->ScriptHash() == hash)
                return thread;
        }

        return nullptr;
    }

    scrThreadState scrThread::OnScriptException(const char* fmt, ...)
    {
        auto thread = GetCurrentThread();
        if (!thread)
            return scrThreadState::KILLED;

        char message[512];

        va_list args;
        va_start(args, fmt);
        std::vsnprintf(message, sizeof(message), fmt, args);
        va_end(args);

        char fullMessage[600];
        std::snprintf(fullMessage, sizeof(fullMessage), "Exception in %s at 0x%X!\nReason: %s", thread->ScriptName(), *thread->Context()->ProgramCounter(), message);
        MessageBoxA(0, fullMessage, "Exception", MB_OK | MB_ICONERROR);

        return *thread->Context()->State() = scrThreadState::KILLED;
    }

#define GET_BYTE (*++codePtr)
#define GET_WORD ((codePtr += 2), *reinterpret_cast<uint16_t*>(codePtr - 1))
#define GET_SWORD ((codePtr += 2), *reinterpret_cast<int16_t*>(codePtr - 1))
#define GET_24BIT ((codePtr += 3), *reinterpret_cast<uint32_t*>(codePtr - 3) >> 8)
#define GET_DWORD ((codePtr += 4), *reinterpret_cast<uint32_t*>(codePtr - 3))

#define JUMP(_index)                           \
    do                                         \
    {                                          \
        int64_t index = _index;                \
        codePtr = program->GetCode(index) - 1; \
        codeBase = &codePtr[-index];           \
    } while (0)

    scrThreadState scrThread::RunThread(scrValue* stack, scrValue** globals, scrProgram* program, scrThreadContext* context)
    {
        auto frameStart = std::chrono::high_resolution_clock::now();

        auto context_ProgramHash = context->ProgramHash(); // Same as m_ScriptHash
        auto context_State = context->State();
        auto context_ProgramCounter = context->ProgramCounter();
        auto context_FramePointer = context->FramePointer();
        auto context_StackPointer = context->StackPointer();
        auto context_StackSize = context->StackSize();
        auto context_CallDepth = context->CallDepth();
        auto context_Callstack = context->Callstack();

        auto scriptThreadName = GetCurrentThread()->ScriptName();

        char buffer[16];
        uint8_t* codePtr;
        uint8_t* codeBase;

        scrValue* stackPtr = stack + *context_StackPointer - 1;
        scrValue* framePtr = stack + *context_FramePointer;

        JUMP(*context_ProgramCounter);

    NEXT:
        // Update these per codePtr start
        *context_ProgramCounter = static_cast<uint32_t>(codePtr - codeBase);
        *context_StackPointer = static_cast<int32_t>(stackPtr - stack + 1);
        *context_FramePointer = static_cast<int32_t>(framePtr - stack);

        if (scrDbgLib::ScriptBreakpoint::Process(context_ProgramHash, context_ProgramCounter, context_State))
            return *context_State; // If we do not return here, the VM will end up executing opcodes until the next NATIVE call.

        uint8_t next = GET_BYTE;
        if (scrDbgLib::ScriptVariableWriteTracker::ShouldBreak(next))
            scrDbgLib::ScriptVariableWriteTracker::Break();

        switch (next)
        {
        case scrOpcode::NOP:
        {
            JUMP(codePtr - codeBase);
            goto NEXT;
        }
        case scrOpcode::IADD:
        {
            --stackPtr;
            stackPtr[0].Int += stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::ISUB:
        {
            --stackPtr;
            stackPtr[0].Int -= stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::IMUL:
        {
            --stackPtr;
            stackPtr[0].Int *= stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::IDIV:
        {
            --stackPtr;
            if (stackPtr[1].Int)
                stackPtr[0].Int /= stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::IMOD:
        {
            --stackPtr;
            if (stackPtr[1].Int)
                stackPtr[0].Int %= stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::INOT:
        {
            stackPtr[0].Int = !stackPtr[0].Int;
            goto NEXT;
        }
        case scrOpcode::INEG:
        {
            stackPtr[0].Int = -stackPtr[0].Int;
            goto NEXT;
        }
        case scrOpcode::IEQ:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int == stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::INE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int != stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::IGT:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int > stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::IGE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int >= stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::ILT:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int < stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::ILE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int <= stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::FADD:
        {
            --stackPtr;
            stackPtr[0].Float += stackPtr[1].Float;
            goto NEXT;
        }
        case scrOpcode::FSUB:
        {
            --stackPtr;
            stackPtr[0].Float -= stackPtr[1].Float;
            goto NEXT;
        }
        case scrOpcode::FMUL:
        {
            --stackPtr;
            stackPtr[0].Float *= stackPtr[1].Float;
            goto NEXT;
        }
        case scrOpcode::FDIV:
        {
            --stackPtr;
            if (stackPtr[1].Int)
                stackPtr[0].Float /= stackPtr[1].Float;
            goto NEXT;
        }
        case scrOpcode::FMOD:
        {
            --stackPtr;
            if (stackPtr[1].Int)
            {
                float x = stackPtr[0].Float;
                float y = stackPtr[1].Float;
                stackPtr[0].Float = y ? x - (static_cast<int32_t>(x / y) * y) : 0.0f;
            }
            goto NEXT;
        }
        case scrOpcode::FNEG:
        {
            stackPtr[0].Uns ^= 0x80000000;
            goto NEXT;
        }
        case scrOpcode::FEQ:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float == stackPtr[1].Float;
            goto NEXT;
        }
        case scrOpcode::FNE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float != stackPtr[1].Float;
            goto NEXT;
        }
        case scrOpcode::FGT:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float > stackPtr[1].Float;
            goto NEXT;
        }
        case scrOpcode::FGE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float >= stackPtr[1].Float;
            goto NEXT;
        }
        case scrOpcode::FLT:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float < stackPtr[1].Float;
            goto NEXT;
        }
        case scrOpcode::FLE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float <= stackPtr[1].Float;
            goto NEXT;
        }
        case scrOpcode::VADD:
        {
            stackPtr -= 3;
            stackPtr[-2].Float += stackPtr[1].Float;
            stackPtr[-1].Float += stackPtr[2].Float;
            stackPtr[0].Float += stackPtr[3].Float;
            goto NEXT;
        }
        case scrOpcode::VSUB:
        {
            stackPtr -= 3;
            stackPtr[-2].Float -= stackPtr[1].Float;
            stackPtr[-1].Float -= stackPtr[2].Float;
            stackPtr[0].Float -= stackPtr[3].Float;
            goto NEXT;
        }
        case scrOpcode::VMUL:
        {
            stackPtr -= 3;
            stackPtr[-2].Float *= stackPtr[1].Float;
            stackPtr[-1].Float *= stackPtr[2].Float;
            stackPtr[0].Float *= stackPtr[3].Float;
            goto NEXT;
        }
        case scrOpcode::VDIV:
        {
            stackPtr -= 3;
            if (stackPtr[1].Int)
                stackPtr[-2].Float /= stackPtr[1].Float;
            if (stackPtr[2].Int)
                stackPtr[-1].Float /= stackPtr[2].Float;
            if (stackPtr[3].Int)
                stackPtr[0].Float /= stackPtr[3].Float;
            goto NEXT;
        }
        case scrOpcode::VNEG:
        {
            stackPtr[-2].Uns ^= 0x80000000;
            stackPtr[-1].Uns ^= 0x80000000;
            stackPtr[0].Uns ^= 0x80000000;
            goto NEXT;
        }
        case scrOpcode::IAND:
        {
            --stackPtr;
            stackPtr[0].Int &= stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::IOR:
        {
            --stackPtr;
            stackPtr[0].Int |= stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::IXOR:
        {
            --stackPtr;
            stackPtr[0].Int ^= stackPtr[1].Int;
            goto NEXT;
        }
        case scrOpcode::I2F:
        {
            stackPtr[0].Float = static_cast<float>(stackPtr[0].Int);
            goto NEXT;
        }
        case scrOpcode::F2I:
        {
            stackPtr[0].Int = static_cast<int32_t>(stackPtr[0].Float);
            goto NEXT;
        }
        case scrOpcode::F2V:
        {
            stackPtr += 2;
            stackPtr[-1].Int = stackPtr[0].Int = stackPtr[-2].Int;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_U8:
        {
            ++stackPtr;
            stackPtr[0].Int = GET_BYTE;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_U8_U8:
        {
            stackPtr += 2;
            stackPtr[-1].Int = GET_BYTE;
            stackPtr[0].Int = GET_BYTE;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_U8_U8_U8:
        {
            stackPtr += 3;
            stackPtr[-2].Int = GET_BYTE;
            stackPtr[-1].Int = GET_BYTE;
            stackPtr[0].Int = GET_BYTE;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_U32:
        case scrOpcode::PUSH_CONST_F:
        {
            ++stackPtr;
            stackPtr[0].Uns = GET_DWORD;
            goto NEXT;
        }
        case scrOpcode::DUP:
        {
            ++stackPtr;
            stackPtr[0].Any = stackPtr[-1].Any;
            goto NEXT;
        }
        case scrOpcode::DROP:
        {
            --stackPtr;
            goto NEXT;
        }
        case scrOpcode::NATIVE:
        {
            uint8_t native = GET_BYTE;
            int32_t argCount = (native >> 2) & 0x3F;
            int32_t retCount = native & 3;

            uint8_t high = GET_BYTE;
            uint8_t low = GET_BYTE;
            scrNativeHandler handler = reinterpret_cast<scrNativeHandler>(program->GetNative((high << 8) | low));

            *context_ProgramCounter = static_cast<int32_t>(codePtr - codeBase - 4);
            *context_StackPointer = static_cast<int32_t>(stackPtr - stack + 1);
            *context_FramePointer = static_cast<int32_t>(framePtr - stack);

            scrNativeCallContext ctx(retCount ? &stack[*context_StackPointer - argCount] : nullptr, argCount, &stack[*context_StackPointer - argCount]);
            (*handler)(&ctx);

            if (scrDbgLib::ScriptLogger::ShouldLog(scrDbgLib::ScriptLogger::LogType::LOG_TYPE_NATIVE_CALLS, *context_ProgramHash))
            {
                auto hash = scrDbgLib::g_Pointers.NativeRegistrationTable->GetHashByHandler(handler);
                auto name = scrDbgShared::NativesBin::GetNameByHash(hash);

                std::string argsStr = "";
                if (argCount > 0 && ctx.m_Args)
                {
                    for (int i = 0; i < argCount; i++)
                    {
                        argsStr += std::to_string(ctx.m_Args[i].Int);
                        if (i != argCount - 1)
                            argsStr += ", ";
                    }
                }

                std::string retStr = "";
                if (retCount > 0 && ctx.m_ReturnValue)
                {
                    retStr += " -> (";
                    for (int i = 0; i < retCount; i++)
                    {
                        retStr += std::to_string(ctx.m_ReturnValue[i].Int);
                        if (i != retCount - 1)
                            retStr += ", ";
                    }
                    retStr += ")";
                }

                if (!name.empty())
                    scrDbgLib::ScriptLogger::Logf("[%s+0x%08X] %s(%s)%s", scriptThreadName, *context_ProgramCounter, name.data(), argsStr.c_str(), retStr.c_str());
                else
                    scrDbgLib::ScriptLogger::Logf("[%s+0x%08X] unk_0x%016llX(%s)%s", scriptThreadName, *context_ProgramCounter, hash, argsStr.c_str(), retStr.c_str());
            }

            // In case WAIT, TERMINATE_THIS_THREAD, or TERMINATE_THREAD is called
            if (*context_State != scrThreadState::RUNNING)
            {
                if (scrDbgLib::ScriptLogger::ShouldLog(scrDbgLib::ScriptLogger::LogType::LOG_TYPE_FRAME_TIME, *context_ProgramHash))
                {
                    auto frameEnd = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
                    scrDbgLib::ScriptLogger::Logf("[%s] %.3f ms", scriptThreadName, duration);
                }

                return *context_State;
            }

            ctx.FixVectors();

            stackPtr -= argCount - retCount;
            goto NEXT;
        }
        case scrOpcode::ENTER:
        {
            uint32_t argCount = GET_BYTE;
            uint32_t frameSize = GET_WORD;
            uint32_t nameCount = GET_BYTE;

            if (*context_CallDepth < 16)
                context_Callstack[*context_CallDepth] = static_cast<int32_t>(codePtr - codeBase + 2);
            ++*context_CallDepth;

            codePtr += nameCount;

            uint32_t current = static_cast<uint32_t>(stackPtr - stack);
            uint32_t max = *context_StackSize - frameSize;
            if (current >= max) [[unlikely]]
                return OnScriptException("Stack overflow: %u >= %u", current, max);

            (++stackPtr)->Int = static_cast<int32_t>(framePtr - stack);

            framePtr = stackPtr - argCount - 1;

            while (frameSize--)
                (++stackPtr)->Any = 0;

            stackPtr -= argCount;

            if (scrDbgLib::ScriptLogger::ShouldLog(scrDbgLib::ScriptLogger::LogType::LOG_TYPE_FUNCTION_CALLS, *context_ProgramHash))
            {
                auto funcName = scrDbgLib::ScriptFunctionNames::GetName(program, *context_ProgramCounter);

                std::string argsStr = "";
                for (int i = 0; i < argCount; i++)
                {
                    argsStr += std::to_string((framePtr + 1 + i)->Int);
                    if (i != argCount - 1)
                        argsStr += ", ";
                }

                scrDbgLib::ScriptLogger::Logf("[%s+0x%08X] %s(%s)", scriptThreadName, *context_ProgramCounter, funcName->c_str(), argsStr.c_str());
            }

            goto NEXT;
        }
        case scrOpcode::LEAVE: // TO-DO: Log function returns as well
        {
            --*context_CallDepth;

            uint32_t argCount = GET_BYTE;
            uint32_t retCount = GET_BYTE;

            scrValue* ret = stackPtr - retCount;
            stackPtr = framePtr + argCount - 1;
            framePtr = stack + stackPtr[2].Uns;

            uint32_t caller = stackPtr[1].Uns;
            JUMP(caller);
            stackPtr -= argCount;

            while (retCount--)
                *++stackPtr = *++ret;

            // Script reached end of code
            if (!caller)
                return *context_State = scrThreadState::KILLED;

            goto NEXT;
        }
        case scrOpcode::LOAD:
        {
            stackPtr[0].Any = stackPtr[0].Reference->Any;
            goto NEXT;
        }
        case scrOpcode::STORE:
        {
            stackPtr -= 2;
            scrDbgLib::ScriptVariableWriteTracker::Finalize(scriptThreadName, *context_ProgramCounter, stackPtr[1].Int);
            stackPtr[2].Reference->Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::STORE_REV:
        {
            --stackPtr;
            stackPtr[0].Reference->Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::LOAD_N:
        {
            scrValue* data = (stackPtr--)->Reference;
            uint32_t itemCount = (stackPtr--)->Int;
            for (uint32_t i = 0; i < itemCount; i++)
                (++stackPtr)->Any = data[i].Any;
            goto NEXT;
        }
        case scrOpcode::STORE_N:
        {
            scrValue* data = (stackPtr--)->Reference;
            uint32_t itemCount = (stackPtr--)->Int;

            scrDbgLib::ScriptVariableWriteTracker::Finalize(scriptThreadName, *context_ProgramCounter, itemCount, true);
            for (uint32_t i = 0; i < itemCount; i++)
                data[itemCount - 1 - i].Any = (stackPtr--)->Any;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U8:
        {
            --stackPtr;
            scrValue* ref = stackPtr[1].Reference;
            uint32_t index = stackPtr[0].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);

            uint8_t size = GET_BYTE;
            scrDbgLib::ScriptVariableWriteTracker::AddArrayIndex(index, size);

            ref += 1U + index * size;
            stackPtr[0].Reference = ref;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U8_LOAD:
        {
            --stackPtr;
            scrValue* ref = stackPtr[1].Reference;
            uint32_t index = stackPtr[0].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_BYTE;
            stackPtr[0].Any = ref->Any;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U8_STORE:
        {
            stackPtr -= 3;
            scrValue* ref = stackPtr[3].Reference;
            uint32_t index = stackPtr[2].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);

            uint8_t size = GET_BYTE;
            scrDbgLib::ScriptVariableWriteTracker::AddArrayIndex(index, size);
            scrDbgLib::ScriptVariableWriteTracker::Finalize(scriptThreadName, *context_ProgramCounter, stackPtr[1].Int);

            ref += 1U + index * size;
            ref->Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U8:
        {
            ++stackPtr;
            stackPtr[0].Reference = framePtr + GET_BYTE;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U8_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = framePtr[GET_BYTE].Any;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U8_STORE:
        {
            --stackPtr;
            framePtr[GET_BYTE].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::STATIC_U8:
        {
            ++stackPtr;
            uint8_t offset = GET_BYTE;
            scrDbgLib::ScriptVariableWriteTracker::Begin(*context_ProgramHash, offset, false);
            stackPtr[0].Reference = stack + offset;
            goto NEXT;
        }
        case scrOpcode::STATIC_U8_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = stack[GET_BYTE].Any;
            goto NEXT;
        }
        case scrOpcode::STATIC_U8_STORE:
        {
            --stackPtr;
            uint8_t _static = GET_BYTE;

            if (scrDbgLib::ScriptLogger::ShouldLog(scrDbgLib::ScriptLogger::LogType::LOG_TYPE_STATIC_WRITES, *context_ProgramHash))
                scrDbgLib::ScriptLogger::Logf("[%s+0x%08X] Static_%u = %d", scriptThreadName, *context_ProgramCounter, _static, stackPtr[1].Int);

            stack[_static].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::IADD_U8:
        {
            stackPtr[0].Int += GET_BYTE;
            goto NEXT;
        }
        case scrOpcode::IMUL_U8:
        {
            stackPtr[0].Int *= GET_BYTE;
            goto NEXT;
        }
        case scrOpcode::IOFFSET:
        {
            --stackPtr;
            stackPtr[0].Any += stackPtr[1].Int * sizeof(scrValue);
            goto NEXT;
        }
        case scrOpcode::IOFFSET_U8:
        {
            uint8_t offset = GET_BYTE;
            scrDbgLib::ScriptVariableWriteTracker::AddFieldOffset(offset);
            stackPtr[0].Any += offset * sizeof(scrValue);
            goto NEXT;
        }
        case scrOpcode::IOFFSET_U8_LOAD:
        {
            stackPtr[0].Any = stackPtr[0].Reference[GET_BYTE].Any;
            goto NEXT;
        }
        case scrOpcode::IOFFSET_U8_STORE:
        {
            stackPtr -= 2;
            uint8_t offset = GET_BYTE;
            scrDbgLib::ScriptVariableWriteTracker::AddFieldOffset(offset);
            scrDbgLib::ScriptVariableWriteTracker::Finalize(scriptThreadName, *context_ProgramCounter, stackPtr[1].Int);
            stackPtr[2].Reference[offset].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_S16:
        {
            ++stackPtr;
            int16_t offset = GET_SWORD;
            scrDbgLib::ScriptVariableWriteTracker::AddFieldOffset(offset);
            stackPtr[0].Int = offset;
            goto NEXT;
        }
        case scrOpcode::IADD_S16:
        {
            stackPtr[0].Int += GET_SWORD;
            goto NEXT;
        }
        case scrOpcode::IMUL_S16:
        {
            stackPtr[0].Int *= GET_SWORD;
            goto NEXT;
        }
        case scrOpcode::IOFFSET_S16:
        {
            int16_t offset = GET_SWORD;
            scrDbgLib::ScriptVariableWriteTracker::AddFieldOffset(offset);
            stackPtr[0].Any += offset * sizeof(scrValue);
            goto NEXT;
        }
        case scrOpcode::IOFFSET_S16_LOAD:
        {
            stackPtr[0].Any = stackPtr[0].Reference[GET_SWORD].Any;
            goto NEXT;
        }
        case scrOpcode::IOFFSET_S16_STORE:
        {
            stackPtr -= 2;
            int16_t offset = GET_SWORD;
            scrDbgLib::ScriptVariableWriteTracker::AddFieldOffset(offset);
            scrDbgLib::ScriptVariableWriteTracker::Finalize(scriptThreadName, *context_ProgramCounter, stackPtr[1].Int);
            stackPtr[2].Reference[offset].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U16:
        {
            --stackPtr;
            scrValue* ref = stackPtr[1].Reference;
            uint32_t index = stackPtr[0].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);

            uint16_t size = GET_WORD;
            scrDbgLib::ScriptVariableWriteTracker::AddArrayIndex(index, size);

            ref += 1U + index * size;
            stackPtr[0].Reference = ref;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U16_LOAD:
        {
            --stackPtr;
            scrValue* ref = stackPtr[1].Reference;
            uint32_t index = stackPtr[0].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_WORD;
            stackPtr[0].Any = ref->Any;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U16_STORE:
        {
            stackPtr -= 3;
            scrValue* ref = stackPtr[3].Reference;
            uint32_t index = stackPtr[2].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);

            uint16_t size = GET_WORD;
            scrDbgLib::ScriptVariableWriteTracker::AddArrayIndex(index, size);
            scrDbgLib::ScriptVariableWriteTracker::Finalize(scriptThreadName, *context_ProgramCounter, stackPtr[1].Int);

            ref += 1U + index * size;
            ref->Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U16:
        {
            ++stackPtr;
            stackPtr[0].Reference = framePtr + GET_WORD;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U16_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = framePtr[GET_WORD].Any;
            goto NEXT;
        }
        case scrOpcode::LOCAL_U16_STORE:
        {
            --stackPtr;
            framePtr[GET_WORD].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::STATIC_U16:
        {
            ++stackPtr;
            uint16_t offset = GET_WORD;
            scrDbgLib::ScriptVariableWriteTracker::Begin(*context_ProgramHash, offset, false);
            stackPtr[0].Reference = stack + offset;
            goto NEXT;
        }
        case scrOpcode::STATIC_U16_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = stack[GET_WORD].Any;
            goto NEXT;
        }
        case scrOpcode::STATIC_U16_STORE:
        {
            --stackPtr;
            uint16_t _static = GET_WORD;

            if (scrDbgLib::ScriptLogger::ShouldLog(scrDbgLib::ScriptLogger::LogType::LOG_TYPE_STATIC_WRITES, *context_ProgramHash))
                scrDbgLib::ScriptLogger::Logf("[%s+0x%08X] Static_%u = %d", scriptThreadName, *context_ProgramCounter, _static, stackPtr[1].Int);

            stack[_static].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U16:
        {
            ++stackPtr;
            uint16_t global = GET_WORD;
            scrDbgLib::ScriptVariableWriteTracker::Begin(*context_ProgramHash, global, true);
            stackPtr[0].Reference = globals[0] + global;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U16_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = globals[0][GET_WORD].Any;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U16_STORE:
        {
            --stackPtr;
            uint16_t global = GET_WORD;

            if (scrDbgLib::ScriptLogger::ShouldLog(scrDbgLib::ScriptLogger::LogType::LOG_TYPE_GLOBAL_WRITES, *context_ProgramHash))
                scrDbgLib::ScriptLogger::Logf("[%s+0x%08X] Global_%u = %d", scriptThreadName, *context_ProgramCounter, global, stackPtr[1].Int);

            globals[0][global].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::J:
        {
            int32_t ofs = GET_SWORD;
            JUMP(codePtr - codeBase + ofs);
            goto NEXT;
        }
        case scrOpcode::JZ:
        {
            int32_t ofs = GET_SWORD;
            --stackPtr;
            if (stackPtr[1].Int == 0)
                JUMP(codePtr - codeBase + ofs);
            else
                JUMP(codePtr - codeBase);
            goto NEXT;
        }
        case scrOpcode::IEQ_JZ:
        {
            int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int == stackPtr[2].Int))
                JUMP(codePtr - codeBase + ofs);
            else
                JUMP(codePtr - codeBase);
            goto NEXT;
        }
        case scrOpcode::INE_JZ:
        {
            int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int != stackPtr[2].Int))
                JUMP(codePtr - codeBase + ofs);
            else
                JUMP(codePtr - codeBase);
            goto NEXT;
        }
        case scrOpcode::IGT_JZ:
        {
            int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int > stackPtr[2].Int))
                JUMP(codePtr - codeBase + ofs);
            else
                JUMP(codePtr - codeBase);
            goto NEXT;
        }
        case scrOpcode::IGE_JZ:
        {
            int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int >= stackPtr[2].Int))
                JUMP(codePtr - codeBase + ofs);
            else
                JUMP(codePtr - codeBase);
            goto NEXT;
        }
        case scrOpcode::ILT_JZ:
        {
            int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int < stackPtr[2].Int))
                JUMP(codePtr - codeBase + ofs);
            else
                JUMP(codePtr - codeBase);
            goto NEXT;
        }
        case scrOpcode::ILE_JZ:
        {
            int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int <= stackPtr[2].Int))
                JUMP(codePtr - codeBase + ofs);
            else
                JUMP(codePtr - codeBase);
            goto NEXT;
        }
        case scrOpcode::CALL:
        {
            uint32_t ofs = GET_24BIT;
            ++stackPtr;
            stackPtr[0].Uns = static_cast<int32_t>(codePtr - codeBase);
            JUMP(ofs);
            goto NEXT;
        }
        case scrOpcode::STATIC_U24:
        {
            ++stackPtr;
            uint32_t offset = GET_24BIT;
            scrDbgLib::ScriptVariableWriteTracker::Begin(*context_ProgramHash, offset, false);
            stackPtr[0].Reference = stack + offset;
            goto NEXT;
        }
        case scrOpcode::STATIC_U24_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = stack[GET_24BIT].Any;
            goto NEXT;
        }
        case scrOpcode::STATIC_U24_STORE:
        {
            --stackPtr;
            uint32_t _static = GET_24BIT;

            if (scrDbgLib::ScriptLogger::ShouldLog(scrDbgLib::ScriptLogger::LogType::LOG_TYPE_STATIC_WRITES, *context_ProgramHash))
                scrDbgLib::ScriptLogger::Logf("[%s+0x%08X] Static_%u = %d", scriptThreadName, *context_ProgramCounter, _static, stackPtr[1].Int);

            stack[_static].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U24:
        {
            uint32_t global = GET_24BIT;
            uint32_t block = global >> 0x12U;
            uint32_t index = global & 0x3FFFFU;
            ++stackPtr;
            if (!globals[block]) [[unlikely]]
                return OnScriptException("Global block %u (index %u) is not loaded!", block, index);
            else
            {
                scrDbgLib::ScriptVariableWriteTracker::Begin(*context_ProgramHash, global, true);
                stackPtr[0].Reference = &globals[block][index];
            }
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U24_LOAD:
        {
            uint32_t global = GET_24BIT;
            uint32_t block = global >> 0x12U;
            uint32_t index = global & 0x3FFFFU;
            ++stackPtr;
            if (!globals[block]) [[unlikely]]
                return OnScriptException("Global block %u (index %u) is not loaded!", block, index);
            else
                stackPtr[0].Any = globals[block][index].Any;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U24_STORE:
        {
            uint32_t global = GET_24BIT;
            uint32_t block = global >> 0x12U;
            uint32_t index = global & 0x3FFFFU;
            --stackPtr;
            if (!globals[block]) [[unlikely]]
                return OnScriptException("Global block %u (index %u) is not loaded!", block, index);
            else
            {
                if (scrDbgLib::ScriptLogger::ShouldLog(scrDbgLib::ScriptLogger::LogType::LOG_TYPE_GLOBAL_WRITES, *context_ProgramHash))
                    scrDbgLib::ScriptLogger::Logf("[%s+0x%08X] Global_%u = %d", scriptThreadName, *context_ProgramCounter, global, stackPtr[1].Int);

                globals[block][index].Any = stackPtr[1].Any;
            }
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_U24:
        {
            ++stackPtr;
            uint32_t offset = GET_24BIT;
            scrDbgLib::ScriptVariableWriteTracker::AddFieldOffset(offset);
            stackPtr[0].Int = offset;
            goto NEXT;
        }
        case scrOpcode::SWITCH:
        {
            --stackPtr;
            uint32_t switchVal = stackPtr[1].Uns;
            uint32_t caseCount = GET_BYTE;
            JUMP(codePtr - codeBase);
            for (uint32_t i = 0; i < caseCount; i++)
            {
                uint32_t caseVal = GET_DWORD;
                uint32_t ofs = GET_WORD;
                if (switchVal == caseVal)
                {
                    JUMP(codePtr - codeBase + ofs);
                    break;
                }
            }
            JUMP(codePtr - codeBase);
            goto NEXT;
        }
        case scrOpcode::STRING:
        {
            uint32_t ofs = stackPtr[0].Uns;
            stackPtr[0].String = program->GetString(ofs);
            goto NEXT;
        }
        case scrOpcode::STRINGHASH:
        {
            stackPtr[0].Uns = Joaat(stackPtr[0].String);
            goto NEXT;
        }
        case scrOpcode::TEXT_LABEL_ASSIGN_STRING:
        {
            stackPtr -= 2;
            char* dest = const_cast<char*>(stackPtr[2].String);
            const char* src = stackPtr[1].String;
            scrStringAssign(dest, GET_BYTE, src);
            goto NEXT;
        }
        case scrOpcode::TEXT_LABEL_ASSIGN_INT:
        {
            stackPtr -= 2;
            char* dest = const_cast<char*>(stackPtr[2].String);
            int32_t value = stackPtr[1].Int;
            scrStringItoa(buffer, value);
            scrStringAssign(dest, GET_BYTE, buffer);
            goto NEXT;
        }
        case scrOpcode::TEXT_LABEL_APPEND_STRING:
        {
            stackPtr -= 2;
            char* dest = const_cast<char*>(stackPtr[2].String);
            const char* src = stackPtr[1].String;
            scrStringAppend(dest, GET_BYTE, src);
            goto NEXT;
        }
        case scrOpcode::TEXT_LABEL_APPEND_INT:
        {
            stackPtr -= 2;
            char* dest = const_cast<char*>(stackPtr[2].String);
            int32_t value = stackPtr[1].Int;
            scrStringItoa(buffer, value);
            scrStringAppend(dest, GET_BYTE, buffer);
            goto NEXT;
        }
        case scrOpcode::TEXT_LABEL_COPY:
        {
            stackPtr -= 3;
            scrValue* dest = stackPtr[3].Reference;
            int32_t destSize = stackPtr[2].Int;
            int32_t srcSize = stackPtr[1].Int;
            if (srcSize > destSize)
            {
                int32_t excess = srcSize - destSize;
                stackPtr -= excess;
                srcSize = destSize;
            }
            scrValue* destPtr = dest + srcSize - 1;
            for (int32_t i = 0; i < srcSize; i++)
                *destPtr-- = *stackPtr--;
            reinterpret_cast<char*>(dest)[srcSize * sizeof(scrValue) - 1] = '\0';
            goto NEXT;
        }
        case scrOpcode::CATCH:
        {
            // No need to cache these at the start as they are unused
            *context->CatchProgramCounter() = static_cast<int32_t>(codePtr - codeBase);
            *context->CatchFramePointer() = static_cast<int32_t>(framePtr - stack);
            *context->CatchStackPointer() = static_cast<int32_t>(stackPtr - stack + 1);
            ++stackPtr;
            stackPtr[0].Int = -1;
            goto NEXT;
        }
        case scrOpcode::THROW:
        {
            int32_t ofs = stackPtr[0].Int;
            if (!*context->CatchProgramCounter()) [[unlikely]]
                return OnScriptException("Catch PC is NULL!");
            else
            {
                JUMP(*context->CatchProgramCounter());
                framePtr = stack + *context->CatchFramePointer();
                stackPtr = stack + *context->CatchStackPointer();
                stackPtr[0].Int = ofs;
            }
            goto NEXT;
        }
        case scrOpcode::CALLINDIRECT:
        {
            uint32_t ofs = stackPtr[0].Uns;
            if (!ofs) [[unlikely]]
                return OnScriptException("Function pointer is NULL!");
            stackPtr[0].Uns = static_cast<int32_t>(codePtr - codeBase);
            JUMP(ofs);
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_M1:
        {
            ++stackPtr;
            stackPtr[0].Int = -1;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_0:
        {
            ++stackPtr;
            stackPtr[0].Any = 0;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_1:
        {
            ++stackPtr;
            stackPtr[0].Int = 1;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_2:
        {
            ++stackPtr;
            stackPtr[0].Int = 2;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_3:
        {
            ++stackPtr;
            stackPtr[0].Int = 3;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_4:
        {
            ++stackPtr;
            stackPtr[0].Int = 4;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_5:
        {
            ++stackPtr;
            stackPtr[0].Int = 5;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_6:
        {
            ++stackPtr;
            stackPtr[0].Int = 6;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_7:
        {
            ++stackPtr;
            stackPtr[0].Int = 7;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_FM1:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0xBF800000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F0:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x00000000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F1:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x3F800000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F2:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40000000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F3:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40400000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F4:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40800000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F5:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40A00000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F6:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40C00000;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_F7:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40E00000;
            goto NEXT;
        }
        case scrOpcode::IS_BIT_SET:
        {
            --stackPtr;
            stackPtr[0].Int = (stackPtr[0].Int & (1 << stackPtr[1].Int)) != 0;
            goto NEXT;
        }
        }

        [[unlikely]] return OnScriptException("Unknown codePtr: 0x%02X", static_cast<uint32_t>(*codePtr));
    }
}