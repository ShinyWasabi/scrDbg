#pragma once

namespace rage
{
    namespace shared
    {
        class scrNativeRegistration;
        class scrProgram;
        template <typename T>
        class atArray;
    }

    namespace legacy
    {
        class scrThread;
    }

    namespace enhanced
    {
        class scrThread;
    }
}

namespace scrDbgLib
{
    namespace Functions
    {
        using PauseGameNow = void (*)();
        using UnpauseGameNow = void (*)();
        using TogglePausedRenderPhases = void (*)(bool, int);
    }

    struct PointerData
    {
        rage::shared::scrNativeRegistration* NativeRegistrationTable;
        rage::shared::scrProgram** ScriptPrograms;
        PVOID ScriptProgramConstructor1; // for YSC
        PVOID LoadScriptProgramSCO;      // for SCO
        PVOID ScriptProgramDestructor;
        rage::shared::atArray<rage::legacy::scrThread*>* ScriptThreadsLegacy;
        rage::shared::atArray<rage::enhanced::scrThread*>* ScriptThreadsEnhanced;
        PVOID RunScriptThread;
        Functions::PauseGameNow PauseGameNow;
        Functions::UnpauseGameNow UnpauseGameNow;
        Functions::TogglePausedRenderPhases TogglePausedRenderPhases;
        std::uint8_t* PauseGameNowPatch;
    };

    struct Pointers : PointerData
    {
        bool Init();
    };

    inline Pointers g_Pointers;
}