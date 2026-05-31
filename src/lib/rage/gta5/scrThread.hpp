#pragma once
#include "rage/shared/scrValue.hpp"
#include "scrOpcode.hpp"

#if defined(_M_X64)

namespace rage
{
    class scrProgram;

    enum class scrThreadState : uint32_t
    {
        RUNNING = 0U,
        WAITING = 1U,
        KILLED = 2U,
        PAUSED = 3U
    };

    enum class scrThreadPriority : uint32_t
    {
        HIGHEST = 0U,
        NORMAL = 1U,
        LOWEST = 2U,
        MANUAL_UPDATE = 100U
    };

    template <typename T>
    T GetTLSInternal();

    template <typename T>
    T GetByHashInternal(uint32_t hash);

    template <typename T>
    scrThreadState OnExceptionInternal(uint32_t pc, scrOpcode op, const char* fmt, va_list args);

    template <typename T>
    scrThreadState RunThreadInternal(scrValue* stack, scrValue** globals, scrProgram* program, typename T::Context* context);

    class scrThreadGEN8
    {
    public:
        struct Context
        {
            uint32_t m_Id;
            uint32_t m_ProgramHash;
            scrThreadState m_State;
            uint32_t m_Pc;
            uint32_t m_Fp;
            uint32_t m_Sp;
            float m_TimerA;
            float m_TimerB;
            float m_WaitTime;
            uint32_t m_MinPc; // unused, debug-only
            uint32_t m_MaxPc; // unused, debug-only
            char m_Pad2[0x04];
            scrValue m_Tls[0x04]; // unused
            uint32_t m_StackSize;
            uint32_t m_CatchPc; // unused, debug-only
            uint32_t m_CatchFp; // unused, debug-only
            uint32_t m_CatchSp; // unused, debug-only
            scrThreadPriority m_Priority;
            uint8_t m_CallDepth;
            char m_Pad3[0x03];
            uint32_t m_CallStack[16];
        };

        struct Snapshot
        {
            uint32_t m_Id;
            uint32_t m_ProgramHash;
            uint32_t m_CreateTime;
            bool m_HasNetComponent;
            uint8_t m_HostBD[10];
            uint8_t m_PlayerBD[10];
            bool m_IsSet;
            char m_Pad2[0x02];
        };

        struct TLS
        {
            scrThreadGEN8** m_CurrentThread;
            bool* m_CurrentThreadActive;
        };

        virtual ~scrThreadGEN8() = default;
        virtual void Reset(uint32_t programHash, void* args, uint32_t argCount) = 0;
        virtual scrThreadState Run(uint32_t insnCount) = 0;
        virtual scrThreadState Update(uint32_t insnCount) = 0;
        virtual void Kill() = 0;
        virtual void SetSnapshot(Snapshot* snapshot) = 0;

        Context m_Context;
        scrValue* m_Stack;
        uint32_t m_InsnCount; // unused, debug-only
        uint32_t m_ArgsSize;
        uint32_t m_ArgsOffset;
        uint32_t m_CreateTime; // set by timeGetTime
        const char* m_KillReason;
        uint32_t m_ScriptHash;
        char m_ScriptName[64];
        char m_Pad1[0x04];

        static TLS GetTLS()
        {
            return GetTLSInternal<scrThreadGEN8::TLS>();
        }

        static scrThreadGEN8* GetByHash(uint32_t hash)
        {
            return GetByHashInternal<scrThreadGEN8*>(hash);
        }

        static scrThreadState OnException(uint32_t pc, scrOpcode op, const char* fmt, ...)
        {
            va_list args;
            va_start(args, fmt);

            auto state = OnExceptionInternal<scrThreadGEN8>(pc, op, fmt, args);

            va_end(args);
            return state;
        }

        static scrThreadState RunThread(scrValue* stack, scrValue** globals, scrProgram* program, Context* context)
        {
            return RunThreadInternal<scrThreadGEN8>(stack, globals, program, context);
        }
    };

    class scrThreadGEN9
    {
    public:
        struct Context
        {
            uint32_t m_Id;
            char m_Pad1[0x04];
            uint64_t m_ProgramHash;
            scrThreadState m_State;
            uint32_t m_Pc;
            uint32_t m_Fp;
            uint32_t m_Sp;
            float m_TimerA;
            float m_TimerB;
            float m_WaitTime;
            uint32_t m_MinPc; // unused, debug-only
            uint32_t m_MaxPc; // unused, debug-only
            char m_Pad2[0x04];
            scrValue m_Tls[0x04]; // unused
            uint32_t m_StackSize;
            uint32_t m_CatchPc; // unused, debug-only
            uint32_t m_CatchFp; // unused, debug-only
            uint32_t m_CatchSp; // unused, debug-only
            scrThreadPriority m_Priority;
            uint8_t m_CallDepth;
            char m_Pad3[0x03];
            uint32_t m_CallStack[16];
        };

        struct Snapshot
        {
            uint32_t m_Id;
            char m_Pad1[0x04];
            uint64_t m_ProgramHash;
            uint32_t m_CreateTime;
            bool m_HasNetComponent;
            uint8_t m_HostBD[10];
            uint8_t m_PlayerBD[10];
            bool m_IsSet;
            char m_Pad2[0x02];
        };

        struct TLS
        {
            scrThreadGEN9** m_CurrentThread;
            bool* m_CurrentThreadActive;
        };

        virtual ~scrThreadGEN9() = default;
        virtual void Reset(uint32_t programHash, void* args, uint32_t argCount) = 0;
        virtual scrThreadState Run(uint32_t insnCount) = 0;
        virtual scrThreadState Update(uint32_t insnCount) = 0;
        virtual void Kill() = 0;
        virtual void SetSnapshot(Snapshot* snapshot) = 0;

        Context m_Context;
        scrValue* m_Stack;
        uint32_t m_InsnCount; // unused, debug-only
        uint32_t m_ArgsSize;
        uint32_t m_ArgsOffset;
        uint32_t m_CreateTime; // set by timeGetTime
        char m_KillReason[128];
        uint32_t m_ScriptHash;
        char m_ScriptName[64];
        char m_Pad1[0x04];

        static TLS GetTLS()
        {
            return GetTLSInternal<scrThreadGEN9::TLS>();
        }

        static scrThreadGEN9* GetByHash(uint32_t hash)
        {
            return GetByHashInternal<scrThreadGEN9*>(hash);
        }

        static scrThreadState OnException(uint32_t pc, scrOpcode op, const char* fmt, ...)
        {
            va_list args;
            va_start(args, fmt);

            auto state = OnExceptionInternal<scrThreadGEN9>(pc, op, fmt, args);

            va_end(args);
            return state;
        }

        static scrThreadState RunThread(scrValue* stack, scrValue** globals, scrProgram* program, Context* context)
        {
            return RunThreadInternal<scrThreadGEN9>(stack, globals, program, context);
        }
    };
}

#endif