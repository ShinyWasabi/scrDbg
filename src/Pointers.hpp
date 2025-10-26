#pragma once
#include "Pointer.hpp"

namespace scrDbg
{
	struct PointerData
	{
		Pointer GameBuild;
		Pointer OnlineBuild;
		Pointer ScriptThreads;
		Pointer ScriptPrograms;
		Pointer ScriptGlobals;
		Pointer ScriptGlobalBlockCounts;
		Pointer NativeRegistrationTable;
		Pointer TextLabels;
	};

	struct Pointers : PointerData
	{
		bool Init();
	};

	inline Pointers g_Pointers;
}