// cl /EHsc /O2 /W4 /MD scrDbg-x86-injector.cpp /Fe:"..\resources\scrDbg-x86-injector.exe" /link
#include <string>
#include <Windows.h>

int main(int argc, char* argv[])
{
    if (argc < 3)
        return 1;

    DWORD pid = 0;
    try
    {
        pid = std::stoul(argv[1]);
    }
    catch (...)
    {
        return 1;
    }

    const char* dllPath = argv[2];
    size_t pathLen = strlen(dllPath) + 1;

    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process)
        return 1;

    LPVOID remoteMemory = VirtualAllocEx(process, NULL, pathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMemory)
    {
        CloseHandle(process);
        return 1;
    }

    if (!WriteProcessMemory(process, remoteMemory, dllPath, pathLen, NULL))
    {
        VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(process);
        return 1;
    }

    LPVOID loadLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

    HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibrary, remoteMemory, 0, NULL);
    if (!hThread)
    {
        VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(process);
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(process);

    return 0;
}