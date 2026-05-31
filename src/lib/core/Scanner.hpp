#pragma once
#include "Memory.hpp"

namespace scrDbgLib
{
    class Scanner
    {
    public:
        using ScanFunc = std::function<void(Memory)>;

        void Add(const char* pattern, const ScanFunc& func);
        bool Scan();

        static std::optional<Memory> ScanPattern(const char* pattern);

    private:
        struct Pattern
        {
            std::string m_Pattern;
            ScanFunc m_Func;
        };

        std::vector<Pattern> m_Patterns;
    };
}