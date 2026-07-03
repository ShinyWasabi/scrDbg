#pragma once
#include "rage/shared/pgBase.hpp"
#include "rage/shared/scrNativeContext.hpp"
#include "scrOpcode.hpp"

#if defined(_M_X64)

namespace rage::rdr2
{
    class scrProgram
    {
    public:
        class Data : pgBase
        {
        public:
            uint8_t** m_CodePages;
            uint32_t m_CodeSize;
            uint32_t m_ArgCount;
            uint32_t m_StaticCount;
            scrValue* m_Statics; // addressed in 4-byte slots although scrValue is 8 bytes
            uint32_t m_GlobalVersion;
            uint32_t m_NativeCount;
            scrNativeContext::Handler* m_Natives;
            // zeroed in the ctor of Data
            uint32_t m_Unk1;
            uint64_t m_Unk2;

            int GetInsnSize(uint32_t pc) const
            {
                auto code = GetCode(pc);

                switch (static_cast<scrOpcode>(*code))
                {
                case scrOpcode::PUSH_CONST_U8:
                case scrOpcode::ARRAY_U8:
                case scrOpcode::ARRAY_U8_LOAD:
                case scrOpcode::ARRAY_U8_STORE:
                case scrOpcode::LOCAL_U8:
                case scrOpcode::LOCAL_U8_LOAD:
                case scrOpcode::LOCAL_U8_STORE:
                case scrOpcode::STATIC_U8:
                case scrOpcode::STATIC_U8_LOAD:
                case scrOpcode::STATIC_U8_STORE:
                case scrOpcode::IADD_U8:
                case scrOpcode::IOFFSET_U8_LOAD:
                case scrOpcode::IOFFSET_U8_STORE:
                case scrOpcode::IMUL_U8:
                case scrOpcode::TEXT_LABEL_ASSIGN_STRING:
                case scrOpcode::TEXT_LABEL_ASSIGN_INT:
                case scrOpcode::TEXT_LABEL_APPEND_STRING:
                case scrOpcode::TEXT_LABEL_APPEND_INT:
                    return 2;
                case scrOpcode::PUSH_CONST_U8_U8:
                case scrOpcode::NATIVE:
                case scrOpcode::LEAVE:
                case scrOpcode::PUSH_CONST_S16:
                case scrOpcode::IADD_S16:
                case scrOpcode::IOFFSET_S16_LOAD:
                case scrOpcode::IOFFSET_S16_STORE:
                case scrOpcode::IMUL_S16:
                case scrOpcode::ARRAY_U16:
                case scrOpcode::ARRAY_U16_LOAD:
                case scrOpcode::ARRAY_U16_STORE:
                case scrOpcode::LOCAL_U16:
                case scrOpcode::LOCAL_U16_LOAD:
                case scrOpcode::LOCAL_U16_STORE:
                case scrOpcode::STATIC_U16:
                case scrOpcode::STATIC_U16_LOAD:
                case scrOpcode::STATIC_U16_STORE:
                case scrOpcode::GLOBAL_U16:
                case scrOpcode::GLOBAL_U16_LOAD:
                case scrOpcode::GLOBAL_U16_STORE:
                case scrOpcode::CALL:
                case scrOpcode::CALL_U8H_1:
                case scrOpcode::CALL_U8H_2:
                case scrOpcode::CALL_U8H_3:
                case scrOpcode::CALL_U8H_4:
                case scrOpcode::CALL_U8H_5:
                case scrOpcode::CALL_U8H_6:
                case scrOpcode::CALL_U8H_7:
                case scrOpcode::CALL_U8H_8:
                case scrOpcode::CALL_U8H_9:
                case scrOpcode::CALL_U8H_A:
                case scrOpcode::CALL_U8H_B:
                case scrOpcode::CALL_U8H_C:
                case scrOpcode::CALL_U8H_D:
                case scrOpcode::CALL_U8H_E:
                case scrOpcode::CALL_U8H_F:
                case scrOpcode::J:
                case scrOpcode::JZ:
                case scrOpcode::INE_J:
                case scrOpcode::IEQ_J:
                case scrOpcode::ILE_J:
                case scrOpcode::ILT_J:
                case scrOpcode::IGE_J:
                case scrOpcode::IGT_J:
                    return 3;
                case scrOpcode::PUSH_CONST_U8_U8_U8:
                case scrOpcode::GLOBAL_U24:
                case scrOpcode::GLOBAL_U24_LOAD:
                case scrOpcode::GLOBAL_U24_STORE:
                case scrOpcode::PUSH_CONST_U24:
                case scrOpcode::CALL_PATCH:
                case scrOpcode::CALL_OUT_OF_PATCH:
                    return 4;
                case scrOpcode::PUSH_CONST_U32:
                case scrOpcode::PUSH_CONST_F:
                    return 5;
                case scrOpcode::ENTER:
                    return 5 + code[4];
                case scrOpcode::SWITCH:
                    return 2 + code[1] * 6;
                case scrOpcode::STRING:
                    return 2 + code[1];
                case scrOpcode::ARRAY:
                    return 5 + *reinterpret_cast<int32_t*>(code + 1);
                }

                return 1;
            }

            uint32_t GetNumCodePages() const
            {
                return (m_CodeSize + 0x3FFF) >> 14;
            }

            uint32_t GetCodePageSize(uint32_t page) const
            {
                auto num = GetNumCodePages();
                if (page < num)
                {
                    if (page == num - 1)
                        return (m_CodeSize & 0x3FFF);

                    return 0x4000;
                }

                return 0;
            }

            uint8_t* GetCode(uint32_t index) const
            {
                if (index < m_CodeSize)
                    return &m_CodePages[index >> 14][index & 0x3FFF];

                return nullptr;
            }
        };

        struct PatchEntry
        {
            uint32_t m_EntryPc;
            uint32_t m_Unk1;
            uint32_t m_Unk2;
            scrOpcode m_OrigOp;
        };

        struct PatchData : Data
        {
            uint32_t m_PatchCount2;
            uint32_t m_PatchCount1;
            uint32_t m_Unk3;
            PatchEntry m_Entries[16];
        };

        const char* m_Name;
        uint32_t m_NameHash;
        Data* m_Data;
        uint16_t m_RefCount;
        PatchData* m_PatchData;
        bool m_IsRsc;

        std::string GetFuncName(uint32_t pc, uint32_t size) const;

        static scrProgram* GetByHash(uint32_t hash);
    };
}

#endif