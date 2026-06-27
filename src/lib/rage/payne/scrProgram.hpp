#pragma once
#include "rage/shared/scrValue.hpp"
#include "scrOpcode.hpp"

#if defined(_M_IX86)

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

        int GetInsnSize(uint32_t pc) const
        {
            scrOpcode op = static_cast<scrOpcode>(m_Code[pc]);

            switch (op)
            {
            case scrOpcode::J:
            case scrOpcode::JNZ:
            case scrOpcode::JZ:
            case scrOpcode::PUSH_CONST_U32:
            case scrOpcode::PUSH_CONST_F:
            case scrOpcode::CALL:
                return 5;
            case scrOpcode::PUSH_CONST_S16:
            case scrOpcode::LEAVE:
                return 3;
            case scrOpcode::NATIVE:
                return 7;
            case scrOpcode::ENTER:
                return m_Code[pc + 4] + 5;
            case scrOpcode::SWITCH:
                return 8 * m_Code[pc + 1] + 2;
            case scrOpcode::STRING:
                return m_Code[pc + 1] + 2;
            case scrOpcode::TEXT_LABEL_ASSIGN_STRING:
            case scrOpcode::TEXT_LABEL_ASSIGN_INT:
            case scrOpcode::TEXT_LABEL_APPEND_STRING:
            case scrOpcode::TEXT_LABEL_APPEND_INT:
                return 2;
            }

            return 1;
        }

        std::string GetFuncName(uint32_t pc, uint32_t size) const;

        static scrProgram* GetProgram(uint32_t hash);
    };
}

#endif