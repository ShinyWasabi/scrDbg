#pragma once

#if defined(_M_X64)

namespace rage
{
    template <typename T>
    class atArray;
    union scrValue;

    namespace gta5
    {
        class scrNativeRegistration;
        class scrProgramRegistry;
        class scrThreadGEN8;
        class scrThreadGEN9;
    }
}

namespace scrDbgLib
{
    class GTA5 : public Game
    {
    public:
        struct Pointers;

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
            rage::gta5::scrNativeRegistration* NativeRegistrationTable;
            rage::gta5::scrProgramRegistry* ScriptProgramRegistry;
            rage::atArray<rage::gta5::scrThreadGEN8*>* ScriptThreadsGEN8;
            rage::atArray<rage::gta5::scrThreadGEN9*>* ScriptThreadsGEN9;
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