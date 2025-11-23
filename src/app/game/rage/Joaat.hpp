#pragma once

namespace rage
{
    constexpr uint32_t Joaat(const std::string_view str)
    {
        uint32_t hash = 0;
        for (auto c : str)
        {
            hash += c >= 'A' && c <= 'Z' ? c | 1 << 5 : c;
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);
        return hash;
    }
}

#define RAGE_JOAAT(x) rage::Joaat(x)