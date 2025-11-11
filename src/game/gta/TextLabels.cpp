#include "TextLabels.hpp"
#include "Pointers.hpp"

namespace gta::TextLabels
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

    static std::string SearchTextLabelSlot(uint32_t hash, uint64_t slot)
    {
        GXT2Header header = scrDbg::Process::Read<GXT2Header>(slot);

        std::vector<GXT2Entry> entries(header.EntryCount);
        scrDbg::Process::ReadRaw(slot + sizeof(GXT2Header), entries.data(), header.EntryCount * sizeof(GXT2Entry));

        auto it = std::lower_bound(entries.begin(), entries.end(), hash, [](GXT2Entry& entry, uint32_t keyHash) {
            return entry.KeyHash < keyHash;
        });

        if (it != entries.end() && it->KeyHash == hash)
        {
            uint64_t addr = slot + it->KeyOffset;

            char buffer[4096]{};
            scrDbg::Process::ReadRaw(addr, buffer, sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';
            return buffer;
        }

        return {};
    }

    std::string GetTextLabel(uint32_t hash)
    {
        for (int i = 0; i < 23; i++)
        {
            if (auto slot = scrDbg::Process::Read<uint64_t>(scrDbg::g_Pointers.TextLabels + i * sizeof(uint64_t)))
            {
                if (auto label = SearchTextLabelSlot(hash, slot); !label.empty())
                    return label;
            }
        }

        return {};
    }
}