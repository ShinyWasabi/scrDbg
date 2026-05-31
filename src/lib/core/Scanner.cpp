#include "Scanner.hpp"

namespace scrDbgLib
{
    static std::optional<uint8_t> StrToHex(char const c)
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
        default:
            return std::nullopt;
        }
    }

    static std::vector<std::optional<uint8_t>> ParsePattern(std::string_view pattern)
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

    std::optional<Memory> Scanner::ScanPattern(const char* pattern)
    {
        auto* base = reinterpret_cast<uint8_t*>(GetModuleHandleA(nullptr));
        auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
        auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
        auto size = nt->OptionalHeader.SizeOfImage;

        auto parsed = ParsePattern(pattern);
        auto data = parsed.data();
        auto length = parsed.size();

        for (size_t i = 0; i < size - length; i++)
        {
            bool match = true;
            for (size_t j = 0; j < length; j++)
            {
                if (parsed[j].has_value() && base[i + j] != parsed[j].value())
                {
                    match = false;
                    break;
                }
            }

            if (match)
                return Memory(base + i);
        }

        return std::nullopt;
    }

    void Scanner::Add(const char* pattern, const ScanFunc& func)
    {
        m_Patterns.push_back({pattern, func});
    }

    bool Scanner::Scan()
    {
        bool success = true;
        for (auto& pattern : m_Patterns)
        {
            if (auto addr = ScanPattern(pattern.m_Pattern.c_str()))
            {
                pattern.m_Func(*addr);
            }
            else
            {
                success = false;
            }
        }

        return success;
    }
}