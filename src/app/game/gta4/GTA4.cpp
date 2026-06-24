#include "GTA4.hpp"
#include "ResourceLoader.hpp"
#include "core/Scanner.hpp"
#include "disasm/DisassemblerGTA4.hpp"
#include "types/ScriptProgramGTA4.hpp"
#include "types/ScriptThreadGTA4.hpp"

namespace scrDbgApp
{
    bool GTA4::Init()
    {
        Scanner scanner;

        scanner.Add("GameBuild", "68 ? ? ? ? 68 ? ? ? ? 50 E8 ? ? ? ? 83 C4 ? B8", [this](Pointer ptr) {
            m_Pointers.GameBuild = Pointer(ptr.Add(1).Get<uint32_t>());
        });

        scanner.Add("ScriptThreads", "8B 15 ? ? ? ? 0F B7 C7", [this](Pointer ptr) {
            m_Pointers.ScriptThreads = Pointer(ptr.Add(2).Get<uint32_t>());
        });

        scanner.Add("CommandHandlers", "C7 05 ? ? ? ? 00 00 00 00 66 3B C2", [this](Pointer ptr) {
            m_Pointers.CommandHandlers = Pointer(ptr.Add(2).Get<uint32_t>());
        });

        scanner.Add("ScriptPrograms", "8B 3D ? ? ? ? 8B CE ? ? ? 85 C0", [this](Pointer ptr) {
            m_Pointers.ScriptPrograms = Pointer(ptr.Add(2).Get<uint32_t>());
        });

        scanner.Add("ScriptGlobals", "8B 15 ? ? ? ? 57 64 8B 3D", [this](Pointer ptr) {
            m_Pointers.ScriptGlobals = Pointer(ptr.Add(2).Get<uint32_t>());
        });

        scanner.Add("ScriptGlobalsCount", "A3 ? ? ? ? 33 DB BF", [this](Pointer ptr) {
            m_Pointers.ScriptGlobalsCount = Pointer(ptr.Add(1).Get<uint32_t>());
        });

        scanner.Add("TextLabels", "B9 ? ? ? ? 4F E8", [this](Pointer ptr) {
            m_Pointers.TextLabels = Pointer(ptr.Add(1).Get<uint32_t>());
        });

        return scanner.Scan();
    }

    std::unique_ptr<Disassembler> GTA4::CreateDisassembly(std::unique_ptr<ScriptProgram> program) const
    {
        auto disasm = std::make_unique<DisassemblerGTA4>(std::move(program));
        disasm->Refresh();
        return disasm;
    }

    std::string GTA4::GetGameBuild() const
    {
        char gameBuild[64]{};
        m_Pointers.GameBuild.GetBuffer(gameBuild, sizeof(gameBuild));
        return std::string(gameBuild);
    }

    Pointer GTA4::GetGlobal(uint32_t index) const
    {
        return m_Pointers.ScriptGlobals.Deref().Add(index * sizeof(uint32_t));
    }

    uint32_t GTA4::GetGlobalCount() const
    {
        return m_Pointers.ScriptGlobalsCount.Get<uint32_t>();
    }

    std::unique_ptr<ScriptProgram> GTA4::GetProgram(uint32_t hash) const
    {
        if (hash == 0)
            return nullptr;

        uint32_t entries = m_Pointers.ScriptPrograms.Add(0).Get<uint32_t>();
        uint32_t size = m_Pointers.ScriptPrograms.Add(4).Get<uint32_t>();

        if (size == 0)
            return nullptr;

        uint32_t index = hash % size;
        uint32_t probe = hash;

        uint32_t current = Pointer(entries + index * 8).Get<uint32_t>();
        if (current != hash)
        {
            while (current)
            {
                probe = (probe >> 1) + 1;
                index = (index + probe) % size;
                current = Pointer(entries + index * 8).Get<uint32_t>();
                if (current == hash)
                    break;
            }

            if (current == 0)
                return nullptr;
        }

        uint32_t addr = Pointer(entries + index * 8 + sizeof(uint32_t)).Get<uint32_t>();
        if (!addr)
            return nullptr;

        return std::make_unique<ScriptProgramGTA4>(addr);
    }

    std::vector<std::unique_ptr<ScriptThread>> GTA4::GetThreads() const
    {
        std::vector<std::unique_ptr<ScriptThread>> threads;

        Pointer base = m_Pointers.ScriptThreads.Deref();
        uint16_t size = m_Pointers.ScriptThreads.Add(4).Get<uint16_t>();

        for (uint16_t i = 0; i < size; i++)
        {
            Pointer addr = base.GetArray<uint32_t>(i);
            threads.push_back(std::make_unique<ScriptThreadGTA4>(addr));
        }

        return threads;
    }

    std::unique_ptr<ScriptThread> GTA4::GetThread(uint32_t hash) const
    {
        if (hash == 0)
            return nullptr;

        Pointer base = m_Pointers.ScriptThreads.Deref();
        uint16_t size = m_Pointers.ScriptThreads.Add(4).Get<uint16_t>();

        for (uint16_t i = 0; i < size; i++)
        {
            Pointer addr = base.GetArray<uint32_t>(i);

            ScriptThreadGTA4 thread(addr);
            if (thread.GetScriptHash() == hash)
                return std::make_unique<ScriptThreadGTA4>(addr);
        }

        return nullptr;
    }

    std::string_view GTA4::GetNativeNameByHash(uint64_t hash) const
    {
        return scrDbgShared::NativesBin::GetNameByHash(hash);
    }

    uint64_t GTA4::GetNativeHashByHandler(uintptr_t handler) const
    {
        uint32_t entries = m_Pointers.CommandHandlers.Add(0).Get<uint32_t>();
        uint32_t size = m_Pointers.CommandHandlers.Add(4).Get<uint32_t>();

        for (uint32_t i = 0; i < size; i++)
        {
            uint32_t entryHash = Pointer(entries + i * 8).Get<uint32_t>();
            uint32_t entryHandler = Pointer(entries + i * 8 + 4).Get<uint32_t>();

            if (entryHash == 0)
                continue;

            if (entryHandler == static_cast<uint32_t>(handler))
                return entryHash;
        }

        return 0;
    }

    std::unordered_map<uint64_t, uintptr_t> GTA4::GetAllNatives() const
    {
        std::unordered_map<uint64_t, uintptr_t> result;

        uint32_t entries = m_Pointers.CommandHandlers.Add(0).Get<uint32_t>();
        uint32_t size = m_Pointers.CommandHandlers.Add(4).Get<uint32_t>();

        for (uint32_t i = 0; i < size; i++)
        {
            uint32_t entryHash = Pointer(entries + i * 8).Get<uint32_t>();
            uint32_t entryHandler = Pointer(entries + i * 8 + 4).Get<uint32_t>();

            if (entryHash == 0)
                continue;

            result[entryHash] = entryHandler;
        }

        return result;
    }

    std::string GTA4::GetTextLabel(uint32_t hash) const
    {
        struct GXT2Entry
        {
            uint32_t KeyOffset;
            uint32_t KeyHash;
        };

        Pointer data = m_Pointers.TextLabels.Add(0).Deref();
        int32_t count = m_Pointers.TextLabels.Add(4).Get<int32_t>();
        if (!data || count <= 0)
            return {};

        std::vector<GXT2Entry> entries(count);
        data.GetBuffer(entries.data(), count * sizeof(GXT2Entry));

        auto it = std::lower_bound(entries.begin(), entries.end(), hash, [](const GXT2Entry& entry, uint32_t keyHash) {
            return entry.KeyHash < keyHash;
        });

        if (it != entries.end() && it->KeyHash == hash)
        {
            std::vector<wchar_t> buffer(4096);
            m_Pointers.TextLabels.Add(8).Deref().Add(it->KeyOffset).GetBuffer(buffer.data(), buffer.size() * sizeof(wchar_t));

            int32_t len = WideCharToMultiByte(CP_UTF8, 0, buffer.data(), -1, nullptr, 0, nullptr, nullptr);
            if (len <= 0)
                return {};

            std::string result(len - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, buffer.data(), -1, result.data(), len, nullptr, nullptr);
            return result;
        }

        return {};
    }
}