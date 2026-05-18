#include "scrThread.hpp"
#include "core/Memory.hpp"
#include "scrProgram.hpp"
#include "scrString.hpp"

namespace rage
{
    bool scrThread::Init()
    {
        if (auto addr = Memory::ScanPattern("39 35 ? ? ? ? 75 ? C7 46"))
            sm_CurrentThread = *addr->Add(2).As<scrThread***>();

        if (auto addr = Memory::ScanPattern("8B 15 ? ? ? ? 0F B7 C7"))
            sm_Threads = *addr->Add(2).As<atArray<scrThread*>**>();

        if (auto addr = Memory::ScanPattern("A1 ? ? ? ? ? ? ? 5F B0"))
            sm_Stacks = *addr->Add(1).As<atArray<ThreadStack*>**>();

        if (auto addr = Memory::ScanPattern("C7 05 ? ? ? ? 00 00 00 00 66 3B C2"))
            sm_CommandHandlers = *addr->Add(2).As<scrHash<NativeContext::Handler>**>();

        if (!sm_CurrentThread || !sm_Threads || !sm_Stacks || !sm_CommandHandlers)
            return false;

        return true;
    }

    scrThread* scrThread::GetThreadByHash(uint32_t hash)
    {
        if (!sm_Threads)
            return nullptr;

        for (auto& thread : *sm_Threads)
        {
            if (thread && thread->m_Context.m_Id != 0 && thread->m_Context.m_ProgramHash == hash)
                return thread;
        }

        return nullptr;
    }

    scrThread* scrThread::GetThreadById(uint32_t id)
    {
        if (!sm_Threads)
            return nullptr;

        for (auto& thread : *sm_Threads)
        {
            if (thread && thread->m_Context.m_Id == id)
                return thread;
        }

        return nullptr;
    }

    scrThread::ThreadStack* scrThread::GetThreadStack(uint32_t stackSize)
    {
        if (!sm_Stacks)
            return nullptr;

        for (auto& stack : *sm_Stacks)
        {
            if (stack && stack->m_StackSize == stackSize)
                return stack;
        }

        return nullptr;
    }

    scrThread::NativeContext::Handler scrThread::GetCommandHandler(uint32_t hash)
    {
        if (!sm_CommandHandlers)
            return nullptr;

        return sm_CommandHandlers->Lookup(hash);
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
            static_cast<uint8_t>(op) < 80 ? g_scrOpcodeNames[static_cast<uint8_t>(op)] : "PUSH_CONST_8",
            message);
        MessageBoxA(0, fullMessage, "Exception", MB_OK | MB_ICONERROR);

        m_KillReason = fmt; // = message;
        return m_Context.m_State = State::KILLED;
    }

    scrThread::State __fastcall scrThread::RunThread(scrThread* _this, void* edx, int32_t insnCount)
    {
        if (_this->m_Context.m_State == State::KILLED || _this->m_Context.m_State == State::PAUSED)
            return _this->m_Context.m_State;

        scrThread* prevThread = *sm_CurrentThread;
        *sm_CurrentThread = _this;

        struct ThreadRestore
        {
            scrThread** Current;
            scrThread* Previous;

            ~ThreadRestore()
            {
                *Current = Previous;
            }
        } restore{sm_CurrentThread, prevThread};

        scrProgram* program = scrProgram::GetProgram(_this->m_Context.m_ProgramHash);
        if (!program || program->m_NameHash == "remindermp"_J) // We use hash comparison here which is ~36x faster than R*'s _stricmp check from what I tested
            return _this->m_Context.m_State = State::KILLED;

        uint8_t* code = program->m_Code;
        scrValue* stack = _this->m_Stack;
        scrValue* globals = *scrProgram::sm_Globals;

        uint32_t pc = _this->m_Context.m_Pc;
        scrValue* sp = &_this->m_Stack[_this->m_Context.m_Sp - 1];

        while (insnCount-- > 0)
        {
            scrOpcode op = static_cast<scrOpcode>(code[pc++]);

            switch (op)
            {
            case scrOpcode::NOP:
            {
                break;
            }
            case scrOpcode::IADD:
            {
                --sp;
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
                    sp[0].Float = y ? x - (static_cast<int32_t>(x / y) * y) : 0.0f;
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
            case scrOpcode::PUSH_CONST_U16:
            {
                ++sp;
                sp[0].Uns = *reinterpret_cast<uint16_t*>(&code[pc]);
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

                NativeContext::Handler handler = reinterpret_cast<NativeContext::Handler>(nativeAddr);
                if (!handler)
                    return _this->OnException(pc, op, "Missing native command");

                _this->m_Context.m_Sp = static_cast<uint32_t>((sp - stack) + 1);

                NativeContext ctx{};
                ctx.m_Args = &stack[_this->m_Context.m_Sp - argCount];
                ctx.m_ArgCount = argCount;
                ctx.m_Rets = retCount ? &stack[_this->m_Context.m_Sp - argCount] : nullptr;
                ctx.m_VectorRefCount = 0;
                (*handler)(&ctx);

                if (_this->m_Context.m_State == State::REFRESH)
                {
                    insnCount = 1100000;
                    _this->m_Context.m_State = State::RUNNING;
                }
                else if (_this->m_Context.m_State != State::RUNNING)
                {
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

                uint32_t newSp = static_cast<uint32_t>((sp - stack) + 1);
                _this->m_Context.m_Sp = newSp;
                _this->m_Context.m_Fp = newSp - argCount - 2;

                for (uint32_t i = 0; i < localCount; i++)
                    stack[_this->m_Context.m_Sp + i].Int = 0;

                _this->m_Context.m_Sp += localCount - argCount;
                sp = &stack[_this->m_Context.m_Sp - 1];
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
                    return _this->m_Context.m_State = State::KILLED;
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
                sp[0].Reference = &stack[sp[0].Int];
                break;
            }
            case scrOpcode::GLOBAL:
            {
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
                (++sp)[0].Reference = reinterpret_cast<scrValue*>(&sm_NullContainer);
                break;
            }
            case scrOpcode::TEXT_LABEL_ASSIGN_STRING:
            {
                char* dest = const_cast<char*>(sp[0].String);
                const char* src = sp[-1].String;
                uint8_t capacity = code[pc++];
                sp -= 2;

                scrAssignString(dest, capacity, src);
                break;
            }
            case scrOpcode::TEXT_LABEL_ASSIGN_INT:
            {
                char* dest = const_cast<char*>(sp[0].String);
                int32_t val = sp[-1].Int;
                uint8_t capacity = code[pc++];
                sp -= 2;

                char buf[32];
                scrItoa(buf, val);
                scrAssignString(dest, capacity, buf);
                break;
            }
            case scrOpcode::TEXT_LABEL_APPEND_STRING:
            {
                char* dest = const_cast<char*>(sp[0].String);
                const char* src = sp[-1].String;
                uint8_t capacity = code[pc++];
                sp -= 2;

                scrAppendString(dest, capacity, src);
                break;
            }
            case scrOpcode::TEXT_LABEL_APPEND_INT:
            {
                char* dest = const_cast<char*>(sp[0].String);
                int32_t val = sp[-1].Int;
                uint8_t capacity = code[pc++];
                sp -= 2;

                char buf[32];
                scrItoa(buf, val);
                scrAppendString(dest, capacity, buf);
                break;
            }
            case scrOpcode::CATCH:
            {
                _this->m_Context.m_CatchFp = _this->m_Context.m_Fp;
                _this->m_Context.m_CatchPc = pc;
                _this->m_Context.m_CatchSp = static_cast<uint32_t>((sp - stack) + 1);
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
                if (addr < globals || addr >= &globals[*scrProgram::sm_GlobalsCount])
                {
                    isGlobal = false;
                    protectedStorage = _this->m_ProtectedStack;
                }
                else
                {
                    isGlobal = true;
                    protectedStorage = *scrProgram::sm_ProtectedGlobals;
                }

                uint32_t slotIndex = addr->Uns;
                bool freshSlot = false;
                if (slotIndex == 0)
                {
                    freshSlot = true;
                    if (isGlobal)
                    {
                        slotIndex = scrProgram::sm_GetNextProtectedSlot();
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
                if (addr < globals || addr >= &globals[*scrProgram::sm_GlobalsCount])
                {
                    isGlobal = false;
                    protectedStorage = _this->m_ProtectedStack;
                }
                else
                {
                    isGlobal = true;
                    protectedStorage = *scrProgram::sm_ProtectedGlobals;
                }

                uint32_t slotIndex = addr->Uns;
                if (slotIndex == 0)
                {
                    if (isGlobal)
                    {
                        slotIndex = scrProgram::sm_GetNextProtectedSlot();
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
                    reinterpret_cast<scrValue*>(protectedStorage + slotIndex)->Int = value;
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
                if (addr < globals || addr >= &globals[*scrProgram::sm_GlobalsCount])
                {
                    isGlobal = false;
                    protectedStorage = _this->m_ProtectedStack;
                }
                else
                {
                    isGlobal = true;
                    protectedStorage = *scrProgram::sm_ProtectedGlobals;
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
                                    slotIndex = scrProgram::sm_GetNextProtectedSlot();
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
                                    slotIndex = scrProgram::sm_GetNextProtectedSlot();
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
                goto exit;
            }
            default:
            {
                uint8_t raw = static_cast<uint8_t>(op);
                if (raw >= 80)
                {
                    (++sp)[0].Int = raw - 96;
                }
                else
                {
                    return _this->OnException(pc, op, "Invalid opcode: %02X", raw);
                }
                break;
            }
            }
        }

    exit:
        _this->m_Context.m_Pc = pc;
        _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
        return State::RUNNING;
    }
}