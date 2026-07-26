#include "core/PipeServer.hpp"
#include "core/ScriptFiber.hpp"
#include "debugger/VMLogger.hpp"
#if defined(_M_IX86)
#include "game/GTA4.hpp"
#include "game/Payne.hpp"
#elif defined(_M_X64)
#include "game/GTA5.hpp"
#include "game/RDR2.hpp"
#include "game/RDR3.hpp"
#endif

namespace scrDbgLib
{
    static uint32_t GetModuleHash()
    {
        char modulePath[MAX_PATH];
        DWORD len = GetModuleFileNameA(0, modulePath, MAX_PATH);
        if (len == 0 || len == MAX_PATH)
            return 0;

        std::filesystem::path path(modulePath);
        std::string name = path.filename().string();

        return JOAAT(name);
    }

    static DWORD WINAPI Main(PVOID)
    {
        auto hash = GetModuleHash();
        switch (hash)
        {
#if defined(_M_IX86)
        case "GTAIV.exe"_J:
        {
            g_Game = std::make_unique<GTA4>();
            break;
        }
        case "MaxPayne3.exe"_J:
        {
            g_Game = std::make_unique<Payne>();
            break;
        }
#elif defined(_M_X64)
        case "RDR.exe"_J:
        {
            g_Game = std::make_unique<RDR2>();
            break;
        }
        case "GTA5.exe"_J:
        {
            g_Game = std::make_unique<GTA5_GEN8>();
            break;
        }
        case "GTA5_Enhanced.exe"_J:
        {
            g_Game = std::make_unique<GTA5_GEN9>();
            break;
        }
        case "RDR2.exe"_J:
        {
            g_Game = std::make_unique<RDR3>();
            break;
        }
#endif
        }

        if (!g_Game)
            Game::Shutdown("Failed to attach to a game.");

        if (!g_Game->InitPointers())
            Game::Shutdown("Failed to initialize pointers.");

        if (!g_Game->InitHooks())
            Game::Shutdown("Failed to initialize hooking.");

        if (!VMLogger::Init("VM.log"))
            Game::Shutdown("Failed to initialize VM logger.");

        if (!PipeServer::Init("scrDbg"))
            Game::Shutdown("Failed to initialize scrDbg pipe server.");

        auto module = static_cast<HMODULE>(g_DllInstance);
        if (!NativeDB::Load(module, g_Game->GetResourceId()))
            Game::Shutdown("Failed to load natives database.");

        ScriptFiber::Init();

        PipeServer::Run();

        Game::Shutdown();
        return EXIT_SUCCESS;
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