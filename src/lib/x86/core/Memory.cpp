#include "Memory.hpp"

std::optional<uint8_t> StrToHex(char const c)
{
    switch (c)
    {
    case '0':
        return static_cast<uint8_t>(0x0);
    case '1':
        return static_cast<uint8_t>(0x1);
    case '2':
        return static_cast<uint8_t>(0x2);
    case '3':
        return static_cast<uint8_t>(0x3);
    case '4':
        return static_cast<uint8_t>(0x4);
    case '5':
        return static_cast<uint8_t>(0x5);
    case '6':
        return static_cast<uint8_t>(0x6);
    case '7':
        return static_cast<uint8_t>(0x7);
    case '8':
        return static_cast<uint8_t>(0x8);
    case '9':
        return static_cast<uint8_t>(0x9);
    case 'a':
        return static_cast<uint8_t>(0xa);
    case 'b':
        return static_cast<uint8_t>(0xb);
    case 'c':
        return static_cast<uint8_t>(0xc);
    case 'd':
        return static_cast<uint8_t>(0xd);
    case 'e':
        return static_cast<uint8_t>(0xe);
    case 'f':
        return static_cast<uint8_t>(0xf);
    case 'A':
        return static_cast<uint8_t>(0xA);
    case 'B':
        return static_cast<uint8_t>(0xB);
    case 'C':
        return static_cast<uint8_t>(0xC);
    case 'D':
        return static_cast<uint8_t>(0xD);
    case 'E':
        return static_cast<uint8_t>(0xE);
    case 'F':
        return static_cast<uint8_t>(0xF);
    }

    return std::nullopt;
}

std::vector<std::optional<uint8_t>> Memory::ParsePattern(std::string_view pattern)
{
    const size_t sizeMinusOne = pattern.size() - 1;

    std::vector<std::optional<uint8_t>> bytes;
    bytes.reserve(sizeMinusOne / 2);

    for (size_t i = 0; i != sizeMinusOne; i++)
    {
        if (pattern[i] == ' ')
            continue;

        if (pattern[i] != '?')
        {
            auto c1 = StrToHex(pattern[i]);
            auto c2 = StrToHex(pattern[i + 1]);
            if (c1 && c2)
                bytes.emplace_back(static_cast<uint8_t>((*c1 * 0x10) + *c2));
        }
        else
        {
            bytes.push_back({});
        }
    }

    return bytes;
}

std::optional<Memory> Memory::ScanPattern(const char* pattern)
{
    uint8_t* base = reinterpret_cast<uint8_t*>(GetModuleHandleA(nullptr));

    PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
    PIMAGE_NT_HEADERS nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);

    auto parsed = ParsePattern(pattern);

    size_t moduleSize = nt->OptionalHeader.SizeOfImage;
    size_t patternSize = parsed.size();

    for (size_t i = 0; i < moduleSize - patternSize; i++)
    {
        bool match = true;
        for (size_t j = 0; j < patternSize; j++)
        {
            if (parsed[j].has_value() && base[i + j] != parsed[j].value())
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            LOGF("Pattern Match: 0x%p", base + i);
            return Memory(base + i);
        }
    }

    LOG("Pattern not found.");
    return std::nullopt;
}