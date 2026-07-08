#include "RDR3.hpp"
#include "ResourceLoader.hpp"
#include "core/Scanner.hpp"
#include "disasm/DisassemblerRDR3.hpp"
#include "types/ScriptProgramRDR3.hpp"
#include "types/ScriptThreadRDR3.hpp"

namespace scrDbgApp
{
    static std::string SearchTextLabelSlot(Pointer slot, uint32_t hash)
    {
        uint32_t bucketCount = slot.Add(0x18).Get<uint32_t>();
        if (bucketCount == 0)
            return {};

        Pointer bucketArray = slot.Add(0x10).Get<Pointer>();
        Pointer node = bucketArray.GetArray<Pointer>(hash % bucketCount);

        while (node && node.Get<uint32_t>() != hash)
            node = node.Add(0x10).Get<Pointer>();

        if (!node)
            return {};

        Pointer entry = node.Add(0x08).Get<Pointer>();
        if (!entry)
            return {};

        Pointer str = entry.Add(0x10).Get<Pointer>();
        if (!str)
            return {};

        return str.GetString();
    }

    bool RDR3::Init()
    {
        Scanner scanner;

        scanner.Add("GameBuild", "88 05 ? ? ? ? 88 05 ? ? ? ? E9", [this](Pointer ptr) {
            m_Pointers.GameBuild = ptr.Add(2).Rip();
        });

        scanner.Add("ScriptThreads", "48 8B 05 ? ? ? ? FF C2", [this](Pointer ptr) {
            m_Pointers.ScriptThreads = ptr.Add(3).Rip();
        });

        scanner.Add("ScriptPrograms", "48 8D 0D ? ? ? ? E8 ? ? ? ? 8B 48 ? 89 8B", [this](Pointer ptr) {
            m_Pointers.ScriptPrograms = ptr.Add(3).Rip().Add(0xD8);
        });

        scanner.Add("ScriptGlobals", "49 83 A4 FE ? ? ? ? 00 49 8D 8E", [this](Pointer ptr) {
            m_Pointers.ScriptGlobals = Process::GetBaseAddress() + ptr.Add(4).Get<int32_t>();
            m_Pointers.ScriptGlobalBlockCounts = Process::GetBaseAddress() + ptr.Add(0x10).Add(4).Get<int32_t>();
        });

        scanner.Add("NativeRegistrationTable", "4C 8B 05 ? ? ? ? 4C 8B C9 49 F7 D1", [this](Pointer ptr) {
            m_Pointers.NativeRegistrationSeed = ptr.Add(3).Rip();
            m_Pointers.NativeRegistrationTable = ptr.Add(0x24).Add(3).Rip();
        });

        scanner.Add("TextLabels", "48 8D 0D ? ? ? ? 44 0F 45 E8", [this](Pointer ptr) {
            m_Pointers.TextLabels = ptr.Add(3).Rip();
        });

        return scanner.Scan();
    }

    std::unique_ptr<Disassembler> RDR3::CreateDisassembly(std::unique_ptr<ScriptProgram> program) const
    {
        auto disasm = std::make_unique<DisassemblerRDR3>(std::move(program));
        disasm->Refresh();
        return disasm;
    }

    std::string RDR3::GetGameBuild() const
    {
        char gameBuild[64]{};
        m_Pointers.GameBuild.GetBuffer(gameBuild, sizeof(gameBuild));
        return std::string(gameBuild);
    }

    Pointer RDR3::GetGlobal(uint32_t index) const
    {
        uintptr_t block = m_Pointers.ScriptGlobals.GetArray<uintptr_t>((index >> 0x12) & 0x3F);
        return Pointer(block).Add((index & 0x3FFFF) * sizeof(uint64_t));
    }

    uint32_t RDR3::GetGlobalBlockCount(uint32_t block) const
    {
        return m_Pointers.ScriptGlobalBlockCounts.GetArray<uint32_t>(block);
    }

    std::unique_ptr<ScriptProgram> RDR3::GetProgram(uint32_t hash) const
    {
        if (hash == 0)
            return nullptr;

        for (uint32_t i = 0; i < 160; i++)
        {
            Pointer addr = m_Pointers.ScriptPrograms.GetArray<uintptr_t>(i);

            ScriptProgramRDR3 program(addr);
            if (program.GetNameHash() == hash)
                return std::make_unique<ScriptProgramRDR3>(addr);
        }

        return nullptr;
    }

    std::vector<std::unique_ptr<ScriptThread>> RDR3::GetThreads() const
    {
        std::vector<std::unique_ptr<ScriptThread>> threads;

        Pointer base = m_Pointers.ScriptThreads.Deref();
        uint16_t count = m_Pointers.ScriptThreads.Add(10).Get<uint16_t>();

        for (uint16_t i = 0; i < count; i++)
        {
            Pointer addr = base.GetArray<uintptr_t>(i);
            threads.push_back(std::make_unique<ScriptThreadRDR3>(addr));
        }

        return threads;
    }

    std::unique_ptr<ScriptThread> RDR3::GetThread(uint32_t hash) const
    {
        if (hash == 0)
            return nullptr;

        Pointer base = m_Pointers.ScriptThreads.Deref();
        uint16_t count = m_Pointers.ScriptThreads.Add(10).Get<uint16_t>();

        for (uint16_t i = 0; i < count; i++)
        {
            Pointer addr = base.GetArray<uintptr_t>(i);

            ScriptThreadRDR3 thread(addr);
            if (thread.GetScriptHash() == hash)
                return std::make_unique<ScriptThreadRDR3>(addr);
        }

        return nullptr;
    }

    std::string_view RDR3::GetNativeNameByHash(uint64_t hash) const
    {
        return scrDbgShared::NativesBin::GetNameByHash(hash);
    }

    uint64_t RDR3::GetNativeHashByHandler(uintptr_t handler) const
    {
        uint64_t seed = m_Pointers.NativeRegistrationSeed.Get<uint64_t>();

        for (uint64_t bucket = 0; bucket < 256; bucket++)
        {
            Pointer node(m_Pointers.NativeRegistrationTable.GetArray<uintptr_t>(bucket));

            while (node)
            {
                uint64_t count = node.Add(64).Get<uint32_t>();

                for (uint64_t i = 0; i < count; i++)
                {
                    uint64_t storedHandler = node.Add(8 + i * 8).Get<uint64_t>();
                    if (storedHandler == handler)
                    {
                        uint64_t storedKey = node.Add(72 + i * 8).Get<uint64_t>();
                        return UnscrambleNativeKey(storedKey, seed);
                    }
                }

                node = Pointer(node.Get<uintptr_t>());
            }
        }

        return 0;
    }

    std::unordered_map<uint64_t, uintptr_t> RDR3::GetAllNatives() const
    {
        std::unordered_map<uint64_t, uintptr_t> natives;

        uint64_t seed = m_Pointers.NativeRegistrationSeed.Get<uint64_t>();

        for (uint64_t bucket = 0; bucket < 256; bucket++)
        {
            Pointer node(m_Pointers.NativeRegistrationTable.GetArray<uintptr_t>(bucket));

            while (node)
            {
                uint64_t count = node.Add(64).Get<uint32_t>();

                for (uint64_t i = 0; i < count; i++)
                {
                    uint64_t storedKey = node.Add(72 + i * 8).Get<uint64_t>();
                    uint64_t storedHandler = node.Add(8 + i * 8).Get<uint64_t>();
                    uint64_t hash = UnscrambleNativeKey(storedKey, seed);
                    natives.emplace(hash, storedHandler);
                }

                node = Pointer(node.Get<uintptr_t>());
            }
        }

        return natives;
    }

    std::string RDR3::GetTextLabel(uint32_t hash) const
    {
        static std::unordered_map<uint32_t, std::string> labelCache;

        auto it = labelCache.find(hash);
        if (it != labelCache.end())
            return it->second;

        Pointer remapMap = m_Pointers.TextLabels.Add(0xD0);
        int16_t generation = m_Pointers.TextLabels.Add(0xE8).Get<int16_t>();
        uint32_t lookupHash = hash;

        uint32_t remapBucketCount = remapMap.Add(0x08).Get<uint32_t>();
        if (remapBucketCount != 0)
        {
            Pointer bucketArray = remapMap.Deref();
            Pointer node = bucketArray.GetArray<Pointer>(hash % remapBucketCount);

            while (node && node.Get<uint32_t>() != hash)
                node = node.Add(0x18).Get<Pointer>();

            if (node)
            {
                Pointer valueArray = node.Add(0x08);
                Pointer entries = valueArray.Add(0x00).Deref();
                uint16_t entryCount = valueArray.Add(0x08).Get<uint16_t>();

                for (uint16_t i = 0; i < entryCount; i++)
                {
                    Pointer entry = entries.Add(i * 0x08);
                    if (entry.Add(0x04).Get<int16_t>() == generation)
                    {
                        uint32_t remapped = entry.Add(0x00).Get<uint32_t>();
                        if (remapped != 0)
                            lookupHash = remapped;
                        break;
                    }
                }
            }
        }

        // search every loaded slot
        Pointer slotArrayBase = m_Pointers.TextLabels.Add(0x40).Get<Pointer>();
        Pointer validFlagsBase = m_Pointers.TextLabels.Add(0x48).Get<Pointer>();
        int32_t slotCount = m_Pointers.TextLabels.Add(0x50).Get<int32_t>();
        int32_t stride = m_Pointers.TextLabels.Add(0x54).Get<int32_t>();

        for (int32_t i = 0; i < slotCount; i++)
        {
            uint8_t flag = validFlagsBase.GetArray<uint8_t>(i);
            if (flag & 0x80) // high bit set = empty slot
                continue;

            Pointer slot = slotArrayBase.Add(i * stride).Get<Pointer>();
            if (!slot)
                continue;

            std::string result = SearchTextLabelSlot(slot, lookupHash);
            if (!result.empty())
            {
                labelCache[hash] = result;
                return result;
            }
        }

        // Cache the failure (empty string) as well so we never search memory for it again
        labelCache[hash] = "";
        return {};
    }
}