#pragma once

namespace rage::enhanced
{
    class scrThread;

    class tlsContext
    {
    public:
        char m_Pad1[0x7A0];
        scrThread* m_CurrentScriptThread;
        bool m_ScriptThreadActive;

        static tlsContext* Get()
        {
            constexpr std::uint32_t TlsIndex = 0x0;
            return *reinterpret_cast<tlsContext**>(__readgsqword(0x58) + TlsIndex);
        }
    };
    static_assert(sizeof(tlsContext) == 0x7B0);
}