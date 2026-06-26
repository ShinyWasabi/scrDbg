#pragma once
#include "scrCommand.hpp"
#include "scrOpcode.hpp"

#if defined(_M_IX86)

namespace rage::payne
{
    class scrProgram;

    class scrThread
    {
    public:
        enum class State
        {
            RUNNING = 0U,
            WAITING = 1U,
            KILLED = 2U,
            PAUSED = 3U,
            REFRESH = 4U,
            THROW = 5U
        };

        enum class RefType : uint32_t
        {
            INVALID = 0U,  // resolves to nullptr
            LOCAL = 1U,    // function local variable
            STATIC = 2U,   // static variable
            GLOBAL = 3U,   // global variable
            STRING = 4U,   // string literal inside code
            REFERENCE = 5U // indirect reference
        };

        // Address encoding used by LOCAL, STATIC, GLOBAL, and STRING opcodes.
        // Bits 31-29 = type, bits 28-0 = offset (in bytes).
        enum class AddrType : uint32_t
        {
            LOCAL = 0x20000000U,
            STATIC = 0x40000000U,
            GLOBAL = 0x60000000U,
            STRING = 0x80000000U,
            REFERENCE = 0xA0000000U
        };

        struct Context
        {
            uint32_t m_Id;
            uint32_t m_ProgramHash;
            State m_State;
            uint32_t m_Pc;
            uint32_t m_Fp;
            uint32_t m_Sp;
            uint32_t m_TimerA;
            uint32_t m_TimerB;
            uint32_t m_TimerC;
            uint32_t m_WaitTime;
            uint32_t m_StackSize;
            uint32_t m_CatchPc;
            uint32_t m_CatchFp;
            uint32_t m_CatchSp;
            // Flags set by START_NEW_SCRIPT_TYPED. Known bits:
            // bit 2: creates a sevDispatcher in GtaThread::Initialise
            // bits 0-1: if == 2, routes commands with bit 2 set through InvokeSynchronizedCommand
            uint32_t m_TypedFlags;
        };

        virtual ~scrThread() = default;
        virtual void Reset(uint32_t programHash, void* args, uint32_t argsSize) = 0;
        virtual State Run(uint32_t insnCount) = 0;
        virtual const char* GetScriptName() = 0;      // overriden by GtaThread to set m_ScriptName
        virtual const char* GetScriptName2() = 0;     // just calls GetScriptName
        virtual State Update(uint32_t insnCount) = 0; // overriden by GtaThread
        virtual void Kill() = 0;                      // overriden by GtaThread
        // Synchronized native execution via the sev dispatcher system.
        // Only reached when (m_Context.m_TypedFlags & 3) == 2 && (cmd->m_Flags & 4) != 0.
        // If g_SevDispatcherMgr->m_Enabled is true, it executes the native, wraps return values
        // in a sevEvent (type chosen by return size: 0 bytes -> sevEventEmpty, 4 -> sevEventRaw1,
        // 8 -> sevEventRaw2, 12 -> sevEventRaw3; max 3 return values), then induces the event
        // into g_SevDispatcherMgr for other threads to consume.
        // If the pending event has been flushed: copies return values from the event into ctx, resumes thread.
        // Otherwise, sets thread state to WAITING until the event is ready.
        // Overridden by GtaThread, base implementation does nothing.
        virtual void InvokeSynchronizedCommand(scrCommand* cmd, scrCommand::Context* ctx, uint32_t retCount) = 0;
        virtual bool AllocateStack(uint32_t stackSize) = 0; // overriden by GtaThread, which just calls the def impl, virtual in MP3 for some reason
        virtual bool GrowStack(uint32_t stackSize) = 0;     // overriden by GtaThread, which just calls the def impl, which is unused (does nothing and returns false)

        Context m_Context;
        scrValue* m_Stack;
        uint32_t m_InsnCount;
        uint32_t m_ArgsSize;
        uint32_t m_ArgsOffset;
        uint32_t m_Unk; // Neither set nor read anywhere
        const char* m_KillReason;
        // rest belongs to GtaThread
        char m_Pad1[0x2C];
        char m_ScriptName[24];

        scrValue* ResolveAddress(scrValue* ref, scrProgram* program, scrValue* globals);
        scrValue* PopStack();
        State OnException(uint32_t pc, scrOpcode op, const char* fmt, ...);

        static scrThread* GetByHash(uint32_t hash);
        static scrCommand::Context::Handler GetCommandHandler(uint32_t hash);
        static uint32_t GetCommandHash(scrCommand::Context::Handler handler);
        static State __fastcall RunThread(scrThread* _this, void* edx, int32_t insnCount);
    };
}

#endif