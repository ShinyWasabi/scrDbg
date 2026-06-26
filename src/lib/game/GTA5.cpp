#include "GTA5.hpp"
#include "core/Hooking.hpp"
#include "core/Scanner.hpp"
#include "debugger/DebuggerGTA5.hpp"
#include "rage/gta5/scrThread.hpp"

#if defined(_M_X64)

namespace scrDbgLib
{
    GTA5_GEN8::GTA5_GEN8()
    {
        m_Debugger = std::make_unique<DebuggerGTA5_GEN8>();
    }

    bool GTA5_GEN8::InitPointers() const
    {
        Scanner scanner;

        scanner.Add("48 8D 0D ? ? ? ? 48 8B 14 FA E8 ? ? ? ? 48 85 C0 75 0A", [this](Memory addr) {
            m_Pointers.NativeRegistrationTable = addr.Add(3).Rip().As<decltype(m_Pointers.NativeRegistrationTable)>();
        });

        scanner.Add("48 8D 0D ? ? ? ? 41 8B D6 E8 ? ? ? ? FF 05", [this](Memory addr) {
            m_Pointers.ScriptProgramRegistry = addr.Add(3).Rip().As<decltype(m_Pointers.ScriptProgramRegistry)>();
        });

        scanner.Add("45 33 F6 8B E9 85 C9 B8", [this](Memory addr) {
            m_Pointers.ScriptThreadsGEN8 = addr.Sub(4).Rip().Sub(8).As<decltype(m_Pointers.ScriptThreadsGEN8)>();
        });

        scanner.Add("E8 ? ? ? ? 48 85 FF 48 89 1D", [this](Memory addr) {
            m_Pointers.RunScriptThread = addr.Add(1).Rip().As<decltype(m_Pointers.RunScriptThread)>();
        });

        scanner.Add("0A 05 ? ? ? ? 23 0D", [this](Memory addr) {
            m_Pointers.TimerUserPause = addr.Add(2).Rip().As<decltype(m_Pointers.TimerUserPause)>();
            m_Pointers.TimerScriptPause = addr.Add(2).Rip().Add(1).As<decltype(m_Pointers.TimerScriptPause)>();
        });

        return scanner.Scan();
    }

    bool GTA5_GEN8::InitHooks() const
    {
        Hooking::AddHook(m_Pointers.RunScriptThread, rage::gta5::scrThreadGEN8::RunThread);

        return Hooking::Init();
    }

    GTA5_GEN9::GTA5_GEN9()
    {
        m_Debugger = std::make_unique<DebuggerGTA5_GEN9>();
    }

    bool GTA5_GEN9::InitPointers() const
    {
        Scanner scanner;

        scanner.Add("4C 8D 0D ? ? ? ? 4C 8D 15 ? ? ? ? 45 31 F6", [this](Memory addr) {
            m_Pointers.NativeRegistrationTable = addr.Add(3).Rip().As<decltype(m_Pointers.NativeRegistrationTable)>();
        });

        scanner.Add("48 C7 84 C8 D8 00 00 00 00 00 00 00", [this](Memory addr) {
            m_Pointers.ScriptProgramRegistry = addr.Add(0x13).Add(3).Rip().As<decltype(m_Pointers.ScriptProgramRegistry)>();
        });

        scanner.Add("48 8B 05 ? ? ? ? 48 89 34 F8 48 FF C7 48 39 FB 75 97", [this](Memory addr) {
            m_Pointers.ScriptThreadsGEN9 = addr.Add(3).Rip().As<decltype(m_Pointers.ScriptThreadsGEN9)>();
        });

        scanner.Add("49 63 41 1C", [this](Memory addr) {
            m_Pointers.RunScriptThread = addr.Sub(0x24).As<decltype(m_Pointers.RunScriptThread)>();
        });

        scanner.Add("0F B6 0D ? ? ? ? 0F B6 05 ? ? ? ? 08 C8", [this](Memory addr) {
            m_Pointers.TimerUserPause = addr.Add(3).Rip().As<decltype(m_Pointers.TimerUserPause)>();
            m_Pointers.TimerScriptPause = addr.Add(3).Rip().Add(1).As<decltype(m_Pointers.TimerScriptPause)>();
        });

        return scanner.Scan();
    }

    bool GTA5_GEN9::InitHooks() const
    {
        Hooking::AddHook(m_Pointers.RunScriptThread, rage::gta5::scrThreadGEN9::RunThread);

        return Hooking::Init();
    }
}

#endif