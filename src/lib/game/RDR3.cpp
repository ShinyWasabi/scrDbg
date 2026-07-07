#include "RDR3.hpp"
#include "core/Hooking.hpp"
#include "core/Scanner.hpp"
#include "debugger/DebuggerRDR3.hpp"

#if defined(_M_X64)

namespace scrDbgLib
{
    RDR3::RDR3()
    {
        m_Debugger = std::make_unique<DebuggerRDR3>();
    }

    bool RDR3::InitPointers() const
    {
        Scanner scanner;

        scanner.Add("4C 8B 05 ? ? ? ? 4C 8B C9 49 F7 D1", [this](Memory addr) {
            m_Pointers.NativeRegistrationSeed = addr.Add(3).Rip().As<decltype(m_Pointers.NativeRegistrationSeed)>();
            m_Pointers.NativeRegistrationTable = addr.Add(0x24).Add(3).Rip().As<decltype(m_Pointers.NativeRegistrationTable)>();
        });

        scanner.Add("48 8D 0D ? ? ? ? E8 ? ? ? ? 8B 48 ? 89 8B", [this](Memory addr) {
            m_Pointers.ScriptProgramRegistry = addr.Add(3).Rip().As<decltype(m_Pointers.ScriptProgramRegistry)>();
        });

        scanner.Add("48 8B 05 ? ? ? ? FF C2", [this](Memory addr) {
            m_Pointers.ScriptThreads = addr.Add(3).Rip().As<decltype(m_Pointers.ScriptThreads)>();
        });

        scanner.Add("E8 ? ? ? ? 65 48 8B 0C 25 ? ? ? ? 48 85 F6", [this](Memory addr) {
            m_Pointers.RunScriptThread = addr.Add(1).Rip().As<decltype(m_Pointers.RunScriptThread)>();
        });

        scanner.Add("E8 ? ? ? ? 8B 43 ? 48 8B 4C 24", [this](Memory addr) {
            m_Pointers.ResolveSecureScriptGlobal = addr.Add(1).Rip().As<decltype(m_Pointers.ResolveSecureScriptGlobal)>();
        });

        scanner.Add("E8 ? ? ? ? EB ? 48 8D 54 24 ? E8 ? ? ? ? 8B 43", [this](Memory addr) {
            m_Pointers.ResolveSecureScriptFrame = addr.Add(1).Rip().As<decltype(m_Pointers.ResolveSecureScriptFrame)>();
        });

        scanner.Add("E8 ? ? ? ? EB ? E8 ? ? ? ? EB ? 48 8D 54 24 ? E8 ? ? ? ? 8B 43", [this](Memory addr) {
            m_Pointers.ResolveSecureScriptStatic = addr.Add(1).Rip().As<decltype(m_Pointers.ResolveSecureScriptStatic)>();
        });

        scanner.Add("E8 ? ? ? ? 44 8B 45 ? 48 8B 0D", [this](Memory addr) {
            m_Pointers.DecryptSecureScriptValue = addr.Add(1).Rip().As<decltype(m_Pointers.DecryptSecureScriptValue)>();
        });

        scanner.Add("E8 ? ? ? ? 48 83 EB ? 4C 8B 5C 24", [this](Memory addr) {
            m_Pointers.SetSecureScriptValue = addr.Add(1).Rip().As<decltype(m_Pointers.SetSecureScriptValue)>();
        });

        scanner.Add("E8 ? ? ? ? 83 63 ? 00 83 63 ? 00 8B 53", [this](Memory addr) {
            m_Pointers.ResetSecureScriptFrame = addr.Add(1).Rip().As<decltype(m_Pointers.ResetSecureScriptFrame)>();
        });

        scanner.Add("40 88 3D ? ? ? ? 40 38 3D ? ? ? ? 74 ? 40 88 3D", [this](Memory addr) {
            m_Pointers.TimerUserPause = addr.Add(3).Rip().As<decltype(m_Pointers.TimerUserPause)>();
            m_Pointers.TimerScriptPause = addr.Add(3).Rip().Add(1).As<decltype(m_Pointers.TimerScriptPause)>();
        });

        return scanner.Scan();
    }

    bool RDR3::InitHooks() const
    {
        Hooking::AddHook(m_Pointers.RunScriptThread, rage::rdr3::scrThread::RunThread);

        return Hooking::Init();
    }
}

#endif