#include "Process.hpp"

namespace scrDbg
{
    bool Process::InitImpl(const wchar_t* processName)
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
            return false;

        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(entry);

        uint32_t pid = 0;
        if (Process32First(snapshot, &entry))
        {
            do
            {
                if (_wcsicmp(entry.szExeFile, processName) == 0)
                {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);

        if (pid == 0)
            return false;

        m_Handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (!m_Handle)
            return false;

        HMODULE modules[0xFF];
        DWORD pcbNeeded;
        if (!EnumProcessModulesEx(m_Handle, modules, sizeof(modules), &pcbNeeded, LIST_MODULES_64BIT))
        {
            CloseHandle(m_Handle);
            m_Handle = nullptr;
            return false;
        }

        m_BaseAddress = reinterpret_cast<uintptr_t>(modules[0]);

        MODULEINFO moduleInfo;
        if (!GetModuleInformation(m_Handle, modules[0], &moduleInfo, sizeof(moduleInfo)))
        {
            CloseHandle(m_Handle);
            m_Handle = nullptr;
            m_BaseAddress = 0;
            return false;
        }

        m_Size = moduleInfo.SizeOfImage;

        std::wstring ws(processName);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        m_Name = converter.to_bytes(ws);

        return true;
    }

    void Process::DestroyImpl()
    {
        if (m_Handle)
        {
            CloseHandle(m_Handle);
            m_Handle = nullptr;
            m_BaseAddress = 0;
            m_Size = 0;
        }
    }

    bool Process::IsRunningImpl()
    {
        return m_Handle && WaitForSingleObject(m_Handle, 0) == WAIT_TIMEOUT;
    }

    bool Process::IsModuleLoadedImpl(const wchar_t* moduleName)
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(m_Handle));
        if (snapshot == INVALID_HANDLE_VALUE)
            return false;

        MODULEENTRY32W me{};
        me.dwSize = sizeof(MODULEENTRY32W);

        bool found = false;
        if (Module32FirstW(snapshot, &me))
        {
            do
            {
                if (_wcsicmp(me.szModule, moduleName) == 0)
                {
                    found = true;
                    break;
                }
            } while (Module32NextW(snapshot, &me));
        }

        CloseHandle(snapshot);
        return found;
    }

    bool Process::ReadRawImpl(uintptr_t base, void* value, size_t size)
    {
        return ReadProcessMemory(m_Handle, reinterpret_cast<const void*>(base), value, size, nullptr);
    }

    bool Process::WriteRawImpl(uintptr_t base, const void* value, size_t size)
    {
        return WriteProcessMemory(m_Handle, reinterpret_cast<void*>(base), value, size, nullptr);
    }
}