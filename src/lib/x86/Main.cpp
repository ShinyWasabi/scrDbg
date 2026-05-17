DWORD WINAPI Main(LPVOID)
{
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