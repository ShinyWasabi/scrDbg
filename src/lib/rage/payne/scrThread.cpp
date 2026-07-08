#include "scrThread.hpp"
#include "ResourceLoader.hpp"
#include "debugger/VMLogger.hpp"
#include "game/Payne.hpp"
#include "rage/shared/atArray.hpp"
#include "rage/shared/scrHash.hpp"
#include "rage/shared/scrString.hpp"
#include "scrProgram.hpp"

#if defined(_M_IX86)

namespace rage::payne
{
    using namespace scrDbgLib;
    using namespace scrDbgShared;

    static int32_t g_NullContainer = 0;

    static std::string FormatNativeTypes(scrValue value, scrThread* thread, scrProgram* program, scrValue* globals, NativesBin::NativeTypes type)
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
        {
            auto str = reinterpret_cast<const char*>(thread->ResolveAddress(&value, program, globals));
            return str ? "\"" + std::string(str) + "\"" : "NULL";
        }
        case NativesBin::NativeTypes::REFERENCE:
        {
            auto ref = thread->ResolveAddress(&value, program, globals)->Reference;
            return "0x" + std::to_string(reinterpret_cast<uintptr_t>(ref));
        }
        }

        return std::to_string(value.Int);
    }

    scrThread* scrThread::GetByHash(uint32_t hash)
    {
        auto threads = Payne::GetPointers().ScriptThreads;
        if (!threads)
            return nullptr;

        for (auto& thread : *threads)
        {
            if (thread && thread->m_Context.m_Id != 0 && thread->m_Context.m_ProgramHash == hash)
                return thread;
        }

        return nullptr;
    }

    scrCommand* scrThread::GetCommand(uint32_t hash)
    {
        auto commands = Payne::GetPointers().Commands;
        if (!commands)
            return nullptr;

        return commands->Lookup(hash);
    }

    uint32_t scrThread::GetCommandHash(scrCommand* command)
    {
        auto commands = Payne::GetPointers().Commands;
        if (!commands)
            return 0;

        return commands->LookupHash(command);
    }

    scrValue* scrThread::ResolveAddress(scrValue* ref, scrProgram* program, scrValue* globals)
    {
        uint32_t encoded = ref->Uns;
        RefType type = static_cast<RefType>((encoded >> 29) & 7);
        uint32_t offset = encoded & 0x1FFFFFFFU;

        switch (type)
        {
        case RefType::LOCAL:
        case RefType::STATIC:
            return reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(m_Stack) + offset);
        case RefType::GLOBAL:
            return reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(globals) + offset);
        case RefType::STRING:
            return reinterpret_cast<scrValue*>(program->m_Code + offset);
        case RefType::REFERENCE:
            return reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(m_Stack) + offset)->Reference;
        }

        return reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(&g_NullContainer) + offset);
    }

    scrValue* scrThread::PopStack()
    {
        if (m_Context.m_Sp > 0)
            return &m_Stack[--m_Context.m_Sp];

        return nullptr;
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

// we don't actually need this since GrowStack is unused, but better to keep
#define ENSURE_STACK(count)                                                         \
    do                                                                              \
    {                                                                               \
        if (sp + count >= stackEnd)                                                 \
        {                                                                           \
            _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);          \
            if (!_this->GrowStack(static_cast<int32_t>(_this->m_Context.m_Sp + 1))) \
            {                                                                       \
                _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);      \
                _this->m_Context.m_Pc = pc;                                         \
                return _this->OnException(pc, op, "Stack overflow!");               \
            }                                                                       \
            stack = _this->m_Stack;                                                 \
            sp = &stack[_this->m_Context.m_Sp - 1];                                 \
            stackEnd = &stack[_this->m_Context.m_StackSize];                        \
        }                                                                           \
    } while (0)

    scrThread::State scrThread::RunThread(scrThread* _this, void* edx, int32_t insnCount)
    {
        if (_this->m_Context.m_State == State::KILLED || _this->m_Context.m_State == State::PAUSED)
            return _this->m_Context.m_State;

        auto& pointers = Payne::GetPointers();
        Debugger* debugger = g_Game->GetDebugger();

        scrThread* prevThread = *pointers.CurrentScriptThread;
        const char* prevThreadName = *pointers.CurrentScriptThreadName;
        *pointers.CurrentScriptThread = _this;
        *pointers.CurrentScriptThreadName = _this->GetScriptName2();

        struct ThreadRestore
        {
            scrThread** Current;
            scrThread* Previous;
            const char** CurrentName;
            const char* PreviousName;

            ~ThreadRestore()
            {
                *Current = Previous;
                *CurrentName = PreviousName;
            }
        } restore{pointers.CurrentScriptThread, prevThread, pointers.CurrentScriptThreadName, prevThreadName};

        scrProgram* program = scrProgram::GetProgram(_this->m_Context.m_ProgramHash);
        if (!program)
            return _this->m_Context.m_State = State::KILLED;

        auto frameStart = std::chrono::high_resolution_clock::now();

        uint8_t* code = program->m_Code;
        scrValue* stack = _this->m_Stack;
        scrValue* globals = *pointers.ScriptGlobals;

        uint32_t pc = _this->m_Context.m_Pc;
        scrValue* sp = &stack[_this->m_Context.m_Sp - 1];
        scrValue* stackEnd = &stack[_this->m_Context.m_StackSize];

        _this->m_InsnCount = 0;

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
                ENSURE_STACK(2);
                sp += 2;
                sp[-1].Int = sp[0].Int = sp[-2].Int;
                break;
            }
            case scrOpcode::PUSH_CONST_S16:
            {
                ENSURE_STACK(1);
                ++sp;
                sp[0].Int = *reinterpret_cast<int16_t*>(&code[pc]);
                pc += 2;
                break;
            }
            case scrOpcode::PUSH_CONST_U32:
            case scrOpcode::PUSH_CONST_F:
            {
                ENSURE_STACK(1);
                ++sp;
                sp[0].Uns = *reinterpret_cast<uint32_t*>(&code[pc]);
                pc += 4;
                break;
            }
            case scrOpcode::DUP:
            {
                ENSURE_STACK(1);
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
                scrCommand* command = *reinterpret_cast<scrCommand**>(&code[pc]);
                pc += 4;

                _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);

                scrCommand::Context ctx{};
                ctx.m_Thread = _this;
                ctx.m_ArgCount = argCount;
                ctx.m_Args = &stack[_this->m_Context.m_Sp - argCount];
                ctx.m_Rets = retCount ? &stack[_this->m_Context.m_Sp - argCount] : nullptr;
                ctx.m_Command = command;
                ctx.m_VectorRefCount = 0;

                uint32_t hash{};
                std::string_view name{};
                std::string argsStr{};
                std::string retsStr{};
                bool shouldLog = VMLogger::ShouldLog(VMLogType::NATIVE_CALLS, _this->m_Context.m_ProgramHash);

                // log args before calling the native because rets will be written to the same stack slot
                if (shouldLog)
                {
                    hash = GetCommandHash(command);
                    name = NativesBin::GetNameByHash(hash);

                    auto args = NativesBin::GetArgsByHash(hash);
                    if (argCount > 0 && ctx.m_Args && args && !args->empty())
                    {
                        for (int32_t i = 0; i < argCount; i++)
                        {
                            auto type = (i < args->size()) ? (*args)[i] : NativesBin::NativeTypes::NONE;
                            argsStr += FormatNativeTypes(ctx.m_Args[i], _this, program, globals, type);

                            if (i < argCount - 1)
                                argsStr += ", ";
                        }
                    }
                }

                if ((_this->m_Context.m_TypedFlags & 3) == 2 && (command->m_Flags & 4) != 0)
                    _this->InvokeSynchronizedCommand(command, &ctx, retCount);
                else
                    command->m_Handler(&ctx);

                if (shouldLog)
                {
                    auto rets = NativesBin::GetRetsByHash(hash);
                    if (retCount > 0 && ctx.m_Rets && rets && !rets->empty())
                    {
                        retsStr += " -> (";
                        for (int32_t i = 0; i < retCount; i++)
                        {
                            auto type = (i < rets->size()) ? (*rets)[i] : NativesBin::NativeTypes::NONE;
                            retsStr += FormatNativeTypes(ctx.m_Rets[i], _this, program, globals, type);

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
                    insnCount = 1000000;
                    _this->m_Context.m_State = State::RUNNING;
                }
                else if (_this->m_Context.m_State == State::THROW)
                {
                    scrValue* exceptionVal = _this->PopStack();
                    if (!exceptionVal)
                        return _this->OnException(pc, op, "Stack underflow!");

                    if (uint32_t catchPc = _this->m_Context.m_CatchPc)
                    {
                        pc = catchPc;
                        _this->m_Context.m_Fp = _this->m_Context.m_CatchFp;
                        _this->m_Context.m_Sp = _this->m_Context.m_CatchSp;

                        stack = _this->m_Stack;
                        sp = &stack[_this->m_Context.m_Sp]; // sp points to the slot we're about to write
                        ENSURE_STACK(1);
                        sp[0].Int = exceptionVal->Int;

                        return _this->m_Context.m_State = State::RUNNING;
                    }
                    else
                    {
                        _this->m_Context.m_Pc = pc;
                        return _this->OnException(pc, op, "Native threw with no CATCH block");
                    }
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
                ENSURE_STACK(1);
                ++sp;
                sp[0].Int = retPc;
                break;
            }
            case scrOpcode::ENTER:
            {
                uint8_t argCount = code[pc++];
                uint16_t localCount = *reinterpret_cast<uint16_t*>(&code[pc]);
                pc += 2;
                uint8_t nameLen = code[pc++];
                pc += nameLen;

                ENSURE_STACK(1);
                ++sp;
                sp[0].Int = _this->m_Context.m_Fp;

                uint32_t callsitePc = sp[-1].Uns - 5;

                uint32_t newSp = static_cast<uint32_t>(sp - stack + 1);
                _this->m_Context.m_Sp = newSp;
                _this->m_Context.m_Fp = newSp - argCount - 2;

                if (sp + localCount >= stackEnd)
                {
                    if (!_this->GrowStack(static_cast<int32_t>(newSp + localCount)))
                    {
                        _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
                        _this->m_Context.m_Pc = pc;
                        return _this->OnException(pc, op, "Script thread stackoverflow!");
                    }

                    stack = _this->m_Stack;
                    sp = &stack[_this->m_Context.m_Sp - 1];
                    stackEnd = &stack[_this->m_Context.m_StackSize];
                }

                for (uint32_t i = 0; i < localCount; i++)
                    stack[_this->m_Context.m_Sp + i].Int = 0;

                _this->m_Context.m_Sp += localCount - argCount;
                sp = &stack[_this->m_Context.m_Sp - 1];

                if (VMLogger::ShouldLog(VMLogType::FUNCTION_CALLS, _this->m_Context.m_ProgramHash))
                {
                    auto funcName = program->GetFuncName(pc - nameLen - 5, nameLen);

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
                sp[0].Int = _this->ResolveAddress(sp, program, globals)->Int;
                break;
            }
            case scrOpcode::STORE:
            {
                scrValue* addr = _this->ResolveAddress(sp, program, globals);
                int32_t val = sp[-1].Int;
                sp -= 2;
                addr->Int = val;
                debugger->FinalizeTracking(_this->m_ScriptName, pc - 1, val, false);
                break;
            }
            case scrOpcode::STORE_REV:
            {
                int32_t val = sp[0].Int;
                scrValue* addr = _this->ResolveAddress(sp - 1, program, globals);
                --sp;
                addr->Int = val;
                break;
            }
            case scrOpcode::LOAD_N:
            {
                scrValue* addr = _this->ResolveAddress(sp, program, globals);
                int32_t count = sp[-1].Int;
                sp -= 2;

                if (sp + count >= stackEnd)
                {
                    _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
                    if (!_this->GrowStack(static_cast<int32_t>(_this->m_Context.m_Sp + count)))
                    {
                        _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
                        _this->m_Context.m_Pc = pc;
                        return _this->OnException(pc, op, "Script thread stackoverflow!");
                    }

                    stack = _this->m_Stack;
                    sp = &stack[_this->m_Context.m_Sp - 1];
                    stackEnd = &stack[_this->m_Context.m_StackSize];
                }

                for (int32_t i = 0; i < count; i++)
                    (++sp)[0].Int = addr[i].Int;
                break;
            }
            case scrOpcode::STORE_N:
            {
                scrValue* addr = _this->ResolveAddress(sp, program, globals);
                int32_t count = sp[-1].Int;
                sp -= 2;
                for (int32_t i = count - 1; i >= 0; i--)
                    addr[i].Int = (sp--)[0].Int;
                debugger->FinalizeTracking(_this->m_ScriptName, pc - 1, count, true);
                break;
            }
            case scrOpcode::LOCAL_U8_0:
            case scrOpcode::LOCAL_U8_1:
            case scrOpcode::LOCAL_U8_2:
            case scrOpcode::LOCAL_U8_3:
            case scrOpcode::LOCAL_U8_4:
            case scrOpcode::LOCAL_U8_5:
            case scrOpcode::LOCAL_U8_6:
            case scrOpcode::LOCAL_U8_7:
            {
                uint32_t slot = static_cast<uint8_t>(op) - static_cast<uint8_t>(scrOpcode::LOCAL_U8_0);
                ENSURE_STACK(1);
                (++sp)[0].Uns = static_cast<uint32_t>(AddrType::LOCAL) | ((_this->m_Context.m_Fp + slot) * sizeof(scrValue));
                break;
            }
            case scrOpcode::LOCAL:
            {
                uint32_t slot = static_cast<uint32_t>(sp[0].Int);
                sp[0].Uns = static_cast<uint32_t>(AddrType::LOCAL) | ((_this->m_Context.m_Fp + slot) * sizeof(scrValue));
                break;
            }
            case scrOpcode::STATIC:
            {
                debugger->BeginTracking(_this->m_Context.m_ProgramHash, sp[0].Int, false);
                uint32_t idx = static_cast<uint32_t>(sp[0].Int);
                sp[0].Uns = static_cast<uint32_t>(AddrType::STATIC) | (idx * sizeof(scrValue));
                break;
            }
            case scrOpcode::GLOBAL:
            {
                debugger->BeginTracking(_this->m_Context.m_ProgramHash, sp[0].Int, true);
                uint32_t idx = static_cast<uint32_t>(sp[0].Int);
                sp[0].Uns = static_cast<uint32_t>(AddrType::GLOBAL) | (idx * sizeof(scrValue));
                break;
            }
            case scrOpcode::ARRAY:
            {
                uint32_t index = sp[-2].Uns;
                int32_t itemSize = sp[-1].Int;
                scrValue encoded = sp[0];
                sp -= 2;

                uint32_t size = _this->ResolveAddress(&encoded, program, globals)->Int;
                if (index < 0 || index >= size)
                {
                    _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
                    _this->m_Context.m_Pc = pc;
                    return _this->OnException(pc, op, "Array overrun: %d >= %d", index, size);
                }

                debugger->AddArrayIndex(sp[0].Int, size);
                sp[0].Uns = encoded.Uns + static_cast<uint32_t>((1 + index * itemSize) * sizeof(scrValue));
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
                uint32_t len = code[pc++];
                if (len == 0)
                {
                    len = static_cast<uint32_t>(code[pc]) | (static_cast<uint32_t>(code[pc + 1]) << 8);
                    pc += 2;
                }
                ENSURE_STACK(1);
                (++sp)[0].Uns = pc | static_cast<uint32_t>(AddrType::STRING);
                pc += len;
                break;
            }
            case scrOpcode::_NULL:
            {
                ENSURE_STACK(1);
                (++sp)[0].Int = 0;
                break;
            }
            case scrOpcode::TEXT_LABEL_ASSIGN_STRING:
            {
                scrValue* destRef = sp;
                scrValue* srcRef = sp - 1;
                uint8_t capacity = code[pc++];
                sp -= 2;

                char* dest = reinterpret_cast<char*>(_this->ResolveAddress(destRef, program, globals));
                const char* src = reinterpret_cast<const char*>(_this->ResolveAddress(srcRef, program, globals));

                scrStringAssign(dest, capacity, src);
                break;
            }
            case scrOpcode::TEXT_LABEL_ASSIGN_INT:
            {
                scrValue* destRef = sp;
                int32_t val = sp[-1].Int;
                uint8_t capacity = code[pc++];
                sp -= 2;

                char* dest = reinterpret_cast<char*>(_this->ResolveAddress(destRef, program, globals));

                char buf[32];
                scrStringItoa(buf, val);
                scrStringAssign(dest, capacity, buf);
                break;
            }
            case scrOpcode::TEXT_LABEL_APPEND_STRING:
            {
                scrValue* destRef = sp;
                scrValue* srcRef = sp - 1;
                uint8_t capacity = code[pc++];
                sp -= 2;

                char* dest = reinterpret_cast<char*>(_this->ResolveAddress(destRef, program, globals));
                const char* src = reinterpret_cast<const char*>(_this->ResolveAddress(srcRef, program, globals));

                scrStringAppend(dest, capacity, src);
                break;
            }
            case scrOpcode::TEXT_LABEL_APPEND_INT:
            {
                scrValue* destRef = sp;
                int32_t val = sp[-1].Int;
                uint8_t capacity = code[pc++];
                sp -= 2;

                char* dest = reinterpret_cast<char*>(_this->ResolveAddress(destRef, program, globals));

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
                ENSURE_STACK(1);
                (++sp)[0].Int = -1; // exception slot, -1 = no exception yet
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
                ENSURE_STACK(1);
                sp[0].Int = val;
                break;
            }
            case scrOpcode::TEXT_LABEL_COPY:
            {
                scrValue* destRef = sp;
                int32_t count = sp[-1].Int;
                int32_t srcCount = sp[-2].Int;
                sp -= 3;

                uint32_t* dest = reinterpret_cast<uint32_t*>(_this->ResolveAddress(destRef, program, globals));

                if (srcCount > count)
                {
                    sp -= (srcCount - count);
                    srcCount = count;
                }

                for (int32_t i = srcCount - 1; i >= 0; i--)
                    dest[i] = (sp--)[0].Int;

                reinterpret_cast<char*>(dest)[(4 * srcCount) - 1] = 0;
                break;
            }
            case scrOpcode::CALLINDIRECT:
            {
                uint32_t target = sp[0].Uns;
                sp[0].Uns = pc;
                pc = target;
                break;
            }
            default:
            {
                if (op >= scrOpcode::PUSH_CONST_M16 && op <= scrOpcode::PUSH_CONST_159)
                {
                    ENSURE_STACK(1);
                    (++sp)[0].Int = static_cast<uint8_t>(op) - 96;
                }
                else
                {
                    _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
                    _this->m_Context.m_Pc = pc;
                    return _this->OnException(pc, op, "Invalid opcode: %02X", op);
                }
                break;
            }
            }
        }

        _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
        _this->m_Context.m_Pc = pc;
        return State::RUNNING;
    }
}

#endif