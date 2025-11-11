#include "Scanner.hpp"
#include "Pointer.hpp"

namespace scrDbg
{
    static std::vector<std::optional<uint8_t>> CompilePattern(const std::string& pattern)
    {
        std::vector<std::optional<uint8_t>> compiled;

        size_t len = pattern.size();
        for (size_t i = 0; i < len;)
        {
            if (pattern[i] == ' ')
            {
                i++;
                continue;
            }

            if (pattern[i] == '?')
            {
                compiled.push_back(std::nullopt);
                i++;

                if (i < len && pattern[i] == '?')
                    i++;

                continue;
            }

            auto hexChar = [](char c) -> uint8_t {
                c = static_cast<char>(std::toupper(c));

                if (c >= '0' && c <= '9')
                    return c - '0';

                if (c >= 'A' && c <= 'F')
                    return 10 + (c - 'A');

                return 0;
            };

            if (i + 1 >= len)
                break;

            uint8_t byte = hexChar(pattern[i]) * 16 + hexChar(pattern[i + 1]);
            compiled.push_back(byte);
            i += 2;
        }

        return compiled;
    }

    std::optional<uint64_t> Scanner::ScanPattern(const std::string& pattern)
    {
        constexpr int CHUNK_SIZE = 0x1000;

        std::optional<uint64_t> result;

        uint8_t* buffer = new uint8_t[CHUNK_SIZE];
        auto compiled = CompilePattern(pattern);

        uintptr_t addr = Process::GetBaseAddress();
        uintptr_t end = addr + Process::GetSize();
        while (addr < end)
        {
            if (!Process::ReadRaw(addr, buffer, CHUNK_SIZE))
            {
                addr += CHUNK_SIZE;
                continue;
            }

            for (int i = 0; i < CHUNK_SIZE; i++)
            {
                bool matched = true;
                for (size_t j = 0; j < compiled.size(); j++)
                {
                    if (!compiled[j].has_value())
                        continue;

                    if (buffer[i + j] != compiled[j].value())
                    {
                        matched = false;
                        break;
                    }
                }

                if (matched)
                {
                    result = addr + i;
                    break;
                }
            }

            if (result.has_value())
                break;

            addr += CHUNK_SIZE;
        }

        delete[] buffer;
        return result;
    }

    void Scanner::Add(const std::string& name, const std::string& pattern, const ScanFunc& func)
    {
        m_Patterns.push_back({name, pattern, func});
    }

    bool Scanner::Scan()
    {
        bool success = true;
        for (auto& pat : m_Patterns)
        {
            if (auto addr = ScanPattern(pat.m_Pattern))
            {
                pat.m_Func(Pointer(*addr));
            }
            else
            {
                success = false;
            }
        }

        return success;
    }
}