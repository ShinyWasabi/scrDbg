#pragma once
#include "rage/shared/scrNativeContext.hpp"
#include "rage/shared/scrValue.hpp"
#include "scrOpcode.hpp"

#if defined(_M_IX86)

namespace rage
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
            uint32_t m_ProgramHash;
            State m_State;
            uint32_t m_Pc;
            uint32_t m_Fp;
            uint32_t m_Sp;
            uint32_t m_TimerA;
            uint32_t m_TimerB;
            uint32_t m_TimerC;
            uint32_t m_WaitTime;
            uint32_t m_MinPc;
            uint32_t m_MaxPc;
            scrValue m_Tls[4];
            uint32_t m_StackSize;
            uint32_t m_CatchPc;
            uint32_t m_CatchFp;
            uint32_t m_CatchSp;
            int32_t m_ProtectedSp;
        };

        struct ThreadStack
        {
            scrThread* m_Thread;
            uint32_t m_StackSize;
            scrValue* m_Stack;
        };

        virtual ~scrThread() = default;
        virtual void Reset(uint32_t programHash, void* args, uint32_t argsSize) = 0;
        virtual State Run(int32_t insnCount) = 0;
        virtual State Update(int32_t insnCount) = 0;
        virtual void Kill() = 0;

        Context m_Context;
        scrValue* m_Stack;
        uint32_t m_InsnCount;
        uint8_t* m_ProtectedStack;
        int32_t m_ProtectedSlotIndex;
        scrValue* m_ProtectedTempStack;
        const char* m_KillReason;
        char m_ScriptName[24]; // This actually belongs to GtaThread in GTA 4

        State OnException(uint32_t pc, scrOpcode op, const char* fmt, ...);

        static scrThread* GetByHash(uint32_t hash);
        static scrNativeContext::Handler GetCommandHandler(uint32_t hash);
        static uint32_t GetCommandHash(scrNativeContext::Handler handler);
        static State __fastcall RunThread(scrThread* _this, void* edx, int32_t insnCount);
    };
}

#endif