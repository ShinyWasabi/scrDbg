#include "Memory.hpp"

std::optional<std::uint8_t> StrToHex(char const c)
{
    switch (c)
    {
    case '0':
        return static_cast<std::uint8_t>(0x0);
    case '1':
        return static_cast<std::uint8_t>(0x1);
    case '2':
        return static_cast<std::uint8_t>(0x2);
    case '3':
        return static_cast<std::uint8_t>(0x3);
    case '4':
        return static_cast<std::uint8_t>(0x4);
    case '5':
        return static_cast<std::uint8_t>(0x5);
    case '6':
        return static_cast<std::uint8_t>(0x6);
    case '7':
        return static_cast<std::uint8_t>(0x7);
    case '8':
        return static_cast<std::uint8_t>(0x8);
    case '9':
        return static_cast<std::uint8_t>(0x9);
    case 'a':
        return static_cast<std::uint8_t>(0xa);
    case 'b':
        return static_cast<std::uint8_t>(0xb);
    case 'c':
        return static_cast<std::uint8_t>(0xc);
    case 'd':
        return static_cast<std::uint8_t>(0xd);
    case 'e':
        return static_cast<std::uint8_t>(0xe);
    case 'f':
        return static_cast<std::uint8_t>(0xf);
    case 'A':
        return static_cast<std::uint8_t>(0xA);
    case 'B':
        return static_cast<std::uint8_t>(0xB);
    case 'C':
        return static_cast<std::uint8_t>(0xC);
    case 'D':
        return static_cast<std::uint8_t>(0xD);
    case 'E':
        return static_cast<std::uint8_t>(0xE);
    case 'F':
        return static_cast<std::uint8_t>(0xF);
    default:
        return std::nullopt;
    }
}

std::vector<std::optional<std::uint8_t>> ParsePattern(std::string_view pattern)
{
    const auto sizeMinusOne = pattern.size() - 1;

    std::vector<std::optional<uint8_t>> bytes;
    bytes.reserve(sizeMinusOne / 2);
    for (size_t i = 0; i != sizeMinusOne; ++i)
    {
        if (pattern[i] == ' ')
            continue;

        if (pattern[i] != '?')
        {
            auto c1 = StrToHex(pattern[i]);
            auto c2 = StrToHex(pattern[i + 1]);
            if (c1 && c2)
            {
                bytes.emplace_back(static_cast<uint8_t>((*c1 * 0x10) + *c2));
            }
        }
        else
        {
            bytes.push_back({});
        }
    }

    return bytes;
}

std::optional<Memory> Memory::ScanPattern(const std::optional<std::uint8_t>* pattern, std::size_t length, Memory begin, std::size_t moduleSize)
{
    std::size_t maxShift = length;
    std::size_t maxIdx = length - 1;

    std::size_t wildCardIdx{static_cast<std::size_t>(-1)};
    for (int i{static_cast<int>(maxIdx - 1)}; i >= 0; --i)
    {
        if (!pattern[i])
        {
            maxShift = maxIdx - i;
            wildCardIdx = i;
            break;
        }
    }

    std::size_t shiftTable[UINT8_MAX + 1]{};
    for (std::size_t i{}; i <= UINT8_MAX; ++i)
    {
        shiftTable[i] = maxShift;
    }

    for (std::size_t i{wildCardIdx + 1}; i != maxIdx; ++i)
    {
        shiftTable[*pattern[i]] = maxIdx - i;
    }

    const auto scanEnd = moduleSize - length;
    for (std::size_t currentIdx{}; currentIdx <= scanEnd;)
    {
        for (std::ptrdiff_t patternIdx{(std::ptrdiff_t)maxIdx}; patternIdx >= 0; --patternIdx)
        {
            if (pattern[patternIdx] && *begin.Add(currentIdx + patternIdx).As<std::uint8_t*>() != *pattern[patternIdx])
            {
                currentIdx += shiftTable[*begin.Add(currentIdx + maxIdx).As<std::uint8_t*>()];
                break;
            }
            else if (patternIdx == NULL)
            {
                return begin.Add(currentIdx);
            }
        }
    }

    return std::nullopt;
}

std::optional<Memory> Memory::ScanPattern(const char* pattern)
{
    Memory base = GetModuleHandleA(0);
    auto dosHeader = base.As<IMAGE_DOS_HEADER*>();
    auto size = base.Add(dosHeader->e_lfanew).As<IMAGE_NT_HEADERS*>()->OptionalHeader.SizeOfImage;

    auto parsed = ParsePattern(pattern);
    auto data = parsed.data();
    auto length = parsed.size();

    if (auto result = ScanPattern(data, length, base, size))
        return result;

    return std::nullopt;
}