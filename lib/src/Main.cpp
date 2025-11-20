#include "core/Memory.hpp"
#include "core/PipeServer.hpp"
#include "debug/ScriptBreakpoint.hpp"
#include "rage/enhanced/scrThread.hpp"
#include "rage/legacy/scrThread.hpp"
#include "rage/shared/Joaat.hpp"
#include <MinHook.h>

std::uint32_t GetModuleHash()
{
    char modulePath[MAX_PATH];
    DWORD len = GetModuleFileNameA(0, modulePath, MAX_PATH);
    if (len == 0 || len == MAX_PATH)
        return 0;

    std::filesystem::path path(modulePath);
    std::string name = path.filename().string();

    return rage::shared::Joaat(name);
}

DWORD Main(PVOID)
{
    auto hash = GetModuleHash();
    switch (hash)
    {
    case "GTA5.exe"_J:
    {
        auto pattern = Memory::ScanPattern("E8 ? ? ? ? 48 85 FF 48 89 1D");
        if (!pattern)
            return EXIT_FAILURE;

        auto addr = pattern->Add(1).Rip().As<void*>();
        MH_Initialize();
        MH_CreateHook(addr, reinterpret_cast<void*>(rage::legacy::scrThread::RunThread), nullptr);
        MH_EnableHook(addr);
        break;
    }
    case "GTA5_Enhanced.exe"_J:
    {
        auto pattern = Memory::ScanPattern("49 63 41 1C");
        if (!pattern)
            return EXIT_FAILURE;

        auto addr = pattern->Sub(0x24).As<void*>();
        MH_Initialize();
        MH_CreateHook(addr, reinterpret_cast<void*>(rage::enhanced::scrThread::RunThread), nullptr);
        MH_EnableHook(addr);
        g_IsEnhanced = true;
        break;
    }
    default:
        MessageBoxA(0, "Unknown module.", "Module", MB_ICONERROR);
        return EXIT_FAILURE;
    }

    if (!PipeServer::Init("scrDbg"))
    {
        MessageBoxA(0, "Failed to initialize scrDbg pipe server.", "Pipe", MB_ICONERROR);
        return EXIT_FAILURE;
    }

    PipeServer::Run();

    ScriptBreakpoint::RemoveAll();
    ScriptBreakpoint::SetPauseGame(false);
    PipeServer::Destroy();
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    CloseHandle(g_MainThread);
    FreeLibraryAndExitThread(g_DllInstance, 0);

    return EXIT_SUCCESS;
}

BOOL WINAPI DllMain(HINSTANCE dllInstance, DWORD reason, PVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(dllInstance);
        g_DllInstance = dllInstance;
        g_MainThread = CreateThread(nullptr, 0, Main, nullptr, 0, nullptr);
    }

    return TRUE;
}