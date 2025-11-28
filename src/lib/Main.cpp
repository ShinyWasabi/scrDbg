#include "Hooking.hpp"
#include "Pointers.hpp"
#include "ResourceLoader.hpp"
#include "core/PipeServer.hpp"
#include "debug/ScriptBreakpoint.hpp"
#include "debug/ScriptFunctionNames.hpp"
#include "debug/ScriptLogger.hpp"
#include "rage/shared/Joaat.hpp"

namespace scrDbgLib
{
    static std::uint32_t GetModuleHash()
    {
        char modulePath[MAX_PATH];
        DWORD len = GetModuleFileNameA(0, modulePath, MAX_PATH);
        if (len == 0 || len == MAX_PATH)
            return 0;

        std::filesystem::path path(modulePath);
        std::string name = path.filename().string();

        return rage::shared::Joaat(name);
    }

    static int Cleanup(int exitcode, const char* message = nullptr)
    {
        if (exitcode == EXIT_FAILURE && message)
            MessageBoxA(0, message, "Error", MB_ICONERROR);

        ScriptBreakpoint::RemoveAll();
        ScriptBreakpoint::SetPauseGame(false);
        PipeServer::Destroy();
        ScriptLogger::Destroy();
        Hooking::Destroy();

        CloseHandle(g_MainThread);
        FreeLibraryAndExitThread(g_DllInstance, 0);
        return exitcode;
    }

    static DWORD Main(PVOID)
    {
        auto hash = GetModuleHash();
        if (hash == "GTA5_Enhanced.exe"_J)
            g_IsEnhanced = true;

        if (!g_Pointers.Init())
            return Cleanup(EXIT_FAILURE, "Failed to initialize pointers.");

        if (!Hooking::Init())
            return Cleanup(EXIT_FAILURE, "Failed to initialize hooking.");

        if (!ScriptLogger::Init("scrDbg.log"))
            return Cleanup(EXIT_FAILURE, "Failed to initialize script logger.");

        if (!PipeServer::Init("scrDbg"))
            return Cleanup(EXIT_FAILURE, "Failed to initialize scrDbg pipe server.");

        if (!scrDbgShared::NativesBin::Load(static_cast<HMODULE>(g_DllInstance)))
            MessageBoxA(0, "Failed to load native names.", "Warning", MB_ICONWARNING);

        ScriptFunctionNames::GenerateNamesForAllPrograms();

        PipeServer::Run();

        return Cleanup(EXIT_SUCCESS);
    }
}

BOOL WINAPI DllMain(HINSTANCE dllInstance, DWORD reason, PVOID)
{
    using namespace scrDbgLib;

    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(dllInstance);
        g_DllInstance = dllInstance;
        g_MainThread = CreateThread(nullptr, 0, Main, nullptr, 0, nullptr);
    }

    return TRUE;
}