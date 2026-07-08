#pragma once
#include "rage/rdr3/scrThread.hpp"

#if defined(_M_X64)

namespace rage
{
    template <typename T>
    class atArray;
    union scrValue;

    namespace rdr3
    {
        class scrNativeRegistration;
        class scrProgramRegistry;
    }
}

namespace scrDbgLib
{
    class RDR3 : public Game
    {
    public:
        struct Pointers;

        explicit RDR3();

        bool InitPointers() const override;
        bool InitHooks() const override;

        GameType GetType() const override
        {
            return GameType::RDR3;
        }

        int GetResourceId() const override
        {
            return NATIVES_RDR3_BIN;
        }

        static const Pointers& GetPointers()
        {
            return m_Pointers;
        }

    private:
        struct Pointers
        {
            uint64_t* NativeRegistrationSeed;
            rage::rdr3::scrNativeRegistration** NativeRegistrationTable;
            rage::rdr3::scrProgramRegistry* ScriptProgramRegistry;
            rage::rdr3::scrThread** CurrentScriptThread;
            rage::atArray<rage::rdr3::scrThread*>* ScriptThreads;
            void* RunScriptThread;
            void (*ResolveSecureScriptGlobal)(rage::scrValue*, rage::scrValue*);
            void (*ResolveSecureScriptFrame)(rage::scrValue*, rage::scrValue*, rage::rdr3::scrThread::Context*);
            void (*ResolveSecureScriptStatic)(rage::scrValue*, rage::scrValue*, rage::rdr3::scrThread::Context*);
            void (*DecryptSecureScriptValue)(void*, uint32_t*);
            void (*SetSecureScriptValue)(void*, uint32_t*);
            void (*ResetSecureScriptFrame)(void*);
            bool* TimerUserPause;
            bool* TimerScriptPause;
        };

        static inline Pointers m_Pointers;
    };
}

#endif