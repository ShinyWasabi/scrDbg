#include "Payne.hpp"
#include "core/Hooking.hpp"
#include "core/Scanner.hpp"
#include "debugger/DebuggerPayne.hpp"
#include "rage/payne/scrThread.hpp"

#if defined(_M_IX86)

namespace scrDbgLib
{
    Payne::Payne()
    {
        m_Debugger = std::make_unique<DebuggerPayne>();
    }

    bool Payne::InitPointers() const
    {
        Scanner scanner;

        scanner.Add("B9 ? ? ? ? E8 ? ? ? ? 8B 4E ? 51 E8", [this](Memory addr) {
            m_Pointers.ScriptPrograms = *addr.Add(1).As<decltype(m_Pointers.ScriptPrograms)*>();
        });

        scanner.Add("2B 3D ? ? ? ? 8B F1", [this](Memory addr) {
            m_Pointers.ScriptGlobals = *addr.Add(2).As<decltype(m_Pointers.ScriptGlobals)*>();
        });

        scanner.Add("89 35 ? ? ? ? 8D 44 24", [this](Memory addr) {
            m_Pointers.CurrentScriptThread = *addr.Add(2).As<decltype(m_Pointers.CurrentScriptThread)*>();
        });

        scanner.Add("A3 ? ? ? ? 8B C7 5F", [this](Memory addr) {
            m_Pointers.CurrentScriptThreadName = *addr.Add(1).As<decltype(m_Pointers.CurrentScriptThreadName)*>();
        });

        scanner.Add("8B 35 ? ? ? ? 0F B7 0D", [this](Memory addr) {
            m_Pointers.ScriptThreads = *addr.Add(2).As<decltype(m_Pointers.ScriptThreads)*>();
        });

        scanner.Add("89 1D ? ? ? ? 66 3B 0D", [this](Memory addr) {
            m_Pointers.Commands = *addr.Add(2).As<decltype(m_Pointers.Commands)*>();
        });

        scanner.Add("55 8B EC 83 E4 ? 81 EC ? ? ? ? 53 56 57 8B F9 8B 47 ? 83 F8", [this](Memory addr) {
            m_Pointers.RunScriptThread = addr.As<decltype(m_Pointers.RunScriptThread)>();
        });

        scanner.Add("A0 ? ? ? ? 8A C8 0A 0D", [this](Memory addr) {
            m_Pointers.TimerUserPause = *addr.Add(1).As<decltype(m_Pointers.TimerUserPause)*>();
        });

        return scanner.Scan();
    }

    bool Payne::InitHooks() const
    {
        Hooking::AddHook(m_Pointers.RunScriptThread, rage::payne::scrThread::RunThread);

        return Hooking::Init();
    }
}

#endif