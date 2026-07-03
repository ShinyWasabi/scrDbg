#include "RDR2.hpp"
#include "ResourceLoader.hpp"
#include "core/Scanner.hpp"
#include "disasm/DisassemblerRDR2.hpp"
#include "types/ScriptProgramRDR2.hpp"
#include "types/ScriptThreadRDR2.hpp"

namespace scrDbgApp
{
    bool RDR2::Init()
    {
        Scanner scanner;

        scanner.Add("GameBuild", "41 BF ? ? ? ? 48 8D 4E", [this](Pointer ptr) {
            m_Pointers.GameBuild = ptr.Add(0x0D).Add(3).Rip();
        });

        scanner.Add("ScriptThreads", "48 8B 0D ? ? ? ? FF C0", [this](Pointer ptr) {
            m_Pointers.ScriptThreads = ptr.Add(3).Rip();
        });

        scanner.Add("CommandHandlers", "4C 8B 1D ? ? ? ? 41 8B C1", [this](Pointer ptr) {
            m_Pointers.CommandHandlers = ptr.Add(3).Rip();
        });

        scanner.Add("ScriptPrograms", "4C 8B 1D ? ? ? ? 41 8B CA", [this](Pointer ptr) {
            m_Pointers.ScriptPrograms = ptr.Add(3).Rip();
        });

        scanner.Add("ScriptGlobals&ScriptGlobalsCount", "48 63 0D ? ? ? ? 48 89 05", [this](Pointer ptr) {
            m_Pointers.ScriptGlobalsCount = ptr.Add(3).Rip();
            m_Pointers.ScriptGlobals = ptr.Add(7).Add(3).Rip();
        });

        scanner.Add("StringTables", "48 8B 0D ? ? ? ? ? ? E8 ? ? ? ? ? ? ? ? ? ? 48 83 C4", [this](Pointer ptr) {
            m_Pointers.StringTables = ptr.Add(3).Rip();
        });

        return scanner.Scan();
    }

    std::unique_ptr<Disassembler> RDR2::CreateDisassembly(std::unique_ptr<ScriptProgram> program) const
    {
        auto disasm = std::make_unique<DisassemblerRDR2>(std::move(program));
        disasm->Refresh();
        return disasm;
    }

    std::string RDR2::GetGameBuild() const
    {
        char gameBuild[64]{};
        m_Pointers.GameBuild.GetBuffer(gameBuild, sizeof(gameBuild));
        return std::string(gameBuild);
    }

    Pointer RDR2::GetGlobal(uint32_t index) const
    {
        return m_Pointers.ScriptGlobals.Deref().Add(4 * index);
    }

    uint32_t RDR2::GetGlobalCount() const
    {
        return m_Pointers.ScriptGlobalsCount.Get<uint32_t>();
    }

    std::unique_ptr<ScriptProgram> RDR2::GetProgram(uint32_t hash) const
    {
        if (hash == 0)
            return nullptr;

        uintptr_t entries = m_Pointers.ScriptPrograms.Add(0).Get<uintptr_t>();
        uint32_t size = m_Pointers.ScriptPrograms.Add(8).Get<uint32_t>();

        if (size == 0)
            return nullptr;

        uint32_t index = hash % size;
        uint32_t probe = hash;

        uint32_t current = Pointer(entries + index * 16).Get<uint32_t>();
        if (current != hash)
        {
            while (current)
            {
                probe = (probe >> 1) + 1;
                index = (index + probe) % size;
                current = Pointer(entries + index * 16).Get<uint32_t>();
                if (current == hash)
                    break;
            }

            if (current == 0)
                return nullptr;
        }

        uintptr_t addr = Pointer(entries + index * 16 + sizeof(uintptr_t)).Get<uintptr_t>();
        if (!addr)
            return nullptr;

        return std::make_unique<ScriptProgramRDR2>(addr);
    }

    std::vector<std::unique_ptr<ScriptThread>> RDR2::GetThreads() const
    {
        std::vector<std::unique_ptr<ScriptThread>> threads;

        Pointer base = m_Pointers.ScriptThreads.Deref();
        uint16_t size = m_Pointers.ScriptThreads.Add(8).Get<uint16_t>();

        for (uint16_t i = 0; i < size; i++)
        {
            uintptr_t addr = base.GetArray<uintptr_t>(i);
            threads.push_back(std::make_unique<ScriptThreadRDR2>(addr));
        }

        return threads;
    }

    std::unique_ptr<ScriptThread> RDR2::GetThread(uint32_t hash) const
    {
        if (hash == 0)
            return nullptr;

        Pointer base = m_Pointers.ScriptThreads.Deref();
        uint16_t size = m_Pointers.ScriptThreads.Add(8).Get<uint16_t>();

        for (uint16_t i = 0; i < size; i++)
        {
            uintptr_t addr = base.GetArray<uintptr_t>(i);

            ScriptThreadRDR2 thread(addr);
            if (JOAAT(thread.GetScriptName()) == hash) // Intentionally not using GetScriptHash, because program hash includes the full script path
                return std::make_unique<ScriptThreadRDR2>(addr);
        }

        return nullptr;
    }

    std::string_view RDR2::GetNativeNameByHash(uint64_t hash) const
    {
        return scrDbgShared::NativesBin::GetNameByHash(hash);
    }

    uint64_t RDR2::GetNativeHashByHandler(uintptr_t handler) const
    {
        uintptr_t entries = m_Pointers.CommandHandlers.Add(0).Get<uintptr_t>();
        uint32_t size = m_Pointers.CommandHandlers.Add(8).Get<uint32_t>();

        for (uint32_t i = 0; i < size; i++)
        {
            uint32_t entryHash = Pointer(entries + i * 16).Get<uint32_t>();
            uintptr_t entryHandler = Pointer(entries + i * 16 + sizeof(uintptr_t)).Get<uintptr_t>();

            if (entryHash == 0)
                continue;

            if (entryHandler == handler)
                return entryHash;
        }

        return 0;
    }

    std::unordered_map<uint64_t, uintptr_t> RDR2::GetAllNatives() const
    {
        std::unordered_map<uint64_t, uintptr_t> result;

        uintptr_t entries = m_Pointers.CommandHandlers.Add(0).Get<uintptr_t>();
        uint32_t size = m_Pointers.CommandHandlers.Add(8).Get<uint32_t>();

        for (uint32_t i = 0; i < size; i++)
        {
            uint32_t entryHash = Pointer(entries + i * 8).Get<uint32_t>();
            uintptr_t entryHandler = Pointer(entries + i * 8 + sizeof(uintptr_t)).Get<uintptr_t>();

            if (entryHash == 0)
                continue;

            result[entryHash] = entryHandler;
        }

        return result;
    }

    std::string RDR2::GetTextLabel(uint32_t rawHash) const
    {
        static std::unordered_map<uint32_t, std::string> labelCache;

        auto it = labelCache.find(rawHash);
        if (it != labelCache.end())
            return it->second;

        Pointer stringTables = m_Pointers.StringTables.Deref();
        if (!stringTables)
            return {};

        auto searchTableMap = [](Pointer tableContainerPtr, uint32_t targetHash) -> std::string {
            Pointer mapPtr = tableContainerPtr.Add(16).Deref();
            if (!mapPtr)
                return {};

            uint32_t bucketCount = mapPtr.Add(0).Get<uint32_t>();
            Pointer bucketsArray = mapPtr.Add(8).Deref();

            if (!bucketsArray || bucketCount == 0)
                return {};

            uint32_t bucketIndex = targetHash % bucketCount;
            Pointer nodePtr = bucketsArray.Add(bucketIndex * 8).Deref();

            while (nodePtr)
            {
                uint32_t nodeKey = nodePtr.Add(0).Get<uint32_t>();
                if (nodeKey == targetHash)
                {
                    Pointer resolvedStringStruct = nodePtr.Add(8).Deref();
                    if (resolvedStringStruct)
                    {
                        Pointer realStrStruct = resolvedStringStruct.Add(8).Deref();
                        if (realStrStruct)
                            return realStrStruct.GetString(4096);
                    }
                }

                nodePtr = nodePtr.Add(16).Deref();
            }

            return {};
        };

        int32_t optTable1Count = stringTables.Add(104).Get<int32_t>();
        Pointer optTable1Array = stringTables.Add(72);
        for (int32_t i = 0; i < optTable1Count; ++i)
        {
            Pointer entryTable = optTable1Array.Add(i * 8).Deref();
            if (entryTable)
            {
                std::string res = searchTableMap(entryTable, rawHash);
                if (!res.empty())
                {
                    labelCache[rawHash] = res;
                    return res;
                }
            }
        }

        Pointer baseTable = stringTables.Add(32);
        std::string baseRes = searchTableMap(baseTable, rawHash);
        if (!baseRes.empty())
        {
            labelCache[rawHash] = baseRes;
            return baseRes;
        }

        int32_t optTable2Count = stringTables.Add(240).Get<int32_t>();
        Pointer optTable2Array = stringTables.Add(112);
        for (int32_t i = 0; i < optTable2Count; i++)
        {
            Pointer entryTable = optTable2Array.Add(i * 8).Deref();
            if (entryTable)
            {
                std::string res = searchTableMap(entryTable, rawHash);
                if (!res.empty())
                {
                    labelCache[rawHash] = res;
                    return res;
                }
            }
        }

        // Cache the failure (empty string) as well so we never search memory for it again
        labelCache[rawHash] = "";
        return {};
    }
}