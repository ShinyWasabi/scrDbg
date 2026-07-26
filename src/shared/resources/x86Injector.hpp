#pragma once

namespace scrDbg
{
    class x86Injector
    {
    public:
        static bool Run(HMODULE module, int resourceId, DWORD pid, const char* dllPath);

    private:
        static std::string GetTempPathForInjector();
    };
}