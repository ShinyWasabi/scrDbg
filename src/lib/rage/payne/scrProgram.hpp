#pragma once

#if defined(_M_IX86)

#include "rage/shared/scrValue.hpp"
#include "scrOpcode.hpp"

namespace rage::payne
{
    class scrProgram
    {
    public:
        const char* m_Name;
        uint32_t m_NameHash;
        int32_t m_Unk; // only set to -1 in the ctor
        uint8_t* m_Code;
        scrValue* m_Statics;
        uint32_t m_CodeSize;
        uint16_t m_StaticCount;
        uint16_t m_ArgCount;
        uint16_t m_RefCount;
        bool m_IsPTScript; // PT = Payne Thresolds (it looks like R* forgot to turn off function names for these)

        std::string GetFuncName(uint32_t pc, uint32_t size) const;

        static scrProgram* GetProgram(uint32_t hash);
    };
}

#endif