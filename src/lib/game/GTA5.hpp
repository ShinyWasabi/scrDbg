#pragma once

#if defined(_M_X64)

namespace rage
{
    class scrNativeRegistration;
    class scrProgramRegistry;
    union scrValue;
    class scrThreadGEN8;
    class scrThreadGEN9;
    template <typename T>
    class atArray;
}

namespace scrDbgLib
{
    class GTA5 : public Game
    {
    public:
        struct Pointers;

        Debugger* GetDebugger() const override
        {
            return m_Debugger.get();
        }

        int GetResourceId() const override
        {
            return NATIVES_GTA5_BIN;
        }

        static const Pointers& GetPointers()
        {
            return m_Pointers;
        }

    protected:
        struct Pointers
        {
            rage::scrNativeRegistration* NativeRegistrationTable;
            rage::scrProgramRegistry* ScriptProgramRegistry;
            rage::atArray<rage::scrThreadGEN8*>* ScriptThreadsGEN8;
            rage::atArray<rage::scrThreadGEN9*>* ScriptThreadsGEN9;
            void* RunScriptThread;
            bool* TimerUserPause;
            bool* TimerScriptPause;
        };

        static inline Pointers m_Pointers;
    };

    class GTA5_GEN8 : public GTA5
    {
    public:
        explicit GTA5_GEN8();

        bool InitPointers() const override;
        bool InitHooks() const override;

        GameType GetType() const override
        {
            return GameType::GTA5_GEN8;
        }
    };

    class GTA5_GEN9 : public GTA5
    {
    public:
        explicit GTA5_GEN9();

        bool InitPointers() const override;
        bool InitHooks() const override;

        GameType GetType() const override
        {
            return GameType::GTA5_GEN9;
        }
    };
}

#endif