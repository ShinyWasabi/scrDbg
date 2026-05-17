#pragma once
#include "scrThreadContext.hpp"

namespace rage
{
    template <typename T>
    class atArray;
    class scrProgram;
    union scrValue;

    class scrThread
    {
    public:
        scrThreadContext* Context()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThread_GEN9*>(this)->m_Context : &reinterpret_cast<scrThread_GEN8*>(this)->m_Context;
        }

        scrValue* Stack()
        {
            return scrDbgLib::g_IsEnhanced ? reinterpret_cast<scrThread_GEN9*>(this)->m_Stack : reinterpret_cast<scrThread_GEN8*>(this)->m_Stack;
        }

        uint32_t* ArgSize()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThread_GEN9*>(this)->m_ArgSize : &reinterpret_cast<scrThread_GEN8*>(this)->m_ArgSize;
        }

        uint32_t* ArgLoc()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThread_GEN9*>(this)->m_ArgLoc : &reinterpret_cast<scrThread_GEN8*>(this)->m_ArgLoc;
        }

        const char* ErrorMessage()
        {
            return scrDbgLib::g_IsEnhanced ? reinterpret_cast<scrThread_GEN9*>(this)->m_ErrorMessage : reinterpret_cast<scrThread_GEN8*>(this)->m_ErrorMessage;
        }

        uint32_t* ScriptHash()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThread_GEN9*>(this)->m_ScriptHash : &reinterpret_cast<scrThread_GEN8*>(this)->m_ScriptHash;
        }

        const char* ScriptName()
        {
            return scrDbgLib::g_IsEnhanced ? reinterpret_cast<scrThread_GEN9*>(this)->m_ScriptName : reinterpret_cast<scrThread_GEN8*>(this)->m_ScriptName;
        }

        void Reset(uint32_t programHash, void* args, uint32_t argCount)
        {
            scrDbgLib::g_IsEnhanced ? reinterpret_cast<scrThread_GEN9*>(this)->Reset(programHash, args, argCount) : reinterpret_cast<scrThread_GEN8*>(this)->Reset(programHash, args, argCount);
        }

        scrThreadState Run()
        {
            return scrDbgLib::g_IsEnhanced ? reinterpret_cast<scrThread_GEN9*>(this)->Run() : reinterpret_cast<scrThread_GEN8*>(this)->Run();
        }

        scrThreadState Update()
        {
            return scrDbgLib::g_IsEnhanced ? reinterpret_cast<scrThread_GEN9*>(this)->Update() : reinterpret_cast<scrThread_GEN8*>(this)->Update();
        }

        void Kill()
        {
            scrDbgLib::g_IsEnhanced ? reinterpret_cast<scrThread_GEN9*>(this)->Kill() : reinterpret_cast<scrThread_GEN8*>(this)->Kill();
        }

        void GetInfo(void* info)
        {
            scrDbgLib::g_IsEnhanced ? reinterpret_cast<scrThread_GEN9*>(this)->GetInfo(info) : (void)0;
        }

        static scrThread* GetCurrentThread();
        static scrThread* GetThread(uint32_t hash);
        static scrThreadState OnScriptException(const char* fmt, ...);
        static scrThreadState RunThread(scrValue* stack, scrValue** globals, scrProgram* program, scrThreadContext* context);

    private:
        struct scrThread_GEN8
        {
            virtual ~scrThread_GEN8() = default;
            virtual void Reset(uint32_t programHash, void* args, uint32_t argCount) = 0;
            virtual scrThreadState Run() = 0;
            virtual scrThreadState Update() = 0;
            virtual void Kill() = 0;

            scrThreadContext m_Context;
            char m_Pad1[0xA0];
            scrValue* m_Stack;
            char m_Pad2[0x04];
            uint32_t m_ArgSize;
            uint32_t m_ArgLoc;
            char m_Pad3[0x04];
            const char* m_ErrorMessage;
            uint32_t m_ScriptHash;
            char m_ScriptName[64];
        };
        static_assert(sizeof(scrThread_GEN8) == 0x118);

        struct scrThread_GEN9
        {
            virtual ~scrThread_GEN9() = default;
            virtual void Reset(uint32_t programHash, void* args, uint32_t argCount) = 0;
            virtual scrThreadState Run() = 0;
            virtual scrThreadState Update() = 0;
            virtual void Kill() = 0;
            virtual void GetInfo(void* info) = 0;

            scrThreadContext m_Context;
            char m_Pad1[0xA8];
            scrValue* m_Stack;
            char m_Pad2[0x04];
            uint32_t m_ArgSize;
            uint32_t m_ArgLoc;
            char m_Pad3[0x04];
            char m_ErrorMessage[128];
            uint32_t m_ScriptHash;
            char m_ScriptName[64];
        };
        static_assert(sizeof(scrThread_GEN9) == 0x198);
    };
}