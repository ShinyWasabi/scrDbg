#pragma once
#include "rage/shared/scrValue.hpp"
#include "scrOpcode.hpp"

#if defined(_M_X64)

namespace rage::rdr3
{
    class scrProgram;

    class scrThread
    {
    public:
        enum class State : uint32_t
        {
            RUNNING = 0U,
            WAITING = 1U,
            KILLED = 2U,
            PAUSED = 3U
        };

        enum class Priority : uint32_t
        {
            HIGHEST = 0U,
            NORMAL = 1U,
            LOWEST = 2U,
            MANUAL_UPDATE = 100U
        };

        struct Context
        {
            uint32_t m_Id;
            uint32_t m_ProgramHash;
            State m_State;
            uint32_t m_Pc;
            uint32_t m_Fp;
            uint32_t m_Sp;
            float m_TimerA;
            float m_TimerB;
            float m_WaitTime;
            int32_t m_MinPc;
            int32_t m_MaxPc;
            scrValue m_Tls[4];
            uint32_t m_StackSize;
            uint32_t m_CatchPc;
            uint32_t m_CatchFp;
            uint32_t m_CatchSp;
            Priority m_Priority;
            uint32_t m_CallDepth;
            uint32_t m_CallStack[16];
            void* m_SecureFrames[96];
            void* m_SecureStatics[96];
            void* m_SecureUnk;
        };

        virtual ~scrThread() = default;
        virtual void Reset(uint32_t programHash, void* args, uint32_t argsSize) = 0;
        virtual State Run(uint32_t insnCount) = 0;
        virtual State Update(uint32_t insnCount) = 0;
        virtual void Kill() = 0;

        Context m_Context;
        scrValue* m_Stack;
        uint32_t m_InsnCount;
        uint32_t m_InsnLimit;
        uint32_t m_ArgsSize;
        uint32_t m_ArgsOffset;
        const char* m_KillReason;
        uint32_t m_ScriptHash;

        static uint32_t ScrambleSecureValue(uint32_t value)
        {
            // __ROR4__(__ROL4__(~x, 16), 1) ^ 0xEDB88320
            uint32_t rol = (~value << 16) | (~value >> 16);
            uint32_t ror = (rol >> 1) | (rol << 31);
            return ror ^ 0xEDB88320U;
        }

        static scrThread* GetByHash(uint32_t hash);
        static State OnException(uint32_t pc, scrOpcode op, const char* fmt, ...);
        static void ResolveSecureScriptVariable(scrValue* ref, scrValue* outHandle, Context* context, scrValue* stack, scrProgram* program);
        static State RunThread(scrValue* stack, scrValue** globals, bool* globalBlocksEnabled, scrProgram* program, Context* context);
    };
}

#endif