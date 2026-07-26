#include "x86Injector.hpp"

namespace scrDbg
{
    std::string x86Injector::GetTempPathForInjector()
    {
        char tempDir[MAX_PATH];
        GetTempPathA(MAX_PATH, tempDir);
        return std::string(tempDir) + "scrDbg-x86-injector.exe";
    }

    bool x86Injector::Run(HMODULE module, int resourceId, DWORD pid, const char* dllPath)
    {
        HRSRC res = FindResource(module, MAKEINTRESOURCE(resourceId), RT_RCDATA);
        if (!res)
            return false;

        HGLOBAL data = LoadResource(module, res);
        if (!data)
            return false;

        DWORD size = SizeofResource(module, res);
        void* ptr = LockResource(data);
        if (!ptr || size == 0)
            return false;

        std::string exePath = GetTempPathForInjector();
        HANDLE file = CreateFileA(exePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE)
            return false;

        DWORD written;
        bool success = WriteFile(file, ptr, size, &written, NULL);
        CloseHandle(file);

        if (!success)
            return false;

        std::string cmd = "\"" + exePath + "\" " + std::to_string(pid) + " \"" + dllPath + "\"";

        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi = {0};

        if (CreateProcessA(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        {
            WaitForSingleObject(pi.hProcess, INFINITE);

            DWORD exitCode = 0;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            DeleteFileA(exePath.c_str());

            return exitCode == 0;
        }

        return false;
    }
}