#include "core/Memory.hpp"
#include "core/PipeServer.hpp"
#include "debug/ScriptBreakpoint.hpp"
#include "debug/ScriptLogger.hpp"
#include "rage/enhanced/scrThread.hpp"
#include "rage/legacy/scrThread.hpp"
#include "rage/shared/Joaat.hpp"
#include "rage/shared/scrNativeRegistration.hpp"
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

bool InitPointers()
{
    if (g_IsEnhanced ? !rage::enhanced::scrThread::Init() : !rage::legacy::scrThread::Init())
        return false;

    if (!rage::shared::scrNativeRegistration::Init())
        return false;

    if (!ScriptBreakpoint::Init())
        return false;

    return true;
}

int Cleanup(int exitcode, const char* message = nullptr)
{
    if (exitcode == EXIT_FAILURE && message)
        MessageBoxA(0, message, "Error", MB_ICONERROR);

    ScriptBreakpoint::RemoveAll();
    ScriptBreakpoint::SetPauseGame(false);
    PipeServer::Destroy();
    ScriptLogger::Destroy();
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    CloseHandle(g_MainThread);
    FreeLibraryAndExitThread(g_DllInstance, 0);
    return exitcode;
}

DWORD Main(PVOID)
{
    auto hash = GetModuleHash();
    switch (hash)
    {
    case "GTA5.exe"_J:
    {
        auto addr = Memory::ScanPattern("E8 ? ? ? ? 48 85 FF 48 89 1D");
        if (!addr)
            return Cleanup(EXIT_FAILURE, "Failed to find pattern for hook.");

        auto ptr = addr->Add(1).Rip().As<void*>();
        MH_Initialize();
        MH_CreateHook(ptr, reinterpret_cast<void*>(rage::legacy::scrThread::RunThread), nullptr);
        MH_EnableHook(ptr);
        break;
    }
    case "GTA5_Enhanced.exe"_J:
    {
        auto addr = Memory::ScanPattern("49 63 41 1C");
        if (!addr)
            return Cleanup(EXIT_FAILURE, "Failed to find pattern for hook.");

        auto ptr = addr->Sub(0x24).As<void*>();
        MH_Initialize();
        MH_CreateHook(ptr, reinterpret_cast<void*>(rage::enhanced::scrThread::RunThread), nullptr);
        MH_EnableHook(ptr);
        g_IsEnhanced = true;
        break;
    }
    default:
        return Cleanup(EXIT_FAILURE, "Unknown module.");
    }

    if (!InitPointers())
        return Cleanup(EXIT_FAILURE, "Failed to initialize pointers.");

    if (!ScriptLogger::Init("scrDbg.log"))
        return Cleanup(EXIT_FAILURE, "Failed to initialize script logger.");

    if (!PipeServer::Init("scrDbg"))
        return Cleanup(EXIT_FAILURE, "Failed to initialize scrDbg pipe server.");

    PipeServer::Run();

    return Cleanup(EXIT_SUCCESS);
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