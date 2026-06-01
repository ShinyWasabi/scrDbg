#include "GTA5.hpp"
#include "ResourceLoader.hpp"
#include "core/Scanner.hpp"
#include "disasm/DisassemblerGTA5.hpp"
#include "types/ScriptProgramGTA5.hpp"
#include "types/ScriptThreadGTA5_GEN8.hpp"
#include "types/ScriptThreadGTA5_GEN9.hpp"

namespace scrDbgApp
{
    static std::string SearchTextLabelSlot(uint32_t hash, uintptr_t slot)
    {
        struct GXT2Entry
        {
            uint32_t KeyHash;
            uint32_t KeyOffset;
        };

        struct GXT2Header
        {
            uint32_t Magic; // 2TXG
            uint32_t EntryCount;
        };

        auto header = Pointer(slot).Get<GXT2Header>();

        std::vector<GXT2Entry> entries(header.EntryCount);
        Pointer(slot + sizeof(GXT2Header)).GetBuffer(entries.data(), header.EntryCount * sizeof(GXT2Entry));

        auto it = std::lower_bound(entries.begin(), entries.end(), hash, [](GXT2Entry& entry, uint32_t keyHash) {
            return entry.KeyHash < keyHash;
        });

        if (it != entries.end() && it->KeyHash == hash)
            return Pointer(slot + it->KeyOffset).GetString(4096);

        return {};
    }

    std::unique_ptr<Disassembler> GTA5::CreateDisassembly(std::unique_ptr<ScriptProgram> program) const
    {
        auto disasm = std::make_unique<DisassemblerGTA5>(std::move(program));
        disasm->Refresh();
        return disasm;
    }

    bool GTA5_GEN9::Init()
    {
        Scanner scanner;

        scanner.Add("GameBuild&OnlineBuild", "4C 8D 0D ? ? ? ? 48 8D 5C 24 ? 48 89 D9 48 89 FA", [this](Pointer ptr) {
            m_Pointers.GameBuild = ptr.Add(3).Rip();
            m_Pointers.OnlineBuild = ptr.Add(0x47).Add(3).Rip();
        });

        scanner.Add("ScriptThreads", "48 8B 05 ? ? ? ? 48 89 34 F8 48 FF C7 48 39 FB 75 97", [this](Pointer ptr) {
            m_Pointers.ScriptThreads = ptr.Add(3).Rip();
        });

        scanner.Add("ScriptPrograms", "44 88 05 ? ? ? ? 48 89 F9", [this](Pointer ptr) {
            m_Pointers.ScriptPrograms = ptr.Add(3).Rip().Add(0xD8);
        });

        scanner.Add("ScriptGlobals", "48 8B 8E B8 00 00 00 48 8D 15 ? ? ? ? 49 89 D8", [this](Pointer ptr) {
            m_Pointers.ScriptGlobals = ptr.Add(7).Add(3).Rip();
        });

        scanner.Add("ScriptGlobalBlockCounts", "48 8D 05 ? ? ? ? 42 89 0C B8", [this](Pointer ptr) {
            m_Pointers.ScriptGlobalBlockCounts = ptr.Add(3).Rip();
        });

        scanner.Add("NativeRegistrationTable", "4C 8D 0D ? ? ? ? 4C 8D 15 ? ? ? ? 45 31 F6", [this](Pointer ptr) {
            m_Pointers.NativeRegistrationTable = ptr.Add(3).Rip();
        });

        scanner.Add("TextLabels", "48 8D 0D ? ? ? ? 0F 2E 35", [this](Pointer ptr) {
            m_Pointers.TextLabels = ptr.Add(3).Rip().Add(8);
        });

        return scanner.Scan();
    }

    bool GTA5_GEN8::Init()
    {
        Scanner scanner;

        scanner.Add("GameBuild&OnlineBuild", "8B C3 33 D2 C6 44 24 20", [this](Pointer ptr) {
            m_Pointers.GameBuild = ptr.Add(0x24).Rip();
            m_Pointers.OnlineBuild = ptr.Add(0x24).Rip().Add(0x20);
        });

        scanner.Add("ScriptThreads", "45 33 F6 8B E9 85 C9 B8", [this](Pointer ptr) {
            m_Pointers.ScriptThreads = ptr.Sub(4).Rip().Sub(8);
        });

        scanner.Add("ScriptPrograms", "48 8D 0D ? ? ? ? 41 8B D6 E8 ? ? ? ? FF 05", [this](Pointer ptr) {
            m_Pointers.ScriptPrograms = ptr.Add(3).Rip().Add(0xD8);
        });

        scanner.Add("ScriptGlobals", "48 8D 15 ? ? ? ? 4C 8B C0 E8 ? ? ? ? 48 85 FF 48 89 1D", [this](Pointer ptr) {
            m_Pointers.ScriptGlobals = ptr.Add(3).Rip();
        });

        scanner.Add("ScriptGlobalBlockCounts", "41 89 9C BC", [this](Pointer ptr) {
            m_Pointers.ScriptGlobalBlockCounts = Process::GetBaseAddress() + ptr.Add(4).Get<int32_t>(); // base + RVA
        });

        scanner.Add("NativeRegistrationTable", "48 8D 0D ? ? ? ? 48 8B 14 FA E8 ? ? ? ? 48 85 C0 75 0A", [this](Pointer ptr) {
            m_Pointers.NativeRegistrationTable = ptr.Add(3).Rip();
        });

        scanner.Add("TextLabels", "48 8D 0D ? ? ? ? 22 D8", [this](Pointer ptr) {
            m_Pointers.TextLabels = ptr.Add(3).Rip().Add(8);
        });

        return scanner.Scan();
    }

    std::string GTA5::GetGameBuild() const
    {
        char gameBuild[64]{};
        Process::ReadRaw(m_Pointers.GameBuild, gameBuild, sizeof(gameBuild));

        char onlineBuild[64]{};
        Process::ReadRaw(m_Pointers.OnlineBuild, onlineBuild, sizeof(onlineBuild));

        char result[128]{};
        std::snprintf(result, sizeof(result), "%s-%s", gameBuild, onlineBuild);
        return result;
    }

    int32_t GTA5::GetGlobal(uint32_t index) const
    {
        uintptr_t block = m_Pointers.ScriptGlobals.GetArray<uintptr_t>((index >> 0x12) & 0x3F);
        return static_cast<int32_t>(Pointer(block).GetArray<int64_t>(index & 0x3FFFF));
    }

    void GTA5::SetGlobal(uint32_t index, int32_t value) const
    {
        uintptr_t block = m_Pointers.ScriptGlobals.GetArray<uintptr_t>((index >> 0x12) & 0x3F);
        Pointer(block).SetArray<int64_t>(index & 0x3FFFF, static_cast<int32_t>(value));
    }

    uint32_t GTA5::GetGlobalBlockCount(uint32_t block) const
    {
        return m_Pointers.ScriptGlobalBlockCounts.GetArray<uint32_t>(block);
    }

    std::unique_ptr<ScriptProgram> GTA5::GetProgram(uint32_t hash) const
    {
        if (hash == 0)
            return nullptr;

        for (uint32_t i = 0; i < 176; i++)
        {
            Pointer addr = m_Pointers.ScriptPrograms.GetArray<uintptr_t>(i);

            ScriptProgramGTA5 program(addr);
            if (program.GetNameHash() == hash)
                return std::make_unique<ScriptProgramGTA5>(addr);
        }

        return nullptr;
    }

    std::vector<std::unique_ptr<ScriptThread>> GTA5_GEN9::GetThreads() const
    {
        std::vector<std::unique_ptr<ScriptThread>> threads;

        Pointer base = m_Pointers.ScriptThreads.Deref();
        uint16_t count = m_Pointers.ScriptThreads.Add(10).Get<uint16_t>();

        for (uint16_t i = 0; i < count; i++)
        {
            Pointer addr = base.GetArray<uintptr_t>(i);
            threads.push_back(std::make_unique<ScriptThreadGTA5_GEN9>(addr));
        }

        return threads;
    }

    std::unique_ptr<ScriptThread> GTA5_GEN9::GetThread(uint32_t hash) const
    {
        if (hash == 0)
            return nullptr;

        Pointer base = m_Pointers.ScriptThreads.Deref();
        uint16_t count = m_Pointers.ScriptThreads.Add(10).Get<uint16_t>();

        for (uint16_t i = 0; i < count; i++)
        {
            Pointer addr = base.GetArray<uintptr_t>(i);

            ScriptThreadGTA5_GEN9 thread(addr);
            if (thread.GetScriptHash() == hash)
                return std::make_unique<ScriptThreadGTA5_GEN9>(addr);
        }

        return nullptr;
    }

    std::vector<std::unique_ptr<ScriptThread>> GTA5_GEN8::GetThreads() const
    {
        std::vector<std::unique_ptr<ScriptThread>> threads;

        Pointer base = m_Pointers.ScriptThreads.Deref();
        uint16_t count = m_Pointers.ScriptThreads.Add(10).Get<uint16_t>();

        for (uint16_t i = 0; i < count; i++)
        {
            Pointer addr = base.GetArray<uintptr_t>(i);
            threads.push_back(std::make_unique<ScriptThreadGTA5_GEN8>(addr));
        }

        return threads;
    }

    std::unique_ptr<ScriptThread> GTA5_GEN8::GetThread(uint32_t hash) const
    {
        if (hash == 0)
            return nullptr;

        Pointer base = m_Pointers.ScriptThreads.Deref();
        uint16_t count = m_Pointers.ScriptThreads.Add(10).Get<uint16_t>();

        for (uint16_t i = 0; i < count; i++)
        {
            Pointer addr = base.GetArray<uintptr_t>(i);

            ScriptThreadGTA5_GEN8 thread(addr);
            if (thread.GetScriptHash() == hash)
                return std::make_unique<ScriptThreadGTA5_GEN8>(addr);
        }

        return nullptr;
    }

    std::string_view GTA5::GetNativeNameByHash(uint64_t hash) const
    {
        return scrDbgShared::NativesBin::GetNameByHash(hash);
    }

    // TO-DO: Some natives (especially nullsubs) share the same handler.
    // This makes it impossible to determine which native was actually called.
    // Change this function to return all hashes associated with the handler, rather than stopping at the first match. This applies to the code in scrDbgLib as well.
    uint64_t GTA5::GetNativeHashByHandler(uintptr_t handler) const
    {
        if (!m_Pointers.NativeRegistrationTable || !handler)
            return 0;

        uint8_t buffer[0xC8]{}; // Enough to read it all I guess (scrNativeRegistration + hashes)
        for (int bucket = 0; bucket < 256; ++bucket)
        {
            uintptr_t registration = m_Pointers.NativeRegistrationTable.GetArray<uintptr_t>(bucket);

            while (registration)
            {
                if (!Pointer(registration).GetBuffer(buffer, sizeof(buffer)))
                    break;

                uint32_t numEntries1 = *reinterpret_cast<uint32_t*>(buffer + 0x48);
                uint32_t numEntries2 = *reinterpret_cast<uint32_t*>(buffer + 0x4C);
                uint32_t numEntries = numEntries1 ^ static_cast<uint32_t>(registration + 0x48) ^ numEntries2;

                if (numEntries > 7)
                    numEntries = 7;

                auto handlers = reinterpret_cast<uintptr_t*>(buffer + 0x10);
                for (uint32_t j = 0; j < numEntries; ++j)
                {
                    if (handlers[j] == handler)
                    {
                        uintptr_t addr = registration + 0x54 + 16ULL * j;

                        uint32_t key = static_cast<uint32_t>(addr) ^ Pointer(addr).Add(8).Get<uint32_t>();
                        uint32_t low = key ^ Pointer(addr).Add(0).Get<uint32_t>();
                        uint32_t high = key ^ Pointer(addr).Add(4).Get<uint32_t>();
                        return (static_cast<uint64_t>(high) << 32) | low;
                    }
                }

                uint32_t key = static_cast<uint32_t>(registration) ^ *reinterpret_cast<uint32_t*>(buffer + 8);
                uint32_t low = key ^ *reinterpret_cast<uint32_t*>(buffer + 0);
                uint32_t high = key ^ *reinterpret_cast<uint32_t*>(buffer + 4);
                registration = (static_cast<uint64_t>(high) << 32) | low;
            }
        }

        return 0;
    }

    std::unordered_map<uint64_t, uintptr_t> GTA5::GetAllNatives() const
    {
        std::unordered_map<uint64_t, uintptr_t> result;

        if (!m_Pointers.NativeRegistrationTable)
            return result;

        uint8_t buffer[0xC8]{};

        for (int bucket = 0; bucket < 256; ++bucket)
        {
            uintptr_t registration = m_Pointers.NativeRegistrationTable.GetArray<uintptr_t>(bucket);

            while (registration)
            {
                if (!Pointer(registration).GetBuffer(buffer, sizeof(buffer)))
                    break;

                uint32_t numEntries1 = *reinterpret_cast<uint32_t*>(buffer + 0x48);
                uint32_t numEntries2 = *reinterpret_cast<uint32_t*>(buffer + 0x4C);
                uint32_t numEntries = numEntries1 ^ static_cast<uint32_t>(registration + 0x48) ^ numEntries2;

                if (numEntries > 7)
                    numEntries = 7;

                auto handlers = reinterpret_cast<uintptr_t*>(buffer + 0x10);
                for (uint32_t j = 0; j < numEntries; ++j)
                {
                    uintptr_t handler = handlers[j];
                    uintptr_t addr = registration + 0x54 + 16ULL * j;

                    uint32_t key = static_cast<uint32_t>(addr) ^ Pointer(addr).Add(8).Get<uint32_t>();
                    uint32_t low = key ^ Pointer(addr).Add(0).Get<uint32_t>();
                    uint32_t high = key ^ Pointer(addr).Add(4).Get<uint32_t>();
                    uint64_t hash = (static_cast<uint64_t>(high) << 32) | low;

                    result[hash] = handler;
                }

                uint32_t key = static_cast<uint32_t>(registration) ^ *reinterpret_cast<uint32_t*>(buffer + 8);
                uint32_t low = key ^ *reinterpret_cast<uint32_t*>(buffer + 0);
                uint32_t high = key ^ *reinterpret_cast<uint32_t*>(buffer + 4);
                registration = (static_cast<uint64_t>(high) << 32) | low;
            }
        }

        return result;
    }

    std::string GTA5::GetTextLabel(uint32_t hash) const
    {
        for (int i = 0; i < 23; i++)
        {
            if (auto slot = m_Pointers.TextLabels.GetArray<uintptr_t>(i))
            {
                if (auto label = SearchTextLabelSlot(hash, slot); !label.empty())
                    return label;
            }
        }

        return {};
    }
}