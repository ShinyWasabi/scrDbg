#include "ResourceLoader.hpp"
#include "scrDbg.h"

namespace scrDbgShared
{
    bool NativesBin::Load(HMODULE module, int id)
    {
        HRSRC res = FindResource(module, MAKEINTRESOURCE(id), RT_RCDATA);
        if (!res)
            return false;

        HGLOBAL data = LoadResource(module, res);
        if (!data)
            return false;

        DWORD size = SizeofResource(module, res);
        const char* ptr = static_cast<const char*>(LockResource(data));
        if (!ptr || size < sizeof(uint32_t))
            return false;

        const char* end = ptr + size;

        uint32_t count = 0;
        memcpy(&count, ptr, sizeof(count));
        ptr += sizeof(count);

        Names.clear();
        Args.clear();
        Rets.clear();

        for (uint32_t i = 0; i < count && ptr < end; ++i)
        {
            if (end - ptr < sizeof(uint64_t) + sizeof(uint16_t))
                break;

            uint64_t hash;
            uint16_t nameLen;
            memcpy(&hash, ptr, sizeof(hash));
            ptr += sizeof(hash);
            memcpy(&nameLen, ptr, sizeof(nameLen));
            ptr += sizeof(nameLen);

            if (end - ptr < nameLen)
                break;

            std::string name(ptr, nameLen);
            ptr += nameLen;

            if (end - ptr < sizeof(uint16_t))
                break;
            uint16_t argCount;
            memcpy(&argCount, ptr, sizeof(argCount));
            ptr += sizeof(argCount);

            std::vector<NativeTypes> args;
            args.reserve(argCount);
            for (uint16_t j = 0; j < argCount && ptr < end; ++j)
            {
                if (end - ptr < 1)
                    break;
                args.push_back(static_cast<NativeTypes>(*ptr));
                ++ptr;
            }

            if (end - ptr < sizeof(uint16_t))
                break;
            uint16_t retCount;
            memcpy(&retCount, ptr, sizeof(retCount));
            ptr += sizeof(retCount);

            std::vector<NativeTypes> rets;
            rets.reserve(retCount);
            for (uint16_t j = 0; j < retCount && ptr < end; ++j)
            {
                if (end - ptr < 1)
                    break;
                rets.push_back(static_cast<NativeTypes>(*ptr));
                ++ptr;
            }

            Names.emplace(hash, std::move(name));
            Args.emplace(hash, std::move(args));
            Rets.emplace(hash, std::move(rets));
        }

        return true;
    }

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