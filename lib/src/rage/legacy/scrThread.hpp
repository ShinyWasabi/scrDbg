#pragma once
#include "rage/shared/atArray.hpp"
#include "scrThreadContext.hpp"

namespace rage::shared
{
    union scrValue;
    class scrProgram;
}

namespace rage::legacy
{
    class scrThread
    {
    public:
        virtual ~scrThread() = default;
        virtual void Reset(std::uint32_t scriptHash, void* args, std::uint32_t argCount) = 0;
        virtual scrThreadState Run(std::uint32_t opsToExecute) = 0;
        virtual scrThreadState Update(std::uint32_t opsToExecute) = 0;
        virtual void Kill() = 0;

        scrThreadContext m_Context;
        shared::scrValue* m_Stack;
        char m_Pad1[0x04];
        std::uint32_t m_ArgSize;
        std::uint32_t m_ArgLoc;
        char m_Pad2[0x04];
        const char* m_ErrorMessage;
        std::uint32_t m_ScriptHash;
        char m_ScriptName[64];

        static scrThread* GetCurrentThread();
        static scrThread* GetThread(std::uint32_t hash);
        static scrThreadState OnScriptException(const char* fmt, ...);
        static bool ProcessBreakpoints();
        static bool ResumeActiveBreakpoint();
        static scrThreadState RunThread(shared::scrValue* stack, shared::scrValue** globals, shared::scrProgram* program, scrThreadContext* context);

        static inline shared::atArray<scrThread*>* m_Threads;
    };
    static_assert(sizeof(scrThread) == 0x118);
}