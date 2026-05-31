#include "GTA4.hpp"
#include "core/Hooking.hpp"
#include "core/Scanner.hpp"
#include "debugger/DebuggerGTA4.hpp"
#include "rage/gta4/scrThread.hpp"

#if defined(_M_IX86)

namespace scrDbgLib
{
    GTA4::GTA4()
    {
        m_Debugger = std::make_unique<DebuggerGTA4>();
    }

    bool GTA4::InitPointers() const
    {
        Scanner scanner;

        scanner.Add("8B 3D ? ? ? ? 8B CE ? ? ? 85 C0", [this](Memory addr) {
            m_Pointers.ScriptPrograms = *addr.Add(2).As<decltype(m_Pointers.ScriptPrograms)*>();
        });

        scanner.Add("A3 ? ? ? ? 33 DB BF", [this](Memory addr) {
            m_Pointers.ScriptGlobalsCount = *addr.Add(1).As<decltype(m_Pointers.ScriptGlobalsCount)*>();
        });

        scanner.Add("8B 15 ? ? ? ? 57 64 8B 3D", [this](Memory addr) {
            m_Pointers.ScriptGlobals = *addr.Add(2).As<decltype(m_Pointers.ScriptGlobals)*>();
        });

        scanner.Add("83 3D ? ? ? ? 00 A3 ? ? ? ? 8B 44 24", [this](Memory addr) {
            m_Pointers.ProtectedScriptGlobals = *addr.Add(2).As<decltype(m_Pointers.ProtectedScriptGlobals)*>();
        });

        scanner.Add("A1 ? ? ? ? 3D ? ? ? ? 7D ? 40 A3 ? ? ? ? C1 E0", [this](Memory addr) {
            m_Pointers.GetNextProtectedScriptSlot = addr.As<decltype(m_Pointers.GetNextProtectedScriptSlot)>();
        });

        scanner.Add("A1 ? ? ? ? 53 55 8B E9 56 57", [this](Memory addr) {
            m_Pointers.ScriptProgramCtor = addr.As<decltype(m_Pointers.ScriptProgramCtor)>();
        });

        scanner.Add("56 8B F1 57 FF 76 ? E8 ? ? ? ? 8B 56", [this](Memory addr) {
            m_Pointers.ScriptProgramDtor = addr.As<decltype(m_Pointers.ScriptProgramDtor)>();
        });

        scanner.Add("39 35 ? ? ? ? 75 ? C7 46", [this](Memory addr) {
            m_Pointers.CurrentScriptThread = *addr.Add(2).As<decltype(m_Pointers.CurrentScriptThread)*>();
        });

        scanner.Add("8B 15 ? ? ? ? 0F B7 C7", [this](Memory addr) {
            m_Pointers.ScriptThreads = *addr.Add(2).As<decltype(m_Pointers.ScriptThreads)*>();
        });

        scanner.Add("C7 05 ? ? ? ? 00 00 00 00 66 3B C2", [this](Memory addr) {
            m_Pointers.CommandHandlers = *addr.Add(2).As<decltype(m_Pointers.CommandHandlers)*>();
        });

        scanner.Add("55 8B EC 83 E4 ? 81 EC ? ? ? ? 56 57 8B F9 89 7C 24 ? 8B 47 ? 83 F8", [this](Memory addr) {
            m_Pointers.RunScriptThread = addr.As<decltype(m_Pointers.RunScriptThread)>();
        });

        scanner.Add("8A 0D ? ? ? ? 0A C1", [this](Memory addr) {
            m_Pointers.TimerUserPause = *addr.Add(2).As<decltype(m_Pointers.TimerUserPause)*>();
            m_Pointers.TimerScriptPause = *addr.Add(8).Add(2).As<decltype(m_Pointers.TimerScriptPause)*>();
        });

        return scanner.Scan();
    }

    bool GTA4::InitHooks() const
    {
        Hooking::AddHook(m_Pointers.RunScriptThread, rage::scrThread::RunThread);

        return Hooking::Init();
    }
}

#endif