#include "scrThread.hpp"
#include "core/Memory.hpp"
#include "debug/ScriptBreakpoint.hpp"
#include "rage/shared/Joaat.hpp"
#include "rage/shared/scrNativeCallContext.hpp"
#include "rage/shared/scrOpcode.hpp"
#include "rage/shared/scrProgram.hpp"
#include "rage/shared/scrString.hpp"
#include "rage/shared/scrValue.hpp"
#include "tlsContext.hpp"

namespace rage::enhanced
{
    scrThread* scrThread::GetCurrentThread()
    {
        return tlsContext::Get()->m_CurrentScriptThread;
    }

    scrThread* scrThread::GetThread(std::uint32_t hash)
    {
        static bool init = [] {
            if (auto addr = Memory::ScanPattern("48 8B 05 ? ? ? ? 48 89 34 F8 48 FF C7 48 39 FB 75 97"))
                m_Threads = addr->Add(3).Rip().As<shared::atArray<scrThread*>*>();

            return true;
        }();

        if (!m_Threads)
            return nullptr;

        for (auto& thread : *m_Threads)
        {
            if (thread && thread->m_Context.m_ThreadId && thread->m_ScriptHash == hash)
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
        std::snprintf(fullMessage, sizeof(fullMessage),
            "Exception in %s at 0x%X!\nReason: %s",
            thread->m_ScriptName,
            thread->m_Context.m_ProgramCounter,
            message);

        MessageBoxA(0, fullMessage, "Exception", MB_OK | MB_ICONERROR);

        return thread->m_Context.m_State = scrThreadState::KILLED;
    }

    bool scrThread::ProcessBreakpoints()
    {
        auto thread = GetCurrentThread();
        if (!thread)
            return false;

        if (ScriptBreakpoint::m_ActiveBreakpoint.has_value())
        {
            auto thread = GetThread(ScriptBreakpoint::m_ActiveBreakpoint->first);
            if (!thread)
            {
                // Script died when the breakpoint was active
                ScriptBreakpoint::m_SkipThisBreakpointHit = false;
                ScriptBreakpoint::m_ActiveBreakpoint.reset();
            }
        }

        for (auto& bp : ScriptBreakpoint::m_Breakpoints)
        {
            if (thread->m_ScriptHash != bp.first || thread->m_Context.m_ProgramCounter != bp.second)
                continue;

            if (ScriptBreakpoint::m_SkipThisBreakpointHit)
            {
                ScriptBreakpoint::m_SkipThisBreakpointHit = false;
                return false;
            }

            // Show the message first so scrDbg doesn't attempt to resume when MessageBoxA is active
            char message[128];
            std::snprintf(message, sizeof(message), "Breakpoint hit, paused %s at 0x%X!", thread->m_ScriptName, thread->m_Context.m_ProgramCounter);
            MessageBoxA(0, message, "Breakpoint", MB_OK);

            ScriptBreakpoint::m_ActiveBreakpoint = bp;

            if (ScriptBreakpoint::m_BreakpointsPauseGame)
            {
                ScriptBreakpoint::PauseGame(true);
                ScriptBreakpoint::m_WasGamePausedByBreakpoint = true;
            }
            else
            {
                thread->m_Context.m_State = scrThreadState::PAUSED;
            }

            return true; // Return here to signal the VM that a BP is active, no need to check for others as there can only be one active at a time.
        }

        return false;
    }

    bool scrThread::ResumeActiveBreakpoint()
    {
        if (!ScriptBreakpoint::m_ActiveBreakpoint.has_value())
            return false;

        auto thread = GetThread(ScriptBreakpoint::m_ActiveBreakpoint->first);
        if (!thread)
            return false;

        if (ScriptBreakpoint::m_WasGamePausedByBreakpoint)
        {
            ScriptBreakpoint::PauseGame(false);
            ScriptBreakpoint::m_WasGamePausedByBreakpoint = false;
        }
        else
        {
            thread->m_Context.m_State = scrThreadState::RUNNING;
        }

        ScriptBreakpoint::m_SkipThisBreakpointHit = true;
        ScriptBreakpoint::m_ActiveBreakpoint.reset();
        return true;
    }

#define GET_BYTE (*++opcode)
#define GET_WORD ((opcode += 2), *reinterpret_cast<std::uint16_t*>(opcode - 1))
#define GET_SWORD ((opcode += 2), *reinterpret_cast<std::int16_t*>(opcode - 1))
#define GET_24BIT ((opcode += 3), *reinterpret_cast<std::uint32_t*>(opcode - 3) >> 8)
#define GET_DWORD ((opcode += 4), *reinterpret_cast<std::uint32_t*>(opcode - 3))

#define JUMP(_offset)                                              \
    do                                                             \
    {                                                              \
        std::int64_t offset = _offset;                             \
        opcode = bytecode[offset >> 14U] + (offset & 0x3FFFU) - 1; \
        basePtr = &opcode[-offset];                                \
    } while (0)

    scrThreadState scrThread::RunThread(shared::scrValue* stack, shared::scrValue** globals, shared::scrProgram* program, scrThreadContext* context)
    {
        char buffer[16];
        std::uint8_t* opcode;
        std::uint8_t* basePtr;

        std::uint8_t** bytecode = program->m_CodeBlocks;
        shared::scrValue* stackPtr = stack + context->m_StackPointer - 1;
        shared::scrValue* framePtr = stack + context->m_FramePointer;

        JUMP(context->m_ProgramCounter);

    NEXT:
        // Update these per opcode
        context->m_ProgramCounter = static_cast<std::uint32_t>(opcode - basePtr);
        context->m_StackPointer = static_cast<std::int32_t>(stackPtr - stack + 1);
        context->m_FramePointer = static_cast<std::int32_t>(framePtr - stack);

        if (ProcessBreakpoints())
            return context->m_State; // If we do not return here, the VM will end up executing opcodes until the next NATIVE call.

        switch (GET_BYTE)
        {
        case shared::scrOpcode::NOP:
        {
            JUMP(opcode - basePtr);
            goto NEXT;
        }
        case shared::scrOpcode::IADD:
        {
            --stackPtr;
            stackPtr[0].Int += stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::ISUB:
        {
            --stackPtr;
            stackPtr[0].Int -= stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::IMUL:
        {
            --stackPtr;
            stackPtr[0].Int *= stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::IDIV:
        {
            --stackPtr;
            if (stackPtr[1].Int)
                stackPtr[0].Int /= stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::IMOD:
        {
            --stackPtr;
            if (stackPtr[1].Int)
                stackPtr[0].Int %= stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::INOT:
        {
            stackPtr[0].Int = !stackPtr[0].Int;
            goto NEXT;
        }
        case shared::scrOpcode::INEG:
        {
            stackPtr[0].Int = -stackPtr[0].Int;
            goto NEXT;
        }
        case shared::scrOpcode::IEQ:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int == stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::INE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int != stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::IGT:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int > stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::IGE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int >= stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::ILT:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int < stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::ILE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Int <= stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::FADD:
        {
            --stackPtr;
            stackPtr[0].Float += stackPtr[1].Float;
            goto NEXT;
        }
        case shared::scrOpcode::FSUB:
        {
            --stackPtr;
            stackPtr[0].Float -= stackPtr[1].Float;
            goto NEXT;
        }
        case shared::scrOpcode::FMUL:
        {
            --stackPtr;
            stackPtr[0].Float *= stackPtr[1].Float;
            goto NEXT;
        }
        case shared::scrOpcode::FDIV:
        {
            --stackPtr;
            if (stackPtr[1].Int)
                stackPtr[0].Float /= stackPtr[1].Float;
            goto NEXT;
        }
        case shared::scrOpcode::FMOD:
        {
            --stackPtr;
            if (stackPtr[1].Int)
            {
                float x = stackPtr[0].Float;
                float y = stackPtr[1].Float;
                stackPtr[0].Float = y ? x - (static_cast<std::int32_t>(x / y) * y) : 0.0f;
            }
            goto NEXT;
        }
        case shared::scrOpcode::FNEG:
        {
            stackPtr[0].Uns ^= 0x80000000;
            goto NEXT;
        }
        case shared::scrOpcode::FEQ:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float == stackPtr[1].Float;
            goto NEXT;
        }
        case shared::scrOpcode::FNE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float != stackPtr[1].Float;
            goto NEXT;
        }
        case shared::scrOpcode::FGT:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float > stackPtr[1].Float;
            goto NEXT;
        }
        case shared::scrOpcode::FGE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float >= stackPtr[1].Float;
            goto NEXT;
        }
        case shared::scrOpcode::FLT:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float < stackPtr[1].Float;
            goto NEXT;
        }
        case shared::scrOpcode::FLE:
        {
            --stackPtr;
            stackPtr[0].Int = stackPtr[0].Float <= stackPtr[1].Float;
            goto NEXT;
        }
        case shared::scrOpcode::VADD:
        {
            stackPtr -= 3;
            stackPtr[-2].Float += stackPtr[1].Float;
            stackPtr[-1].Float += stackPtr[2].Float;
            stackPtr[0].Float += stackPtr[3].Float;
            goto NEXT;
        }
        case shared::scrOpcode::VSUB:
        {
            stackPtr -= 3;
            stackPtr[-2].Float -= stackPtr[1].Float;
            stackPtr[-1].Float -= stackPtr[2].Float;
            stackPtr[0].Float -= stackPtr[3].Float;
            goto NEXT;
        }
        case shared::scrOpcode::VMUL:
        {
            stackPtr -= 3;
            stackPtr[-2].Float *= stackPtr[1].Float;
            stackPtr[-1].Float *= stackPtr[2].Float;
            stackPtr[0].Float *= stackPtr[3].Float;
            goto NEXT;
        }
        case shared::scrOpcode::VDIV:
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
        case shared::scrOpcode::VNEG:
        {
            stackPtr[-2].Uns ^= 0x80000000;
            stackPtr[-1].Uns ^= 0x80000000;
            stackPtr[0].Uns ^= 0x80000000;
            goto NEXT;
        }
        case shared::scrOpcode::IAND:
        {
            --stackPtr;
            stackPtr[0].Int &= stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::IOR:
        {
            --stackPtr;
            stackPtr[0].Int |= stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::IXOR:
        {
            --stackPtr;
            stackPtr[0].Int ^= stackPtr[1].Int;
            goto NEXT;
        }
        case shared::scrOpcode::I2F:
        {
            stackPtr[0].Float = static_cast<float>(stackPtr[0].Int);
            goto NEXT;
        }
        case shared::scrOpcode::F2I:
        {
            stackPtr[0].Int = static_cast<std::int32_t>(stackPtr[0].Float);
            goto NEXT;
        }
        case shared::scrOpcode::F2V:
        {
            stackPtr += 2;
            stackPtr[-1].Int = stackPtr[0].Int = stackPtr[-2].Int;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_U8:
        {
            ++stackPtr;
            stackPtr[0].Int = GET_BYTE;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_U8_U8:
        {
            stackPtr += 2;
            stackPtr[-1].Int = GET_BYTE;
            stackPtr[0].Int = GET_BYTE;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_U8_U8_U8:
        {
            stackPtr += 3;
            stackPtr[-2].Int = GET_BYTE;
            stackPtr[-1].Int = GET_BYTE;
            stackPtr[0].Int = GET_BYTE;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_U32:
        case shared::scrOpcode::PUSH_CONST_F:
        {
            ++stackPtr;
            stackPtr[0].Uns = GET_DWORD;
            goto NEXT;
        }
        case shared::scrOpcode::DUP:
        {
            ++stackPtr;
            stackPtr[0].Any = stackPtr[-1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::DROP:
        {
            --stackPtr;
            goto NEXT;
        }
        case shared::scrOpcode::NATIVE:
        {
            std::uint8_t native = GET_BYTE;
            std::int32_t argCount = (native >> 2) & 0x3F;
            std::int32_t retCount = native & 3;

            std::uint8_t high = GET_BYTE;
            std::uint8_t low = GET_BYTE;
            shared::scrNativeHandler handler = reinterpret_cast<shared::scrNativeHandler>(program->m_Natives[(high << 8) | low]);

            context->m_ProgramCounter = static_cast<std::int32_t>(opcode - basePtr - 4);
            context->m_StackPointer = static_cast<std::int32_t>(stackPtr - stack + 1);
            context->m_FramePointer = static_cast<std::int32_t>(framePtr - stack);

            shared::scrNativeCallContext ctx(retCount ? &stack[context->m_StackPointer - argCount] : 0, argCount, &stack[context->m_StackPointer - argCount]);
            (*handler)(&ctx);

            // In case WAIT, TERMINATE_THIS_THREAD, or TERMINATE_THREAD is called
            if (context->m_State != scrThreadState::RUNNING)
                return context->m_State;

            ctx.FixVectors();

            stackPtr -= argCount - retCount;
            goto NEXT;
        }
        case shared::scrOpcode::ENTER:
        {
            std::uint32_t argCount = GET_BYTE;
            std::uint32_t frameSize = GET_WORD;
            std::uint32_t nameCount = GET_BYTE;

            if (context->m_CallDepth < 16)
                context->m_CallStack[context->m_CallDepth] = static_cast<std::int32_t>(opcode - basePtr + 2);
            ++context->m_CallDepth;

            opcode += nameCount;

            std::uint32_t current = static_cast<std::uint32_t>(stackPtr - stack);
            std::uint32_t max = context->m_StackSize - frameSize;
            if (current >= max) [[unlikely]]
                return OnScriptException("Stack overflow: %u >= %u", current, max);

            (++stackPtr)->Int = static_cast<std::int32_t>(framePtr - stack);

            framePtr = stackPtr - argCount - 1;

            while (frameSize--)
                (++stackPtr)->Any = 0;

            stackPtr -= argCount;
            goto NEXT;
        }
        case shared::scrOpcode::LEAVE:
        {
            --context->m_CallDepth;

            std::uint32_t argCount = GET_BYTE;
            std::uint32_t retCount = GET_BYTE;

            shared::scrValue* ret = stackPtr - retCount;
            stackPtr = framePtr + argCount - 1;
            framePtr = stack + stackPtr[2].Uns;

            std::uint32_t caller = stackPtr[1].Uns;
            JUMP(caller);
            stackPtr -= argCount;

            while (retCount--)
                *++stackPtr = *++ret;

            // Script reached end of code
            if (!caller)
                return context->m_State = scrThreadState::KILLED;

            goto NEXT;
        }
        case shared::scrOpcode::LOAD:
        {
            stackPtr[0].Any = stackPtr[0].Reference->Any;
            goto NEXT;
        }
        case shared::scrOpcode::STORE:
        {
            stackPtr -= 2;
            stackPtr[2].Reference->Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::STORE_REV:
        {
            --stackPtr;
            stackPtr[0].Reference->Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::LOAD_N:
        {
            shared::scrValue* data = (stackPtr--)->Reference;
            std::uint32_t itemCount = (stackPtr--)->Int;
            for (std::uint32_t i = 0; i < itemCount; i++)
                (++stackPtr)->Any = data[i].Any;
            goto NEXT;
        }
        case shared::scrOpcode::STORE_N:
        {
            shared::scrValue* data = (stackPtr--)->Reference;
            std::uint32_t itemCount = (stackPtr--)->Int;
            for (std::uint32_t i = 0; i < itemCount; i++)
                data[itemCount - 1 - i].Any = (stackPtr--)->Any;
            goto NEXT;
        }
        case shared::scrOpcode::ARRAY_U8:
        {
            --stackPtr;
            shared::scrValue* ref = stackPtr[1].Reference;
            std::uint32_t index = stackPtr[0].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_BYTE;
            stackPtr[0].Reference = ref;
            goto NEXT;
        }
        case shared::scrOpcode::ARRAY_U8_LOAD:
        {
            --stackPtr;
            shared::scrValue* ref = stackPtr[1].Reference;
            std::uint32_t index = stackPtr[0].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_BYTE;
            stackPtr[0].Any = ref->Any;
            goto NEXT;
        }
        case shared::scrOpcode::ARRAY_U8_STORE:
        {
            stackPtr -= 3;
            shared::scrValue* ref = stackPtr[3].Reference;
            std::uint32_t index = stackPtr[2].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_BYTE;
            ref->Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::LOCAL_U8:
        {
            ++stackPtr;
            stackPtr[0].Reference = framePtr + GET_BYTE;
            goto NEXT;
        }
        case shared::scrOpcode::LOCAL_U8_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = framePtr[GET_BYTE].Any;
            goto NEXT;
        }
        case shared::scrOpcode::LOCAL_U8_STORE:
        {
            --stackPtr;
            framePtr[GET_BYTE].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::STATIC_U8:
        {
            ++stackPtr;
            stackPtr[0].Reference = stack + GET_BYTE;
            goto NEXT;
        }
        case shared::scrOpcode::STATIC_U8_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = stack[GET_BYTE].Any;
            goto NEXT;
        }
        case shared::scrOpcode::STATIC_U8_STORE:
        {
            --stackPtr;
            stack[GET_BYTE].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::IADD_U8:
        {
            stackPtr[0].Int += GET_BYTE;
            goto NEXT;
        }
        case shared::scrOpcode::IMUL_U8:
        {
            stackPtr[0].Int *= GET_BYTE;
            goto NEXT;
        }
        case shared::scrOpcode::IOFFSET:
        {
            --stackPtr;
            stackPtr[0].Any += stackPtr[1].Int * sizeof(shared::scrValue);
            goto NEXT;
        }
        case shared::scrOpcode::IOFFSET_U8:
        {
            stackPtr[0].Any += GET_BYTE * sizeof(shared::scrValue);
            goto NEXT;
        }
        case shared::scrOpcode::IOFFSET_U8_LOAD:
        {
            stackPtr[0].Any = stackPtr[0].Reference[GET_BYTE].Any;
            goto NEXT;
        }
        case shared::scrOpcode::IOFFSET_U8_STORE:
        {
            stackPtr -= 2;
            stackPtr[2].Reference[GET_BYTE].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_S16:
        {
            ++stackPtr;
            stackPtr[0].Int = GET_SWORD;
            goto NEXT;
        }
        case shared::scrOpcode::IADD_S16:
        {
            stackPtr[0].Int += GET_SWORD;
            goto NEXT;
        }
        case shared::scrOpcode::IMUL_S16:
        {
            stackPtr[0].Int *= GET_SWORD;
            goto NEXT;
        }
        case shared::scrOpcode::IOFFSET_S16:
        {
            stackPtr[0].Any += GET_SWORD * sizeof(shared::scrValue);
            goto NEXT;
        }
        case shared::scrOpcode::IOFFSET_S16_LOAD:
        {
            stackPtr[0].Any = stackPtr[0].Reference[GET_SWORD].Any;
            goto NEXT;
        }
        case shared::scrOpcode::IOFFSET_S16_STORE:
        {
            stackPtr -= 2;
            stackPtr[2].Reference[GET_SWORD].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::ARRAY_U16:
        {
            --stackPtr;
            shared::scrValue* ref = stackPtr[1].Reference;
            std::uint32_t index = stackPtr[0].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_WORD;
            stackPtr[0].Reference = ref;
            goto NEXT;
        }
        case shared::scrOpcode::ARRAY_U16_LOAD:
        {
            --stackPtr;
            shared::scrValue* ref = stackPtr[1].Reference;
            std::uint32_t index = stackPtr[0].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_WORD;
            stackPtr[0].Any = ref->Any;
            goto NEXT;
        }
        case shared::scrOpcode::ARRAY_U16_STORE:
        {
            stackPtr -= 3;
            shared::scrValue* ref = stackPtr[3].Reference;
            std::uint32_t index = stackPtr[2].Uns;
            if (index >= ref->Uns) [[unlikely]]
                return OnScriptException("Array access out of bounds: %u >= %u", index, ref->Uns);
            ref += 1U + index * GET_WORD;
            ref->Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::LOCAL_U16:
        {
            ++stackPtr;
            stackPtr[0].Reference = framePtr + GET_WORD;
            goto NEXT;
        }
        case shared::scrOpcode::LOCAL_U16_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = framePtr[GET_WORD].Any;
            goto NEXT;
        }
        case shared::scrOpcode::LOCAL_U16_STORE:
        {
            --stackPtr;
            framePtr[GET_WORD].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::STATIC_U16:
        {
            ++stackPtr;
            stackPtr[0].Reference = stack + GET_WORD;
            goto NEXT;
        }
        case shared::scrOpcode::STATIC_U16_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = stack[GET_WORD].Any;
            goto NEXT;
        }
        case shared::scrOpcode::STATIC_U16_STORE:
        {
            --stackPtr;
            stack[GET_WORD].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::GLOBAL_U16:
        {
            ++stackPtr;
            stackPtr[0].Reference = globals[0] + GET_WORD;
            goto NEXT;
        }
        case shared::scrOpcode::GLOBAL_U16_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = globals[0][GET_WORD].Any;
            goto NEXT;
        }
        case shared::scrOpcode::GLOBAL_U16_STORE:
        {
            --stackPtr;
            globals[0][GET_WORD].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::J:
        {
            std::int32_t ofs = GET_SWORD;
            JUMP(opcode - basePtr + ofs);
            goto NEXT;
        }
        case shared::scrOpcode::JZ:
        {
            std::int32_t ofs = GET_SWORD;
            --stackPtr;
            if (stackPtr[1].Int == 0)
                JUMP(opcode - basePtr + ofs);
            else
                JUMP(opcode - basePtr);
            goto NEXT;
        }
        case shared::scrOpcode::IEQ_JZ:
        {
            std::int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int == stackPtr[2].Int))
                JUMP(opcode - basePtr + ofs);
            else
                JUMP(opcode - basePtr);
            goto NEXT;
        }
        case shared::scrOpcode::INE_JZ:
        {
            std::int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int != stackPtr[2].Int))
                JUMP(opcode - basePtr + ofs);
            else
                JUMP(opcode - basePtr);
            goto NEXT;
        }
        case shared::scrOpcode::IGT_JZ:
        {
            std::int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int > stackPtr[2].Int))
                JUMP(opcode - basePtr + ofs);
            else
                JUMP(opcode - basePtr);
            goto NEXT;
        }
        case shared::scrOpcode::IGE_JZ:
        {
            std::int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int >= stackPtr[2].Int))
                JUMP(opcode - basePtr + ofs);
            else
                JUMP(opcode - basePtr);
            goto NEXT;
        }
        case shared::scrOpcode::ILT_JZ:
        {
            std::int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int < stackPtr[2].Int))
                JUMP(opcode - basePtr + ofs);
            else
                JUMP(opcode - basePtr);
            goto NEXT;
        }
        case shared::scrOpcode::ILE_JZ:
        {
            std::int32_t ofs = GET_SWORD;
            stackPtr -= 2;
            if (!(stackPtr[1].Int <= stackPtr[2].Int))
                JUMP(opcode - basePtr + ofs);
            else
                JUMP(opcode - basePtr);
            goto NEXT;
        }
        case shared::scrOpcode::CALL:
        {
            std::uint32_t ofs = GET_24BIT;
            ++stackPtr;
            stackPtr[0].Uns = static_cast<std::int32_t>(opcode - basePtr);
            JUMP(ofs);
            goto NEXT;
        }
        case shared::scrOpcode::STATIC_U24:
        {
            ++stackPtr;
            stackPtr[0].Reference = stack + GET_24BIT;
            goto NEXT;
        }
        case shared::scrOpcode::STATIC_U24_LOAD:
        {
            ++stackPtr;
            stackPtr[0].Any = stack[GET_24BIT].Any;
            goto NEXT;
        }
        case shared::scrOpcode::STATIC_U24_STORE:
        {
            --stackPtr;
            stack[GET_24BIT].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::GLOBAL_U24:
        {
            std::uint32_t global = GET_24BIT;
            std::uint32_t block = global >> 0x12U;
            std::uint32_t index = global & 0x3FFFFU;
            ++stackPtr;
            if (!globals[block]) [[unlikely]]
                return OnScriptException("Global block %u (index %u) is not loaded!", block, index);
            else
                stackPtr[0].Reference = &globals[block][index];
            goto NEXT;
        }
        case shared::scrOpcode::GLOBAL_U24_LOAD:
        {
            std::uint32_t global = GET_24BIT;
            std::uint32_t block = global >> 0x12U;
            std::uint32_t index = global & 0x3FFFFU;
            ++stackPtr;
            if (!globals[block]) [[unlikely]]
                return OnScriptException("Global block %u (index %u) is not loaded!", block, index);
            else
                stackPtr[0].Any = globals[block][index].Any;
            goto NEXT;
        }
        case shared::scrOpcode::GLOBAL_U24_STORE:
        {
            std::uint32_t global = GET_24BIT;
            std::uint32_t block = global >> 0x12U;
            std::uint32_t index = global & 0x3FFFFU;
            --stackPtr;
            if (!globals[block]) [[unlikely]]
                return OnScriptException("Global block %u (index %u) is not loaded!", block, index);
            else
                globals[block][index].Any = stackPtr[1].Any;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_U24:
        {
            ++stackPtr;
            stackPtr[0].Int = GET_24BIT;
            goto NEXT;
        }
        case shared::scrOpcode::SWITCH:
        {
            --stackPtr;
            std::uint32_t switchVal = stackPtr[1].Uns;
            std::uint32_t caseCount = GET_BYTE;
            JUMP(opcode - basePtr);
            for (std::uint32_t i = 0; i < caseCount; i++)
            {
                std::uint32_t caseVal = GET_DWORD;
                std::uint32_t ofs = GET_WORD;
                if (switchVal == caseVal)
                {
                    JUMP(opcode - basePtr + ofs);
                    break;
                }
            }
            JUMP(opcode - basePtr);
            goto NEXT;
        }
        case shared::scrOpcode::STRING:
        {
            std::uint32_t ofs = stackPtr[0].Uns;
            stackPtr[0].String = program->m_Strings[ofs >> 14U] + (ofs & 0x3FFFU);
            goto NEXT;
        }
        case shared::scrOpcode::STRINGHASH:
        {
            stackPtr[0].Uns = shared::Joaat(stackPtr[0].String);
            goto NEXT;
        }
        case shared::scrOpcode::TEXT_LABEL_ASSIGN_STRING:
        {
            stackPtr -= 2;
            char* dest = const_cast<char*>(stackPtr[2].String);
            const char* src = stackPtr[1].String;
            shared::scrStringAssign(dest, GET_BYTE, src);
            goto NEXT;
        }
        case shared::scrOpcode::TEXT_LABEL_ASSIGN_INT:
        {
            stackPtr -= 2;
            char* dest = const_cast<char*>(stackPtr[2].String);
            std::int32_t value = stackPtr[1].Int;
            shared::scrStringItoa(buffer, value);
            shared::scrStringAssign(dest, GET_BYTE, buffer);
            goto NEXT;
        }
        case shared::scrOpcode::TEXT_LABEL_APPEND_STRING:
        {
            stackPtr -= 2;
            char* dest = const_cast<char*>(stackPtr[2].String);
            const char* src = stackPtr[1].String;
            shared::scrStringAppend(dest, GET_BYTE, src);
            goto NEXT;
        }
        case shared::scrOpcode::TEXT_LABEL_APPEND_INT:
        {
            stackPtr -= 2;
            char* dest = const_cast<char*>(stackPtr[2].String);
            std::int32_t value = stackPtr[1].Int;
            shared::scrStringItoa(buffer, value);
            shared::scrStringAppend(dest, GET_BYTE, buffer);
            goto NEXT;
        }
        case shared::scrOpcode::TEXT_LABEL_COPY:
        {
            stackPtr -= 3;
            shared::scrValue* dest = stackPtr[3].Reference;
            std::int32_t destSize = stackPtr[2].Int;
            std::int32_t srcSize = stackPtr[1].Int;
            if (srcSize > destSize)
            {
                std::int32_t excess = srcSize - destSize;
                stackPtr -= excess;
                srcSize = destSize;
            }
            shared::scrValue* destPtr = dest + srcSize - 1;
            for (std::int32_t i = 0; i < srcSize; i++)
                *destPtr-- = *stackPtr--;
            reinterpret_cast<char*>(dest)[srcSize * sizeof(shared::scrValue) - 1] = '\0';
            goto NEXT;
        }
        case shared::scrOpcode::CATCH:
        {
            context->m_CatchProgramCounter = static_cast<std::int32_t>(opcode - basePtr);
            context->m_CatchFramePointer = static_cast<std::int32_t>(framePtr - stack);
            context->m_CatchStackPointer = static_cast<std::int32_t>(stackPtr - stack + 1);
            ++stackPtr;
            stackPtr[0].Int = -1;
            goto NEXT;
        }
        case shared::scrOpcode::THROW:
        {
            std::int32_t ofs = stackPtr[0].Int;
            if (!context->m_CatchProgramCounter) [[unlikely]]
                return OnScriptException("Catch PC is NULL!");
            else
            {
                JUMP(context->m_CatchProgramCounter);
                framePtr = stack + context->m_CatchFramePointer;
                stackPtr = stack + context->m_CatchStackPointer;
                stackPtr[0].Int = ofs;
            }
            goto NEXT;
        }
        case shared::scrOpcode::CALLINDIRECT:
        {
            std::uint32_t ofs = stackPtr[0].Uns;
            if (!ofs) [[unlikely]]
                return OnScriptException("Function pointer is NULL!");
            stackPtr[0].Uns = static_cast<std::int32_t>(opcode - basePtr);
            JUMP(ofs);
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_M1:
        {
            ++stackPtr;
            stackPtr[0].Int = -1;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_0:
        {
            ++stackPtr;
            stackPtr[0].Any = 0;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_1:
        {
            ++stackPtr;
            stackPtr[0].Int = 1;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_2:
        {
            ++stackPtr;
            stackPtr[0].Int = 2;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_3:
        {
            ++stackPtr;
            stackPtr[0].Int = 3;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_4:
        {
            ++stackPtr;
            stackPtr[0].Int = 4;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_5:
        {
            ++stackPtr;
            stackPtr[0].Int = 5;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_6:
        {
            ++stackPtr;
            stackPtr[0].Int = 6;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_7:
        {
            ++stackPtr;
            stackPtr[0].Int = 7;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_FM1:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0xBF800000;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_F0:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x00000000;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_F1:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x3F800000;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_F2:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40000000;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_F3:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40400000;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_F4:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40800000;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_F5:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40A00000;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_F6:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40C00000;
            goto NEXT;
        }
        case shared::scrOpcode::PUSH_CONST_F7:
        {
            ++stackPtr;
            stackPtr[0].Uns = 0x40E00000;
            goto NEXT;
        }
        case shared::scrOpcode::IS_BIT_SET:
        {
            --stackPtr;
            stackPtr[0].Int = (stackPtr[0].Int & (1 << stackPtr[1].Int)) != 0;
            goto NEXT;
        }
        }

        [[unlikely]] return OnScriptException("Unknown opcode: 0x%02X", static_cast<std::uint32_t>(*opcode));
    }
}