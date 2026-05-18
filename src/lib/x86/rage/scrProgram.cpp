#include "scrProgram.hpp"
#include "core/Memory.hpp"

namespace rage
{
    bool scrProgram::Init()
    {
        if (auto addr = Memory::ScanPattern("8B 3D ? ? ? ? 8B CE ? ? ? 85 C0"))
            sm_Programs = *addr->Add(2).As<scrHash<scrProgram*>**>();

        if (auto addr = Memory::ScanPattern("A3 ? ? ? ? 33 DB BF"))
            sm_GlobalsCount = *addr->Add(1).As<int32_t**>();

        if (auto addr = Memory::ScanPattern("8B 15 ? ? ? ? 57 64 8B 3D"))
            sm_Globals = *addr->Add(2).As<scrValue***>();

        if (auto addr = Memory::ScanPattern("83 3D ? ? ? ? 00 A3 ? ? ? ? 8B 44 24"))
            sm_ProtectedGlobals = *addr->Add(2).As<uint8_t***>();

        if (auto addr = Memory::ScanPattern("A1 ? ? ? ? 3D ? ? ? ? 7D ? 40 A3 ? ? ? ? C1 E0"))
            sm_GetNextProtectedSlot = *addr->As<decltype(sm_GetNextProtectedSlot)>();

        if (!sm_Programs || !sm_GlobalsCount || !sm_Globals || !sm_ProtectedGlobals || !sm_GetNextProtectedSlot)
            return false;

        return true;
    }

    scrProgram* scrProgram::GetProgram(uint32_t hash)
    {
        if (!sm_Programs)
            return nullptr;

        return sm_Programs->Lookup(hash);
    }
}