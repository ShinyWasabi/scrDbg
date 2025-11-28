#include "Pointers.hpp"
#include "core/Memory.hpp"

namespace scrDbgLib
{
    bool Pointers::Init()
    {
        if (g_IsEnhanced)
        {
            if (auto addr = Memory::ScanPattern("4C 8D 0D ? ? ? ? 4C 8D 15 ? ? ? ? 45 31 F6"))
                NativeRegistrationTable = addr->Add(3).Rip().As<decltype(NativeRegistrationTable)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("48 C7 84 C8 D8 00 00 00 00 00 00 00"))
                ScriptPrograms = addr->Add(0x13).Add(3).Rip().Add(0xD8).As<decltype(ScriptPrograms)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("41 57 41 56 41 54 56 57 55 53 48 83 EC 20 49 89 D7 49 89 CC"))
                ScriptProgramConstructor1 = addr->As<decltype(ScriptProgramConstructor1)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("E8 ? ? ? ? 48 89 C3 48 89 C1 48 89 F2 45 89 F8"))
                LoadScriptProgramSCO = addr->Add(1).Rip().As<decltype(LoadScriptProgramSCO)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("41 56 56 57 53 48 83 EC 28 48 89 CE 48 8D 05 ? ? ? ? 48 89 01 48 8D 0D"))
                ScriptProgramDestructor = addr->As<decltype(ScriptProgramDestructor)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("48 8B 05 ? ? ? ? 48 89 34 F8 48 FF C7 48 39 FB 75 97"))
                ScriptThreadsEnhanced = addr->Add(3).Rip().As<decltype(ScriptThreadsEnhanced)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("49 63 41 1C"))
                RunScriptThread = addr->Sub(0x24).As<decltype(RunScriptThread)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("56 48 83 EC 20 80 3D ? ? ? ? ? 75 ? 48 8D 0D"))
                PauseGameNow = addr->As<decltype(PauseGameNow)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("56 57 53 48 83 EC 20 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D"))
                UnpauseGameNow = addr->As<decltype(UnpauseGameNow)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("80 3D ? ? ? ? ? 74 ? 48 83 3D ? ? ? ? ? 74 ? 89 D0"))
                TogglePausedRenderPhases = addr->As<decltype(TogglePausedRenderPhases)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("80 88 ? ? ? ? ? EB ? E8"))
                PauseGameNowPatch = addr->Sub(0x1E).As<decltype(PauseGameNowPatch)>();
            else
                return false;
        }
        else
        {
            if (auto addr = Memory::ScanPattern("48 8D 0D ? ? ? ? 48 8B 14 FA E8 ? ? ? ? 48 85 C0 75 0A"))
                NativeRegistrationTable = addr->Add(3).Rip().As<decltype(NativeRegistrationTable)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("48 8D 0D ? ? ? ? 41 8B D6 E8 ? ? ? ? FF 05"))
                ScriptPrograms = addr->Add(3).Rip().Add(0xD8).As<decltype(ScriptPrograms)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 54 41 56 41 57 48 83 EC 20 48 8B F2 48 8B D9"))
                ScriptProgramConstructor1 = addr->As<decltype(ScriptProgramConstructor1)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("E8 ? ? ? ? 48 81 C4 20 02 00 00 5B"))
                LoadScriptProgramSCO = addr->Add(1).Rip().As<decltype(LoadScriptProgramSCO)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8D 05 ? ? ? ? 48 8B D9 48 8B D1"))
                ScriptProgramDestructor = addr->As<decltype(ScriptProgramDestructor)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("45 33 F6 8B E9 85 C9 B8"))
                ScriptThreadsLegacy = addr->Sub(4).Rip().Sub(8).As<decltype(ScriptThreadsLegacy)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("E8 ? ? ? ? 48 85 FF 48 89 1D"))
                RunScriptThread = addr->Add(1).Rip().As<decltype(RunScriptThread)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("C6 05 ? ? ? ? ? 75 ? E8 ? ? ? ? C6 05"))
                PauseGameNow = addr->Sub(0x43).As<decltype(PauseGameNow)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("E8 ? ? ? ? C6 05 ? ? ? ? ? 41 B0 01"))
                UnpauseGameNow = addr->Add(1).Rip().As<decltype(UnpauseGameNow)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("48 83 EC 28 45 33 D2 44 8A C1"))
                TogglePausedRenderPhases = addr->As<decltype(TogglePausedRenderPhases)>();
            else
                return false;

            if (auto addr = Memory::ScanPattern("75 ? E8 ? ? ? ? C6 05 ? ? ? ? ? EB ? 48 8B 05"))
                PauseGameNowPatch = addr->As<decltype(PauseGameNowPatch)>();
            else
                return false;
        }

        return true;
    }
}