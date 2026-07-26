#pragma once

#if defined(_M_X64)

#include "rage/shared/pgBase.hpp"
#include "rage/shared/scrNativeContext.hpp"
#include "scrOpcode.hpp"

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