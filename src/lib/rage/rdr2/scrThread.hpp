#pragma once
#include "rage/shared/scrNativeContext.hpp"
#include "scrOpcode.hpp"

#if defined(_M_X64)

namespace rage::rdr2
{
    class scrThread
    {
    public:
        enum class State : uint32_t
        {
            RUNNING = 0U,
            WAITING = 1U,
            KILLED = 2U,
            PAUSED = 3U,
            REFRESH = 4U
        };

        struct Context
        {
            uint32_t m_Id;
            char m_Pad1[0x04];
            uint32_t m_ProgramHash; // is this 64 bit like GTA 5 enhanced?
            State m_State;
            uint32_t m_Pc;
            uint32_t m_Fp;
            uint32_t m_Sp;
            float m_TimerA;
            float m_TimerB;
            float m_WaitTime;
            int32_t m_MinPc;
            int32_t m_MaxPc;
            scrValue m_Tls[4]; // RDR is the only RAGE game where this is actually used AFAIK
            uint32_t m_StackSize;
            uint32_t m_CatchPc;
            uint32_t m_CatchFp;
            uint32_t m_CatchSp;
            int32_t m_PatchIndex;
            bool m_Patched;
        };

        virtual ~scrThread() = default;
        virtual void Reset(uint32_t programHash, void* args, uint32_t argsSize) = 0;
        virtual State Run(int32_t insnCount) = 0;
        virtual State Update(int32_t insnCount) = 0;
        virtual void Kill() = 0;

        Context m_Context;
        scrValue* m_Stack;
        uint32_t m_InsnCount;
        uint32_t m_ArgsSize;
        uint32_t m_ArgsOffset;
        uint32_t m_ScriptHash;
        const char* m_KillReason;

        State OnException(uint32_t pc, scrOpcode op, const char* fmt, ...);

        static scrThread* GetByHash(uint32_t hash);
        static scrNativeContext::Handler GetCommandHandler(uint32_t hash);
        static uint32_t GetCommandHash(scrNativeContext::Handler handler);
        static State RunThread(scrThread* _this, int32_t insnCount);
    };
}

#endif