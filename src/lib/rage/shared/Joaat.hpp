#pragma once

namespace rage::shared
{
    constexpr std::uint32_t Joaat(const std::string_view str)
    {
        std::uint32_t hash = 0;

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

inline consteval std::uint32_t operator""_J(const char* s, std::size_t n)
{
    std::uint32_t hash = 0;

    for (std::size_t i = 0; i < n; i++)
    {
        hash += s[i] >= 'A' && s[i] <= 'Z' ? s[i] | 1 << 5 : s[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}