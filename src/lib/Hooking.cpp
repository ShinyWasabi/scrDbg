#include "Hooking.hpp"
#include "Pointers.hpp"
#include "rage/Joaat.hpp"
#include "rage/scrProgram.hpp"
#include "rage/scrThread.hpp"
#include <MinHook.h>

namespace scrDbgLib
{
    bool Hooking::Init()
    {
        if (MH_Initialize() != MH_OK)
            return false;

        AddHook("scrProgramConstructor"_J, g_Pointers.ScriptProgramConstructor1, rage::scrProgram::scrProgramConstructor);
        AddHook("ScriptProgramDestructor"_J, g_Pointers.ScriptProgramDestructor, rage::scrProgram::scrProgramDestructor);
        AddHook("LoadScriptProgramSCO"_J, g_Pointers.LoadScriptProgramSCO, rage::scrProgram::LoadScriptProgramSCO);
        AddHook("RunScriptThread"_J, g_Pointers.RunScriptThread, rage::scrThread::RunThread);

        for (auto& hook : m_Hooks)
        {
            if (MH_CreateHook(hook.m_Target, hook.m_Detour, &hook.m_Original) != MH_OK)
                return false;

            if (MH_EnableHook(hook.m_Target) != MH_OK)
                return false;
        }

        return true;
    }

    void Hooking::Destroy()
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_RemoveHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }
}