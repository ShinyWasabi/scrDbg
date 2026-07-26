#pragma once

#if defined(_M_IX86)

#include "rage/shared/scrValue.hpp"
#include "scrOpcode.hpp"

namespace rage::gta4
{
    class scrProgram
    {
    public:
        const char* m_Name;
        uint32_t m_NameHash;
        uint8_t* m_Code;
        scrValue* m_Statics;
        uint32_t m_CodeSize;
        uint16_t m_StaticCount;
        uint16_t m_ArgCount;
        uint16_t m_RefCount;

        std::string GetFuncName(uint32_t pc) const;

        static scrProgram* GetProgram(uint32_t hash);
    };
}

#endif