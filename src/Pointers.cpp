#include "Pointers.hpp"
#include "Pattern.hpp"

namespace scrDbg
{
	bool Pointers::Init()
	{
		Scanner scanner;

		if (g_IsEnhanced)
		{
			scanner.Add("GameBuild&OnlineBuild", "4C 8D 0D ? ? ? ? 48 8D 5C 24 ? 48 89 D9 48 89 FA", [this](Pointer ptr) {
				GameBuild = ptr.Add(3).Rip();
				OnlineBuild = ptr.Add(0x47).Add(3).Rip();
			});

			scanner.Add("ScriptThreads", "48 8B 05 ? ? ? ? 48 89 34 F8 48 FF C7 48 39 FB 75 97", [this](Pointer ptr) {
				ScriptThreads = ptr.Add(3).Rip();
			});

			scanner.Add("ScriptPrograms", "48 C7 84 C8 D8 00 00 00 00 00 00 00", [this](Pointer ptr) {
				ScriptPrograms = ptr.Add(0x13).Add(3).Rip().Add(0xD8);
			});

			scanner.Add("ScriptGlobals", "48 8B 8E B8 00 00 00 48 8D 15 ? ? ? ? 49 89 D8", [this](Pointer ptr) {
				ScriptGlobals = ptr.Add(7).Add(3).Rip();
			});

			scanner.Add("ScriptGlobalBlockCounts", "48 8D 05 ? ? ? ? 42 89 0C B8", [this](Pointer ptr) {
				ScriptGlobalBlockCounts = ptr.Add(3).Rip();
			});

			scanner.Add("NativeRegistrationTable", "4C 8D 0D ? ? ? ? 4C 8D 15 ? ? ? ? 45 31 F6", [this](Pointer ptr) {
				NativeRegistrationTable = ptr.Add(3).Rip();
			});

			scanner.Add("TextLabels", "48 8D 0D ? ? ? ? 0F 2E 35", [this](Pointer ptr) {
				TextLabels = ptr.Add(3).Rip().Add(8);
			});
		}
		else
		{
			scanner.Add("GameBuild&OnlineBuild", "8B C3 33 D2 C6 44 24 20", [this](Pointer ptr) {
				GameBuild = ptr.Add(0x24).Rip();
				OnlineBuild = ptr.Add(0x24).Rip().Add(0x20);
			});

			scanner.Add("ScriptThreads", "45 33 F6 8B E9 85 C9 B8", [this](Pointer ptr) {
				ScriptThreads = ptr.Sub(4).Rip().Sub(8);
			});

			scanner.Add("ScriptPrograms", "48 8D 0D ? ? ? ? 41 8B D6 E8 ? ? ? ? FF 05", [this](Pointer ptr) {
				ScriptPrograms = ptr.Add(3).Rip().Add(0xD8);
			});

			scanner.Add("ScriptGlobals", "48 8D 15 ? ? ? ? 4C 8B C0 E8 ? ? ? ? 48 85 FF 48 89 1D", [this](Pointer ptr) {
				ScriptGlobals = ptr.Add(3).Rip();
			});

			scanner.Add("ScriptGlobalBlockCounts", "41 89 9C BC", [this](Pointer ptr) {
				ScriptGlobalBlockCounts = Process::GetBaseAddress() + ptr.Add(4).Read<int>(); // base + RVA
			});

			scanner.Add("NativeRegistrationTable", "48 8D 0D ? ? ? ? 48 8B 14 FA E8 ? ? ? ? 48 85 C0 75 0A", [this](Pointer ptr) {
				NativeRegistrationTable = ptr.Add(3).Rip();
			});

			scanner.Add("TextLabels", "48 8D 0D ? ? ? ? 22 D8", [this](Pointer ptr) {
				TextLabels = ptr.Add(3).Rip().Add(8);
			});
		}

		return scanner.Scan();
	}
}