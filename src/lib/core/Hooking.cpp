#include "Hooking.hpp"
#include <MinHook.h>

namespace scrDbgLib
{
    bool Hooking::Init()
    {
        if (MH_Initialize() != MH_OK)
            return false;

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