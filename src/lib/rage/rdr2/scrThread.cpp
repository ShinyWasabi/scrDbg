#include "scrThread.hpp"
#include "ResourceLoader.hpp"
#include "debugger/VMLogger.hpp"
#include "game/RDR2.hpp"
#include "rage/shared/atArray.hpp"
#include "rage/shared/scrHash.hpp"
#include "rage/shared/scrString.hpp"
#include "scrProgram.hpp"

#if defined(_M_X64)

namespace rage::rdr2
{
    using namespace scrDbgLib;
    using namespace scrDbgShared;

    static uint64_t g_NullContainer = 0;

    scrThread* scrThread::GetByHash(uint32_t hash)
    {
        auto threads = RDR2::GetPointers().ScriptThreads;
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
        auto handlers = RDR2::GetPointers().CommandHandlers;
        if (!handlers)
            return nullptr;

        return handlers->Lookup(hash);
    }

    uint32_t scrThread::GetCommandHash(scrNativeContext::Handler handler)
    {
        auto handlers = RDR2::GetPointers().CommandHandlers;
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

        const char* scriptName = "???";
        if (!m_Context.m_Tls[0].Any != 0)
            scriptName = reinterpret_cast<const char*>(m_Context.m_Tls[0].Any + 112);

        char fullMessage[600];
        std::snprintf(fullMessage, sizeof(fullMessage), "Exception in %s at 0x%X (%s)!\nReason: %s",
            scriptName,
            pc,
            static_cast<uint8_t>(op) < 256 ? g_scrOpcodeNames[static_cast<uint8_t>(op)] : "???",
            message);
        MessageBoxA(0, fullMessage, "Exception", MB_OK | MB_ICONERROR);

        m_KillReason = fmt; // = message;
        return m_Context.m_State = State::KILLED;
    }

#define GET_U8 ((++pc), curData->GetCode(pc - 1)[0])
#define GET_U16 ((pc += 2), *reinterpret_cast<uint16_t*>(curData->GetCode(pc - 2)))
#define GET_S16 ((pc += 2), *reinterpret_cast<int16_t*>(curData->GetCode(pc - 2)))
#define GET_U24 ((pc += 3), *reinterpret_cast<uint32_t*>(curData->GetCode(pc - 4)) >> 8) // yes, this is -4, not -3
#define GET_U32 ((pc += 4), *reinterpret_cast<uint32_t*>(curData->GetCode(pc - 4)))

    scrThread::State scrThread::RunThread(scrThread* _this, int32_t insnCount)
    {
        if (_this->m_Context.m_State == State::KILLED || _this->m_Context.m_State == State::PAUSED)
            return _this->m_Context.m_State;

        scrProgram* program = scrProgram::GetByHash(_this->m_Context.m_ProgramHash);
        if (!program)
            return _this->OnException(_this->m_Context.m_Pc, scrOpcode::NOP, "Invalid program");

        auto& pointers = RDR2::GetPointers();

        scrThread* prevThread = *pointers.CurrentScriptThread;
        const char* prevThreadName = *pointers.CurrentScriptThreadName;
        *pointers.CurrentScriptThread = _this;
        *pointers.CurrentScriptThreadName = program->m_Name;

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

        scrProgram::Data* curData = nullptr;
        if (_this->m_Context.m_Patched)
            curData = program->m_PatchData;
        else
            curData = program->m_Data;

        scrValue* stack = _this->m_Stack;
        scrValue* globals = *pointers.ScriptGlobals;

        uint32_t pc = _this->m_Context.m_Pc;
        scrValue* fp = &stack[_this->m_Context.m_Fp];
        scrValue* sp = &stack[_this->m_Context.m_Sp - 1];

        _this->m_InsnCount = 0;

        while (insnCount-- > 0)
        {
            _this->m_InsnCount++;

            scrOpcode op = static_cast<scrOpcode>(*curData->GetCode(pc));

            if (op == scrOpcode::PATCH_RET)
            {
                curData = program->m_Data;
                scrProgram::PatchEntry* entry = &program->m_PatchData->m_Entries[_this->m_Context.m_PatchIndex];
                op = entry->m_OrigOp;
                pc = entry->m_EntryPc;
                _this->m_Context.m_PatchIndex = -1;
                _this->m_Context.m_Patched = false;
            }

            ++pc;

            switch (op)
            {
            case scrOpcode::NOP:
            case scrOpcode::PATCH_RET:
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
                sp -= 2;
                sp[-1].Float += sp[1].Float;
                *reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(&sp[-1]) + 4) +=
                    *reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(&sp[1]) + 4);
                sp[0].Float += sp[2].Float;
                break;
            }
            case scrOpcode::VSUB:
            {
                sp -= 2;
                sp[-1].Float -= sp[1].Float;
                *reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(&sp[-1]) + 4) -=
                    *reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(&sp[1]) + 4);
                sp[0].Float -= sp[2].Float;
                break;
            }
            case scrOpcode::VMUL:
            {
                sp -= 2;
                sp[-1].Float *= sp[1].Float;
                *reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(&sp[-1]) + 4) *=
                    *reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(&sp[1]) + 4);
                sp[0].Float *= sp[2].Float;
                break;
            }
            case scrOpcode::VDIV:
            {
                sp -= 2;
                float aY = *reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(&sp[1]) + 4);
                float* bY = reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(&sp[-1]) + 4);
                sp[-1].Float = sp[1].Float != 0.0f ? sp[-1].Float / sp[1].Float : 0.0f;
                *bY = aY != 0.0f ? *bY / aY : 0.0f;
                sp[0].Float = sp[2].Float != 0.0f ? sp[0].Float / sp[2].Float : 0.0f;
                break;
            }
            case scrOpcode::VNEG:
            {
                sp[-1].Uns ^= 0x80000000U;
                *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(&sp[-1]) + 4) ^= 0x80000000U;
                sp[0].Uns ^= 0x80000000U;
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
                ++sp;
                uint32_t x = sp[-1].Uns;
                sp[-1].Any = (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(x);
                sp[0].Int = static_cast<int32_t>(x);
                break;
            }
            case scrOpcode::PUSH_CONST_U8:
            {
                ++sp;
                sp[0].Int = GET_U8;
                break;
            }
            case scrOpcode::PUSH_CONST_U8_U8:
            {
                sp += 2;
                sp[-1].Int = GET_U8;
                sp[0].Int = GET_U8;
                break;
            }
            case scrOpcode::PUSH_CONST_U8_U8_U8:
            {
                sp += 3;
                sp[-2].Int = GET_U8;
                sp[-1].Int = GET_U8;
                sp[0].Int = GET_U8;
                break;
            }
            case scrOpcode::PUSH_CONST_U32:
            case scrOpcode::PUSH_CONST_F:
            {
                ++sp;
                sp[0].Uns = GET_U32;
                break;
            }
            case scrOpcode::DUP:
            {
                ++sp;
                sp[0].Any = sp[-1].Any;
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

                uint8_t byte1 = GET_U8;
                uint8_t byte2 = GET_U8;

                uint32_t argCount = (byte1 >> 1) & 0x1FU;
                uint32_t retCount = byte1 & 1U;
                uint32_t index = ((static_cast<uint32_t>(byte1) * 4U) & 0xFFFFFF00U) | byte2;

                scrNativeContext::Handler handler = curData->m_Natives[index];

                _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);

                scrNativeContext ctx{};
                ctx.m_Rets = retCount ? &stack[_this->m_Context.m_Sp - argCount] : nullptr;
                ctx.m_ArgCount = argCount;
                ctx.m_Args = &stack[_this->m_Context.m_Sp - argCount];
                ctx.m_VectorRefCount = 0;
                (*handler)(&ctx);

                if (_this->m_Context.m_State == State::REFRESH)
                {
                    insnCount = 1000000;
                    _this->m_Context.m_State = State::RUNNING;
                }
                else if (_this->m_Context.m_State != State::RUNNING)
                {
                    _this->m_Context.m_Pc = pc - 3;
                    return _this->m_Context.m_State;
                }

                ctx.CopyVectorsOut2();

                _this->m_Context.m_Sp += retCount - argCount;
                sp = &stack[_this->m_Context.m_Sp - 1];
                break;
            }
            case scrOpcode::ENTER:
            {
                uint8_t argCount = GET_U8;
                uint16_t frameSize = GET_U16;
                uint8_t nameLen = GET_U8;
                pc += nameLen;

                ++sp;
                sp[0].Int = _this->m_Context.m_Fp;

                uint32_t newSp = static_cast<uint32_t>(sp - stack + 1);
                _this->m_Context.m_Sp = newSp;
                _this->m_Context.m_Fp = newSp - argCount - 2;
                fp = &stack[_this->m_Context.m_Fp];

                for (uint32_t i = 0; i < frameSize; i++)
                    stack[_this->m_Context.m_Sp + i].Any = 0; // this must be .Any

                _this->m_Context.m_Sp += frameSize - argCount;
                sp = &stack[_this->m_Context.m_Sp - 1];
                break;
            }
            case scrOpcode::LEAVE:
            case scrOpcode::LEAVE_0_0:
            case scrOpcode::LEAVE_0_1:
            case scrOpcode::LEAVE_0_2:
            case scrOpcode::LEAVE_0_3:
            case scrOpcode::LEAVE_1_0:
            case scrOpcode::LEAVE_1_1:
            case scrOpcode::LEAVE_1_2:
            case scrOpcode::LEAVE_1_3:
            case scrOpcode::LEAVE_2_0:
            case scrOpcode::LEAVE_2_1:
            case scrOpcode::LEAVE_2_2:
            case scrOpcode::LEAVE_2_3:
            case scrOpcode::LEAVE_3_0:
            case scrOpcode::LEAVE_3_1:
            case scrOpcode::LEAVE_3_2:
            case scrOpcode::LEAVE_3_3:
            {
                uint32_t argCount = 0;
                uint32_t retCount = 0;
                if (op == scrOpcode::LEAVE)
                {
                    argCount = GET_U8;
                    retCount = GET_U8;
                }
                else
                {
                    uint8_t enc = static_cast<uint8_t>(op) - static_cast<uint8_t>(scrOpcode::LEAVE_0_0);
                    argCount = enc >> 2;
                    retCount = enc & 3U;
                }

                uint32_t fpOffset = argCount + _this->m_Context.m_Fp + 2;
                scrValue* framePtr = &stack[fpOffset - 2];
                _this->m_Context.m_Sp = fpOffset;
                _this->m_Context.m_Fp = framePtr[1].Uns;
                pc = framePtr[0].Uns;
                fp = &stack[_this->m_Context.m_Fp];

                if ((pc & 0xF8000000U) != 0)
                {
                    _this->m_Context.m_Patched = true;
                    _this->m_Context.m_PatchIndex = static_cast<int32_t>(pc >> 27) - 1;
                    pc &= 0x07FFFFFFU;
                    curData = program->m_PatchData;
                }
                else if (_this->m_Context.m_Patched)
                {
                    _this->m_Context.m_Patched = false;
                    _this->m_Context.m_PatchIndex = -1;
                    curData = program->m_Data;
                }

                uint32_t newSp = (framePtr - stack) - argCount;
                _this->m_Context.m_Sp = newSp;

                int32_t src = sp - stack - retCount + 1;
                for (uint8_t i = 0; i < retCount; i++)
                    stack[_this->m_Context.m_Sp++] = stack[src++];

                sp = &stack[_this->m_Context.m_Sp - 1];

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
                sp -= 2;
                sp[2].Reference->Int = sp[1].Int;
                break;
            }
            case scrOpcode::STORE_REV:
            {
                --sp;
                sp[0].Reference->Any = sp[1].Any;
                break;
            }
            case scrOpcode::LOAD_N:
            {
                scrValue* addr = sp[0].Reference;
                int32_t count = sp[-1].Int;
                sp -= 2;
                for (int32_t i = 0; i < count; i++)
                    (++sp)[0].Any = addr[i].Any;
                break;
            }
            case scrOpcode::STORE_N:
            {
                scrValue* addr = sp[0].Reference;
                int32_t count = sp[-1].Int;
                sp -= 2;
                for (int32_t i = count - 1; i >= 0; i--)
                    addr[i].Any = (sp--)[0].Any;
                break;
            }
            case scrOpcode::ARRAY_U8:
            {
                --sp;
                scrValue* addr = sp[1].Reference;
                int32_t index = sp[0].Int;
                if (index < 0 || index >= addr->Int)
                    return _this->OnException(pc, op, "Array overrun (%d >= %d)", index, addr->Int);
                sp[0].Reference = addr + 1U + index * GET_U8;
                break;
            }
            case scrOpcode::ARRAY_U8_LOAD:
            {
                --sp;
                scrValue* addr = sp[1].Reference;
                int32_t index = sp[0].Int;
                if (index < 0 || index >= addr->Int)
                    return _this->OnException(pc, op, "Array overrun (%d >= %d)", index, addr->Int);
                sp[0].Int = (addr + 1U + index * GET_U8)->Int;
                break;
            }
            case scrOpcode::ARRAY_U8_STORE:
            {
                sp -= 3;
                scrValue* addr = sp[3].Reference;
                int32_t index = sp[2].Int;
                if (index < 0 || index >= addr->Int)
                    return _this->OnException(pc, op, "Array overrun (%d >= %d)", index, addr->Int);
                (addr + 1U + index * GET_U8)->Int = sp[1].Int;
                break;
            }
            case scrOpcode::LOCAL_U8:
            {
                ++sp;
                sp[0].Reference = fp + GET_U8;
                break;
            }
            case scrOpcode::LOCAL_U8_LOAD:
            {
                ++sp;
                sp[0].Int = fp[GET_U8].Int;
                break;
            }
            case scrOpcode::LOCAL_U8_STORE:
            {
                --sp;
                fp[GET_U8].Int = sp[1].Int;
                break;
            }
            case scrOpcode::STATIC_U8:
            {
                ++sp;
                sp[0].Reference = reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(stack) + 4ULL * GET_U8);
                break;
            }
            case scrOpcode::STATIC_U8_LOAD:
            {
                ++sp;
                sp[0].Int = reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(stack) + 4ULL * GET_U8)->Int;
                break;
            }
            case scrOpcode::STATIC_U8_STORE:
            {
                --sp;
                reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(stack) + 4ULL * GET_U8)->Int = sp[1].Int;
                break;
            }
            case scrOpcode::IADD_U8:
            {
                sp[0].Any += GET_U8;
                break;
            }
            case scrOpcode::IOFFSET_U8_LOAD:
            {
                sp[0].Int = reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(sp[0].Reference) + GET_U8)->Int;
                break;
            }
            case scrOpcode::IOFFSET_U8_STORE:
            {
                sp -= 2;
                reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(sp[2].Reference) + GET_U8)->Int = sp[1].Int;
                break;
            }
            case scrOpcode::IMUL_U8:
            {
                sp[0].Int *= GET_U8;
                break;
            }
            case scrOpcode::PUSH_CONST_S16:
            {
                ++sp;
                sp[0].Int = GET_S16;
                break;
            }
            case scrOpcode::IADD_S16:
            {
                sp[0].Any += GET_S16;
                break;
            }
            case scrOpcode::IOFFSET_S16_LOAD:
            {
                sp[0].Int = reinterpret_cast<scrValue*>(sp[0].Any + GET_S16)->Int;
                break;
            }
            case scrOpcode::IOFFSET_S16_STORE:
            {
                sp -= 2;
                reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(sp[2].Reference) + GET_S16)->Int = sp[1].Int;
                break;
            }
            case scrOpcode::IMUL_S16:
            {
                sp[0].Int *= GET_S16;
                break;
            }
            case scrOpcode::ARRAY_U16:
            {
                --sp;
                scrValue* addr = sp[1].Reference;
                int32_t index = sp[0].Int;
                if (index < 0 || index >= addr->Int)
                    return _this->OnException(pc, op, "Array overrun (%d >= %d)", index, addr->Int);
                sp[0].Reference = addr + 1U + index * GET_U16;
                break;
            }
            case scrOpcode::ARRAY_U16_LOAD:
            {
                --sp;
                scrValue* addr = sp[1].Reference;
                int32_t index = sp[0].Int;
                if (index < 0 || index >= addr->Int)
                    return _this->OnException(pc, op, "Array overrun (%d >= %d)", index, addr->Int);
                sp[0].Int = (addr + 1U + index * GET_U16)->Int;
                break;
            }
            case scrOpcode::ARRAY_U16_STORE:
            {
                sp -= 3;
                scrValue* addr = sp[3].Reference;
                int32_t index = sp[2].Int;
                if (index < 0 || index >= addr->Int)
                    return _this->OnException(pc, op, "Array overrun (%d >= %d)", index, addr->Int);
                (addr + 1U + index * GET_U16)->Int = sp[1].Int;
                break;
            }
            case scrOpcode::LOCAL_U16:
            {
                ++sp;
                sp[0].Reference = fp + GET_U16;
                break;
            }
            case scrOpcode::LOCAL_U16_LOAD:
            {
                ++sp;
                sp[0].Int = fp[GET_U16].Int;
                break;
            }
            case scrOpcode::LOCAL_U16_STORE:
            {
                --sp;
                fp[GET_U16].Int = sp[1].Int;
                break;
            }
            case scrOpcode::STATIC_U16:
            {
                ++sp;
                sp[0].Reference = reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(stack) + 4ULL * GET_U16);
                break;
            }
            case scrOpcode::STATIC_U16_LOAD:
            {
                ++sp;
                sp[0].Int = reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(stack) + 4ULL * GET_U16)->Int;
                break;
            }
            case scrOpcode::STATIC_U16_STORE:
            {
                --sp;
                reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(stack) + 4ULL * GET_U16)->Int = sp[1].Int;
                break;
            }
            case scrOpcode::GLOBAL_U16:
            {
                ++sp;
                sp[0].Reference = reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(globals) + 4ULL * GET_U16);
                break;
            }
            case scrOpcode::GLOBAL_U16_LOAD:
            {
                ++sp;
                sp[0].Int = reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(globals) + 4ULL * GET_U16)->Int;
                break;
            }
            case scrOpcode::GLOBAL_U16_STORE:
            {
                --sp;
                reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(globals) + 4ULL * GET_U16)->Int = sp[1].Int;
                break;
            }
            case scrOpcode::CALL:
            case scrOpcode::CALL_U8H_1:
            case scrOpcode::CALL_U8H_2:
            case scrOpcode::CALL_U8H_3:
            case scrOpcode::CALL_U8H_4:
            case scrOpcode::CALL_U8H_5:
            case scrOpcode::CALL_U8H_6:
            case scrOpcode::CALL_U8H_7:
            case scrOpcode::CALL_U8H_8:
            case scrOpcode::CALL_U8H_9:
            case scrOpcode::CALL_U8H_A:
            case scrOpcode::CALL_U8H_B:
            case scrOpcode::CALL_U8H_C:
            case scrOpcode::CALL_U8H_D:
            case scrOpcode::CALL_U8H_E:
            case scrOpcode::CALL_U8H_F:
            {
                ++sp;
                sp[0].Uns = pc + 2;
                pc = ((static_cast<uint8_t>(op) - static_cast<uint8_t>(scrOpcode::CALL)) << 16) | GET_U16;
                break;
            }
            case scrOpcode::J:
            {
                pc += GET_S16;
                break;
            }
            case scrOpcode::JZ:
            {
                --sp;
                int16_t ofs = GET_S16;
                if (sp[1].Int == 0)
                    pc += ofs;
                break;
            }
            case scrOpcode::INE_JZ:
            {
                sp -= 2;
                int16_t ofs = GET_S16;
                if (sp[1].Int != sp[2].Int)
                    pc += ofs;
                break;
            }
            case scrOpcode::IEQ_JZ:
            {
                sp -= 2;
                int16_t ofs = GET_S16;
                if (sp[1].Int == sp[2].Int)
                    pc += ofs;
                break;
            }
            case scrOpcode::ILE_JZ:
            {
                sp -= 2;
                int16_t ofs = GET_S16;
                if (sp[1].Int <= sp[2].Int)
                    pc += ofs;
                break;
            }
            case scrOpcode::ILT_JZ:
            {
                sp -= 2;
                int16_t ofs = GET_S16;
                if (sp[1].Int < sp[2].Int)
                    pc += ofs;
                break;
            }
            case scrOpcode::IGE_JZ:
            {
                sp -= 2;
                int16_t ofs = GET_S16;
                if (sp[1].Int >= sp[2].Int)
                    pc += ofs;
                break;
            }
            case scrOpcode::IGT_JZ:
            {
                sp -= 2;
                int16_t ofs = GET_S16;
                if (sp[1].Int > sp[2].Int)
                    pc += ofs;
                break;
            }
            case scrOpcode::GLOBAL_U24:
            {
                ++sp;
                sp[0].Reference = reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(globals) + 4ULL * GET_U24);
                break;
            }
            case scrOpcode::GLOBAL_U24_LOAD:
            {
                ++sp;
                sp[0].Int = reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(globals) + 4ULL * GET_U24)->Int;
                break;
            }
            case scrOpcode::GLOBAL_U24_STORE:
            {
                --sp;
                reinterpret_cast<scrValue*>(reinterpret_cast<uint8_t*>(globals) + 4ULL * GET_U24)->Int = sp[1].Int;
                break;
            }
            case scrOpcode::PUSH_CONST_U24:
            {
                ++sp;
                sp[0].Int = GET_U24;
                break;
            }
            case scrOpcode::SWITCH:
            {
                --sp;
                uint32_t val = sp[1].Uns;
                uint32_t count = GET_U8;
                for (uint32_t i = 0; i < count; i++)
                {
                    uint32_t caseVal = GET_U32;
                    uint32_t ofs = GET_U16;
                    if (val == caseVal)
                    {
                        pc += ofs;
                        break;
                    }
                }
                break;
            }
            case scrOpcode::STRING:
            {
                ++sp;
                uint8_t len = GET_U8;
                sp[0].String = reinterpret_cast<const char*>(curData->GetCode(pc));
                pc += len;
                break;
            }
            case scrOpcode::ARRAY:
            {
                ++sp;
                uint8_t* addr = curData->GetCode(pc);
                int32_t size = *reinterpret_cast<int32_t*>(addr);
                sp[0].Reference = reinterpret_cast<scrValue*>(addr);
                pc += size + 4;
                break;
            }
            case scrOpcode::_NULL:
            {
                ++sp;
                sp[0].Any = reinterpret_cast<uint64_t>(&g_NullContainer);
                break;
            }
            case scrOpcode::TEXT_LABEL_ASSIGN_STRING:
            {
                sp -= 2;
                char* dest = const_cast<char*>(sp[2].String);
                const char* src = sp[1].String;
                scrStringAssign(dest, GET_U8, src);
                break;
            }
            case scrOpcode::TEXT_LABEL_ASSIGN_INT:
            {
                sp -= 2;
                char* dest = const_cast<char*>(sp[2].String);
                int32_t value = sp[1].Int;
                char buf[64];
                scrStringItoa(buf, value);
                scrStringAssign(dest, GET_U8, buf);
                break;
            }
            case scrOpcode::TEXT_LABEL_APPEND_STRING:
            {
                sp -= 2;
                char* dest = const_cast<char*>(sp[2].String);
                const char* src = sp[1].String;
                scrStringAppend(dest, GET_U8, src);
                break;
            }
            case scrOpcode::TEXT_LABEL_APPEND_INT:
            {
                sp -= 2;
                char* dest = const_cast<char*>(sp[2].String);
                int32_t value = sp[1].Int;
                char buf[64];
                scrStringItoa(buf, value);
                scrStringAppend(dest, GET_U8, buf);
                break;
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
                break;
            }
            case scrOpcode::CATCH:
            {
                _this->m_Context.m_CatchFp = _this->m_Context.m_Fp;
                _this->m_Context.m_CatchPc = pc;
                _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
                _this->m_Context.m_CatchSp = static_cast<uint32_t>(sp - stack + 1);
                ++sp;
                sp[0].Int = -1;
                break;
            }
            case scrOpcode::THROW:
            {
                if (_this->m_Context.m_CatchPc == 0)
                    return _this->OnException(pc, op, "THROW with no CATCH");
                int32_t val = sp[0].Int;
                _this->m_Context.m_Fp = _this->m_Context.m_CatchFp;
                fp = &stack[_this->m_Context.m_CatchFp];
                _this->m_Context.m_Sp = _this->m_Context.m_CatchSp;
                stack[_this->m_Context.m_CatchSp].Int = val;
                sp = &stack[_this->m_Context.m_CatchSp];
                pc = _this->m_Context.m_CatchPc;
                break;
            }
            case scrOpcode::CALLINDIRECT:
            {
                uint32_t target = sp[0].Uns;
                if (_this->m_Context.m_Patched)
                {
                    uint32_t retPc = pc | (static_cast<uint32_t>(_this->m_Context.m_PatchIndex + 1) << 27);
                    pc = target;
                    sp[0].Uns = retPc;
                    _this->m_Context.m_Patched = false;
                    _this->m_Context.m_PatchIndex = -1;
                    curData = program->m_Data;
                }
                else
                {
                    if (target == 0)
                        return _this->OnException(pc, op, "Attempted to call through uninitialized function pointer");
                    sp[0].Uns = pc;
                    pc = target;
                }
                break;
            }
            case scrOpcode::PUSH_CONST_M1:
            {
                ++sp;
                sp[0].Int = -1;
                break;
            }
            case scrOpcode::PUSH_CONST_0:
            {
                ++sp;
                sp[0].Any = 0;
                break;
            }
            case scrOpcode::PUSH_CONST_1:
            {
                ++sp;
                sp[0].Int = 1;
                break;
            }
            case scrOpcode::PUSH_CONST_2:
            {
                ++sp;
                sp[0].Int = 2;
                break;
            }
            case scrOpcode::PUSH_CONST_3:
            {
                ++sp;
                sp[0].Int = 3;
                break;
            }
            case scrOpcode::PUSH_CONST_4:
            {
                ++sp;
                sp[0].Int = 4;
                break;
            }
            case scrOpcode::PUSH_CONST_5:
            {
                ++sp;
                sp[0].Int = 5;
                break;
            }
            case scrOpcode::PUSH_CONST_6:
            {
                ++sp;
                sp[0].Int = 6;
                break;
            }
            case scrOpcode::PUSH_CONST_7:
            {
                ++sp;
                sp[0].Int = 7;
                break;
            }
            case scrOpcode::PUSH_CONST_FM1:
            {
                ++sp;
                sp[0].Uns = 0xBF800000U;
                break;
            }
            case scrOpcode::PUSH_CONST_F0:
            {
                ++sp;
                sp[0].Uns = 0x00000000U;
                break;
            }
            case scrOpcode::PUSH_CONST_F1:
            {
                ++sp;
                sp[0].Uns = 0x3F800000U;
                break;
            }
            case scrOpcode::PUSH_CONST_F2:
            {
                ++sp;
                sp[0].Uns = 0x40000000U;
                break;
            }
            case scrOpcode::PUSH_CONST_F3:
            {
                ++sp;
                sp[0].Uns = 0x40400000U;
                break;
            }
            case scrOpcode::PUSH_CONST_F4:
            {
                ++sp;
                sp[0].Uns = 0x40800000U;
                break;
            }
            case scrOpcode::PUSH_CONST_F5:
            {
                ++sp;
                sp[0].Uns = 0x40A00000U;
                break;
            }
            case scrOpcode::PUSH_CONST_F6:
            {
                ++sp;
                sp[0].Uns = 0x40C00000U;
                break;
            }
            case scrOpcode::PUSH_CONST_F7:
            {
                ++sp;
                sp[0].Uns = 0x40E00000U;
                break;
            }
            case scrOpcode::PATCH_TRAP_0:
            case scrOpcode::PATCH_TRAP_1:
            case scrOpcode::PATCH_TRAP_2:
            case scrOpcode::PATCH_TRAP_3:
            case scrOpcode::PATCH_TRAP_4:
            case scrOpcode::PATCH_TRAP_5:
            case scrOpcode::PATCH_TRAP_6:
            case scrOpcode::PATCH_TRAP_7:
            case scrOpcode::PATCH_TRAP_8:
            case scrOpcode::PATCH_TRAP_9:
            case scrOpcode::PATCH_TRAP_A:
            case scrOpcode::PATCH_TRAP_B:
            case scrOpcode::PATCH_TRAP_C:
            case scrOpcode::PATCH_TRAP_D:
            case scrOpcode::PATCH_TRAP_E:
            case scrOpcode::PATCH_TRAP_F:
            {
                uint8_t index = static_cast<uint8_t>(op) - static_cast<uint8_t>(scrOpcode::PATCH_TRAP_0);
                _this->m_Context.m_Patched = true;
                _this->m_Context.m_PatchIndex = index;
                curData = program->m_PatchData;
                pc = program->m_PatchData->m_Entries[index].m_EntryPc;
                break;
            }
            case scrOpcode::CALL_PATCH:
            {
                ++sp;
                sp[0].Uns = (pc + 3) | (static_cast<uint32_t>(_this->m_Context.m_PatchIndex + 1) << 27);
                pc = GET_U24;
                break;
            }
            case scrOpcode::CALL_OUT_OF_PATCH:
            {
                ++sp;
                sp[0].Uns = (pc + 3) | (static_cast<uint32_t>(_this->m_Context.m_PatchIndex + 1) << 27);
                pc = GET_U24;
                _this->m_Context.m_Patched = false;
                _this->m_Context.m_PatchIndex = -1;
                curData = program->m_Data;
                break;
            }
            case scrOpcode::REF_LOAD:
            {
                sp[0].Any = sp[0].Reference->Any;
                break;
            }
            case scrOpcode::REF_STORE:
            {
                sp -= 2;
                sp[2].Reference->Any = sp[1].Any;
                break;
            }
            case scrOpcode::VECTOR_STORE:
            {
                sp -= 3;
                scrValue* dst = sp[3].Reference;
                dst[0].Any = sp[1].Any;
                dst[1].Uns = sp[2].Uns;
                break;
            }
            case scrOpcode::VECTOR_MAKE:
            {
                --sp;
                uint32_t y = sp[0].Uns;
                sp[0].Uns = sp[1].Uns;
                sp[-1].Any = (sp[-1].Any & 0xFFFFFFFFULL) | (static_cast<uint64_t>(y) << 32);
                break;
            }
            default:
            {
                return _this->OnException(pc, op, "Invalid opcode (%02X)", static_cast<uint8_t>(op));
            }
            }
        }

        _this->m_Context.m_Sp = static_cast<uint32_t>(sp - stack + 1);
        _this->m_Context.m_Pc = pc;
        return State::RUNNING;
    }
}

#endif