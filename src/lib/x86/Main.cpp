#include "core/Memory.hpp"
#include "rage/scrProgram.hpp"
#include "rage/scrThread.hpp"
#include <MinHook.h>

DWORD WINAPI Main(LPVOID)
{
    Logger::Init("GTA4VM.log");

    if (!rage::scrProgram::Init())
    {
        LOG("Failed to initialize rage::scrProgram.");
        return EXIT_FAILURE;
    }

    if (!rage::scrThread::Init())
    {
        LOG("Failed to initialize rage::scrThread.");
        return EXIT_FAILURE;
    }

    auto pattern = Memory::ScanPattern("55 8B EC 83 E4 ? 81 EC ? ? ? ? 56 57 8B F9 89 7C 24 ? 8B 47 ? 83 F8");
    if (!pattern)
    {
        LOG("Failed to initialize VM hook.");
        return EXIT_FAILURE;
    }

    auto addr = pattern->As<void*>();
    MH_Initialize();
    MH_CreateHook(addr, reinterpret_cast<void*>(rage::scrThread::RunThread), nullptr);
    MH_EnableHook(addr);
    LOG("VM hook initialized.");

    return EXIT_SUCCESS;
}

BOOL WINAPI DllMain(HINSTANCE dllInstance, DWORD reason, PVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(dllInstance);
        CreateThread(nullptr, 0, Main, nullptr, 0, nullptr);
    }

    return TRUE;
}