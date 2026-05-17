#pragma once

namespace rage
{
    template <typename T>
    class atArray;
    class scrNativeRegistration;
    class scrProgram;
    class scrThread;
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
        rage::scrNativeRegistration* NativeRegistrationTable;
        rage::scrProgram** ScriptPrograms;
        PVOID ScriptProgramConstructor1; // for YSC
        PVOID LoadScriptProgramSCO;      // for SCO
        PVOID ScriptProgramDestructor;
        rage::atArray<rage::scrThread*>* ScriptThreads;
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