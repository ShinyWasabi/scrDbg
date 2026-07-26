#pragma once

#if defined(_M_X64)

#include "rage/shared/atMap.hpp"
#include "rage/shared/pgBase.hpp"
#include "rage/shared/scrValue.hpp"
#include "scrOpcode.hpp"

namespace rage::gta5
{
    class scrProgram : public pgBase
    {
    public:
        uint8_t** m_CodePages;
        uint32_t m_GlobalVersion;
        uint32_t m_CodeSize;
        uint32_t m_ArgCount;
        uint32_t m_StaticCount;
        uint32_t m_GlobalCountAndBlock;
        uint32_t m_NativeCount;
        scrValue* m_Statics;
        scrValue** m_GlobalPages;
        uint64_t* m_Natives;
        uint32_t m_ProcCount;
        char m_Pad1[0x04];
        const char** m_ProcNames;
        uint32_t m_NameHash;
        uint32_t m_RefCount;
        const char* m_Name;
        const char** m_StringPages;
        uint32_t m_StringsSize;
        char m_Pad2[0x04];
        atMap<uint32_t, bool>* m_Breakpoints;

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

        uint32_t GetGlobalCount() const
        {
            return (m_GlobalCountAndBlock & 0x3FFFF);
        }

        uint32_t GetGlobalBlock() const
        {
            return (m_GlobalCountAndBlock >> 0x12);
        }

        uint32_t GetNumGlobalPages() const
        {
            return ((m_GlobalCountAndBlock & 0x3FFFF) + 0x3FFF) >> 14;
        }

        uint32_t GetGlobalPageSize(uint32_t page) const
        {
            auto num = GetNumGlobalPages();
            if (page < num)
            {
                if (page == num - 1)
                    return (m_GlobalCountAndBlock & 0x3FFFF) - (page << 14);

                return 0x4000;
            }

            return 0;
        }

        scrValue* GetGlobal(uint32_t index) const
        {
            if (index < GetGlobalCount())
                return &m_GlobalPages[index >> 14][index & 0x3FFF];

            return nullptr;
        }

        uint32_t GetNumStringPages() const
        {
            return (m_StringsSize + 0x3FFF) >> 14;
        }

        uint32_t GetStringPageSize(uint32_t page) const
        {
            auto num = GetNumStringPages();
            if (page < num)
            {
                if (page == num - 1)
                    return (m_StringsSize & 0x3FFF);

                return 0x4000;
            }

            return 0;
        }

        const char* GetString(uint32_t index) const
        {
            if (index < m_StringsSize)
                return &m_StringPages[index >> 14][index & 0x3FFF];

            return nullptr;
        }

        std::string GetFuncName(uint32_t pc, uint32_t size) const;

        static scrProgram* GetByHash(uint32_t hash);
    };

    struct scrProgramRegistry
    {
        uint8_t m_NextFreeSlot;
        uint8_t m_BucketHeads[32];
        uint8_t m_NextInChain[176];
        char m_Pad1[0x07];
        scrProgram* m_Programs[176];

        scrProgram* Find(uint32_t hash)
        {
            auto index = m_BucketHeads[hash & 0x1F];

            while (index)
            {
                if (m_Programs[index]->m_NameHash == hash)
                    return m_Programs[index];

                if (!m_NextInChain[index])
                    break;

                index = m_NextInChain[index];
            }

            return nullptr;
        }
    };
}

#endif