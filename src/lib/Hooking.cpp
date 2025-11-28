#include "Hooking.hpp"
#include "Pointers.hpp"
#include "rage/enhanced/scrThread.hpp"
#include "rage/legacy/scrThread.hpp"
#include "rage/shared/Joaat.hpp"
#include "rage/shared/scrProgram.hpp"
#include <MinHook.h>

namespace scrDbgLib
{
    bool Hooking::Init()
    {
        if (MH_Initialize() != MH_OK)
            return false;

        AddHook("ScriptProgramConstructor1"_J, g_Pointers.ScriptProgramConstructor1, rage::shared::scrProgram::scrProgramConstructor1);
        AddHook("LoadScriptProgramSCO"_J, g_Pointers.LoadScriptProgramSCO, rage::shared::scrProgram::LoadScriptProgramSCO);
        AddHook("ScriptProgramDestructor"_J, g_Pointers.ScriptProgramDestructor, rage::shared::scrProgram::scrProgramDestructor);
        if (g_IsEnhanced)
            AddHook("RunScriptThread"_J, g_Pointers.RunScriptThread, rage::enhanced::scrThread::RunThread);
        else
            AddHook("RunScriptThread"_J, g_Pointers.RunScriptThread, rage::legacy::scrThread::RunThread);

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