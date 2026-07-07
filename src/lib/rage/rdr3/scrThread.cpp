#include "scrThread.hpp"
#include "ResourceLoader.hpp"
#include "debugger/VMLogger.hpp"
#include "game/RDR3.hpp"
#include "rage/shared/atArray.hpp"
#include "rage/shared/scrNativeContext.hpp"
#include "rage/shared/scrString.hpp"
#include "scrNativeRegistration.hpp"
#include "scrProgram.hpp"

#if defined(_M_X64)

namespace rage::rdr3
{
    using namespace scrDbgLib;
    using namespace scrDbgShared;

    scrThread* scrThread::GetByHash(uint32_t hash)
    {
        auto threads = RDR3::GetPointers().ScriptThreads;
        if (!threads)
            return nullptr;

        for (auto& thread : *threads)
        {
            if (thread && thread->m_Context.m_Id != 0 && thread->m_Context.m_ProgramHash == hash)
                return thread;
        }

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
            "TODO",
            pc,
            static_cast<uint8_t>(op) < 256 ? g_scrOpcodeNames[static_cast<uint8_t>(op)] : "???",
            message);
        MessageBoxA(0, fullMessage, "Exception", MB_OK | MB_ICONERROR);

        // m_KillReason = fmt; // = message;
        return State::KILLED;
    }

    void scrThread::ResolveSecureScriptVariable(scrValue* ref, scrValue* outHandle, Context* context, scrValue* stack, scrProgram* program)
    {
        auto& pointers = RDR3::GetPointers();
        if (ref < stack || ref > &stack[context->m_Sp - 1])
            pointers.ResolveSecureScriptGlobal(ref, outHandle);
        else if (ref >= &stack[program->m_StaticCount])
            pointers.ResolveSecureScriptFrame(ref, outHandle, context);
        else
            pointers.ResolveSecureScriptStatic(ref, outHandle, context);
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

    scrThread::State scrThread::RunThread(scrValue* stack, scrValue** globals, bool* globalBlocksEnabled, scrProgram* program, Context* context)
    {
        auto& pointers = RDR3::GetPointers();

        char buffer[16];
        uint8_t* code;
        uint8_t* base;

        scrValue* sp = stack + context->m_Sp - 1;
        scrValue* fp = stack + context->m_Fp;

        JUMP(context->m_Pc);

    NEXT:
        uint32_t pc = static_cast<uint32_t>(code - base);
        scrOpcode op = static_cast<scrOpcode>(GET_U8);

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
            (*handler)(&ctx);

            if (context->m_State != State::RUNNING)
                return context->m_State;

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
                return OnException(pc, op, "Stack overflow: %u >= %u", current, max);

            (++sp)->Int = static_cast<int32_t>(fp - stack);

            fp = sp - argCount - 1;
            while (frameSize--)
                (++sp)->Any = 0;

            sp -= argCount;

            context->m_SecureFrames[context->m_CallDepth - 1] = nullptr;
            goto NEXT;
        }
        case scrOpcode::LEAVE:
        {
            --context->m_CallDepth;

            uint32_t argCount = GET_U8;
            uint32_t retCount = GET_U8;

            scrValue* ret = sp - retCount;
            sp = fp + argCount - 1;
            fp = stack + sp[2].Uns;

            uint32_t caller = sp[1].Uns;

            void* secureFrame = context->m_SecureFrames[context->m_CallDepth];
            pointers.ResetSecureScriptFrame(&secureFrame);

            JUMP(caller);
            sp -= argCount;

            while (retCount--)
                *++sp = *++ret;

            if (!caller)
            {
                pointers.ResetSecureScriptFrame(&context->m_SecureUnk);
                return context->m_State = State::KILLED;
            }
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
                return OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_U8;
            sp[0].Reference = ref;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U8_LOAD:
        {
            --sp;
            scrValue* ref = sp[1].Reference;
            uint32_t index = sp[0].Uns;
            if (index >= ref->Uns)
                return OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);
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
                return OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_U8;
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
            sp[0].Reference = stack + GET_U8;
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
            stack[GET_U8].Any = sp[1].Any;
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
            sp[0].Any += GET_U8 * sizeof(scrValue);
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
            sp[2].Reference[GET_U8].Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_S16:
        {
            ++sp;
            sp[0].Int = GET_S16;
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
            sp[0].Any += GET_S16 * sizeof(scrValue);
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
            sp[2].Reference[GET_S16].Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U16:
        {
            --sp;
            scrValue* ref = sp[1].Reference;
            uint32_t index = sp[0].Uns;
            if (index >= ref->Uns)
                return OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_U16;
            sp[0].Reference = ref;
            goto NEXT;
        }
        case scrOpcode::ARRAY_U16_LOAD:
        {
            --sp;
            scrValue* ref = sp[1].Reference;
            uint32_t index = sp[0].Uns;
            if (index >= ref->Uns)
                return OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);
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
                return OnException(pc, op, "Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_U16;
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
            sp[0].Reference = stack + GET_U16;
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
            stack[GET_U16].Any = sp[1].Any;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_U16:
        {
            ++sp;
            sp[0].Reference = globals[0] + GET_U16;
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
            globals[0][GET_U16].Any = sp[1].Any;
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
        case scrOpcode::GLOBAL_U24:
        {
            uint32_t global = GET_U24;
            uint32_t block = global >> 0x12U;
            uint32_t index = global & 0x3FFFFU;
            ++sp;
            if (!globalBlocksEnabled[block])
                return OnException(pc, op, "Global block %u (index %u) is not enabled!", block, index);
            if (!globals[block])
                return OnException(pc, op, "Global block %u (index %u) is not loaded!", block, index);
            else
            {
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
            if (!globalBlocksEnabled[block])
                return OnException(pc, op, "Global block %u (index %u) is not enabled!", block, index);
            if (!globals[block])
                return OnException(pc, op, "Global block %u (index %u) is not loaded!", block, index);
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
            if (!globalBlocksEnabled[block])
                return OnException(pc, op, "Global block %u (index %u) is not enabled!", block, index);
            if (!globals[block])
                return OnException(pc, op, "Global block %u (index %u) is not loaded!", block, index);
            else
            {
                globals[block][index].Any = sp[1].Any;
            }
            goto NEXT;
        }
        case scrOpcode::PUSH_CONST_U24:
        {
            ++sp;
            sp[0].Int = GET_U24;
            goto NEXT;
        }
        case scrOpcode::SWITCH:
        {
            --sp;
            int32_t switchVal = sp[1].Int;
            uint16_t caseCount = GET_U16;
            uint8_t* caseTable = code + 1;

            int32_t matchOfs = -1;
            uint8_t* matchEntryEnd = nullptr;
            int32_t lo = 0;
            int32_t hi = static_cast<int32_t>(caseCount) - 1;

            while (lo <= hi)
            {
                int32_t mid = lo + (hi - lo) / 2;
                uint8_t* entry = caseTable + mid * 6;
                int32_t caseVal = *reinterpret_cast<int32_t*>(entry);
                if (switchVal == caseVal)
                {
                    matchOfs = *reinterpret_cast<int16_t*>(entry + 4);
                    matchEntryEnd = entry + 6;
                    break;
                }
                if (switchVal < caseVal)
                    hi = mid - 1;
                else
                    lo = mid + 1;
            }

            code += caseCount * 6;
            if (matchOfs != -1)
                JUMP((matchEntryEnd - base) - 1 + matchOfs);
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
                return OnException(pc, op, "Catch PC is NULL!");
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
                return OnException(pc, op, "Function pointer is NULL!");
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
        case scrOpcode::LOCAL_LOAD_S: // TO-DO: test whether these actually work
        {
            scrValue* ref = sp[0].Reference;
            scrValue handle = *ref;
            if (handle.Any != 0)
            {
                sp[0].Uns = pointers.DecryptSecureScriptValue(handle.Reference);
            }
            else
            {
                uint32_t zero = 0;
                pointers.ResolveSecureScriptFrame(ref, &handle, context);
                pointers.SetSecureScriptValue(handle.Reference, &zero);
                sp[0].Uns = 0;
            }
            goto NEXT;
        }
        case scrOpcode::LOCAL_STORE_S:
        {
            scrValue* ref = sp[0].Reference;
            scrValue handle = *ref;
            if (handle.Any == 0)
                pointers.ResolveSecureScriptFrame(ref, &handle, context);
            pointers.SetSecureScriptValue(handle.Reference, &sp[-1].Uns);
            sp -= 2;
            goto NEXT;
        }
        case scrOpcode::LOCAL_STORE_SR:
        {
            scrValue* ref = sp[0].Reference;
            scrValue handle = *ref;
            if (handle.Any == 0)
                pointers.ResolveSecureScriptFrame(ref, &handle, context);
            uint32_t scrambled = ScrambleSecureValue(sp[-1].Uns);
            sp[-1].Uns = scrambled;
            pointers.SetSecureScriptValue(handle.Reference, &scrambled);
            sp -= 2;
            goto NEXT;
        }
        case scrOpcode::STATIC_LOAD_S:
        {
            scrValue* ref = sp[0].Reference;
            scrValue handle = *ref;
            if (handle.Any != 0)
            {
                sp[0].Uns = pointers.DecryptSecureScriptValue(handle.Reference);
            }
            else
            {
                uint32_t zero = 0;
                pointers.ResolveSecureScriptStatic(ref, &handle, context);
                pointers.SetSecureScriptValue(handle.Reference, &zero);
                sp[0].Uns = 0;
            }
            goto NEXT;
        }
        case scrOpcode::STATIC_STORE_S:
        {
            scrValue* ref = sp[0].Reference;
            scrValue handle = *ref;
            if (handle.Any == 0)
                pointers.ResolveSecureScriptStatic(ref, &handle, context);
            pointers.SetSecureScriptValue(handle.Reference, &sp[-1].Uns);
            sp -= 2;
            goto NEXT;
        }
        case scrOpcode::STATIC_STORE_SR:
        {
            scrValue* ref = sp[0].Reference;
            scrValue handle = *ref;
            if (handle.Any == 0)
                pointers.ResolveSecureScriptStatic(ref, &handle, context);
            uint32_t scrambled = ScrambleSecureValue(sp[-1].Uns);
            sp[-1].Uns = scrambled;
            pointers.SetSecureScriptValue(handle.Reference, &scrambled);
            sp -= 2;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_LOAD_S:
        {
            scrValue* ref = sp[0].Reference;
            scrValue handle = *ref;
            if (handle.Any != 0)
            {
                sp[0].Uns = pointers.DecryptSecureScriptValue(handle.Reference);
            }
            else
            {
                uint32_t zero = 0;
                pointers.ResolveSecureScriptGlobal(ref, &handle);
                pointers.SetSecureScriptValue(handle.Reference, &zero);
                sp[0].Uns = 0;
            }
            goto NEXT;
        }
        case scrOpcode::GLOBAL_STORE_S:
        {
            scrValue* ref = sp[0].Reference;
            scrValue handle = *ref;
            if (handle.Any == 0)
                pointers.ResolveSecureScriptGlobal(ref, &handle);
            pointers.SetSecureScriptValue(handle.Reference, &sp[-1].Uns);
            sp -= 2;
            goto NEXT;
        }
        case scrOpcode::GLOBAL_STORE_SR:
        {
            scrValue* ref = sp[0].Reference;
            scrValue handle = *ref;
            if (handle.Any == 0)
                pointers.ResolveSecureScriptGlobal(ref, &handle);
            uint32_t scrambled = ScrambleSecureValue(sp[-1].Uns);
            sp[-1].Uns = scrambled;
            pointers.SetSecureScriptValue(handle.Reference, &scrambled);
            sp -= 2;
            goto NEXT;
        }
        case scrOpcode::LOAD_N_S:
        {
            scrValue* data = (sp--)->Reference;
            uint32_t itemCount = (sp--)->Int;
            for (uint32_t i = 0; i < itemCount; i++)
            {
                scrValue* ref = &data[itemCount - 1 - i];
                scrValue handle = *ref;
                if (handle.Any != 0)
                {
                    (++sp)[0].Uns = pointers.DecryptSecureScriptValue(handle.Reference);
                }
                else
                {
                    uint32_t zero = 0;
                    ResolveSecureScriptVariable(ref, &handle, context, stack, program);
                    pointers.SetSecureScriptValue(handle.Reference, &zero);
                    (++sp)[0].Uns = 0;
                }
            }
            goto NEXT;
        }
        case scrOpcode::STORE_N_S:
        {
            scrValue* data = (sp--)->Reference;
            uint32_t itemCount = (sp--)->Int;
            for (uint32_t i = 0; i < itemCount; i++)
            {
                scrValue* ref = &data[itemCount - 1 - i];
                scrValue handle = *ref;
                if (handle.Any == 0)
                    ResolveSecureScriptVariable(ref, &handle, context, stack, program);
                pointers.SetSecureScriptValue(handle.Reference, &(sp--)->Uns);
            }
            goto NEXT;
        }
        case scrOpcode::STORE_N_SR:
        {
            scrValue* data = (sp--)->Reference;
            uint32_t itemCount = (sp--)->Int;
            for (uint32_t i = 0; i < itemCount; i++)
            {
                scrValue* ref = &data[itemCount - 1 - i];
                scrValue handle = *ref;
                if (handle.Any == 0)
                    ResolveSecureScriptVariable(ref, &handle, context, stack, program);
                uint32_t scrambled = ScrambleSecureValue((sp)->Uns);
                (sp)->Uns = scrambled;
                pointers.SetSecureScriptValue(handle.Reference, &scrambled);
                --sp;
            }
            goto NEXT;
        }
        case scrOpcode::STATIC_U24:
        {
            ++sp;
            sp[0].Reference = stack + GET_U24;
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
            stack[GET_U24].Any = sp[1].Any;
            goto NEXT;
        }
        }

        return OnException(pc, op, "Invalid opcode: 0x%02X", static_cast<uint8_t>(op));
    }
}

#endif