#pragma once
#include "scrOpcode.hpp"

#if defined(_M_IX86)

namespace rage
{
    union scrValue;

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

        int GetInsnSize(uint32_t pc) const
        {
            scrOpcode op = static_cast<scrOpcode>(m_Code[pc]);

            switch (op)
            {
            case scrOpcode::J:
            case scrOpcode::JZ:
            case scrOpcode::JNZ:
            case scrOpcode::PUSH_CONST_U32:
            case scrOpcode::PUSH_CONST_F:
            case scrOpcode::CALL:
                return 5;
            case scrOpcode::PUSH_CONST_S16:
                return 3;
            case scrOpcode::NATIVE:
                return 7;
            case scrOpcode::ENTER:
                return 4;
            case scrOpcode::LEAVE:
                return 3;
            case scrOpcode::TEXT_LABEL_ASSIGN_STRING:
            case scrOpcode::TEXT_LABEL_ASSIGN_INT:
            case scrOpcode::TEXT_LABEL_APPEND_STRING:
            case scrOpcode::TEXT_LABEL_APPEND_INT:
                return 2;
            case scrOpcode::STRING:
                return 2 + m_Code[pc + 1];
            case scrOpcode::SWITCH:
                return 2 + m_Code[pc + 1] * 8;
            }

            return 1;
        }

        std::string GetFuncName(uint32_t pc) const;

        static scrProgram* GetProgram(uint32_t hash);
    };
}

#endif