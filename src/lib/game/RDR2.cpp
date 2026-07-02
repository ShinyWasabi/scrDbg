#include "RDR2.hpp"
#include "core/Hooking.hpp"
#include "core/Scanner.hpp"
#include "debugger/DebuggerRDR2.hpp"
#include "rage/rdr2/scrThread.hpp"

#if defined(_M_X64)

namespace scrDbgLib
{
    RDR2::RDR2()
    {
        m_Debugger = std::make_unique<DebuggerRDR2>();
    }

    bool RDR2::InitPointers() const
    {
        Scanner scanner;

        scanner.Add("4C 8B 1D ? ? ? ? 41 8B CA", [this](Memory addr) {
            m_Pointers.ScriptPrograms = addr.Add(3).Rip().As<decltype(m_Pointers.ScriptPrograms)>();
        });

        scanner.Add("48 8B 05 ? ? ? ? 45 33 C9 48 89 85", [this](Memory addr) {
            m_Pointers.ScriptGlobals = addr.Add(3).Rip().As<decltype(m_Pointers.ScriptGlobals)>();
        });

        scanner.Add("48 89 0D ? ? ? ? ? ? ? 0F 18 57", [this](Memory addr) {
            m_Pointers.CurrentScriptThreadName = addr.Add(3).Rip().As<decltype(m_Pointers.CurrentScriptThreadName)>();
            m_Pointers.CurrentScriptThread = addr.Add(0x1E).Add(3).Rip().As<decltype(m_Pointers.CurrentScriptThread)>();
        });

        scanner.Add("48 8B 0D ? ? ? ? FF C0", [this](Memory addr) {
            m_Pointers.ScriptThreads = addr.Add(3).Rip().As<decltype(m_Pointers.ScriptThreads)>();
        });

        scanner.Add("4C 8B 1D ? ? ? ? 41 8B C1", [this](Memory addr) {
            m_Pointers.CommandHandlers = addr.Add(3).Rip().As<decltype(m_Pointers.CommandHandlers)>();
        });

        scanner.Add("89 54 24 ? 55 53 57 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 8B 79", [this](Memory addr) {
            m_Pointers.RunScriptThread = addr.As<decltype(m_Pointers.RunScriptThread)>();
        });

        /*
        scanner.Add("8A 0D ? ? ? ? 0A C1", [this](Memory addr) {
            m_Pointers.TimerUserPause = *addr.Add(2).As<decltype(m_Pointers.TimerUserPause)*>();
            m_Pointers.TimerScriptPause = *addr.Add(8).Add(2).As<decltype(m_Pointers.TimerScriptPause)*>();
        });
        */

        return scanner.Scan();
    }

    bool RDR2::InitHooks() const
    {
        Hooking::AddHook(m_Pointers.RunScriptThread, rage::rdr2::scrThread::RunThread);

        return Hooking::Init();
    }
}

#endif