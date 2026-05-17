#pragma once

namespace rage
{
    union scrValue;

    class scrProgram
    {
    public:
        std::uint8_t* GetCode(std::uint32_t index) const
        {
            if (index < m_CodeSize)
                return &m_CodeBlocks[index >> 14U][index & 0x3FFFU];

            return nullptr;
        }

        uint32_t GetCodeSize() const
        {
            return m_CodeSize;
        }

        uint64_t GetNative(uint32_t index) const
        {
            return m_Natives[index];
        }

        const char* GetString(std::uint32_t index) const
        {
            if (index < m_StringCount)
                return &m_Strings[index >> 14U][index & 0x3FFFU];

            return nullptr;
        }

        static scrProgram* GetProgram(std::uint32_t hash);
        static void scrProgramConstructor(scrProgram* program, std::uint8_t a1);
        static void scrProgramDestructor(scrProgram* program);
        static std::uint32_t LoadScriptProgramSCO(const char* path, const char* scriptName);

    private:
        char m_Pad1[0x10];
        std::uint8_t** m_CodeBlocks;
        std::uint32_t m_GlobalVersion;
        std::uint32_t m_CodeSize;
        std::uint32_t m_ArgCount;
        std::uint32_t m_StaticCount;
        std::uint32_t m_GlobalCount;
        std::uint32_t m_NativeCount;
        scrValue* m_Statics;
        scrValue** m_Globals;
        std::uint64_t* m_Natives;
        std::uint32_t m_ProcCount;
        char m_Pad2[0x04];
        const char** m_ProcNames;
        std::uint32_t m_NameHash;
        std::uint32_t m_RefCount;
        const char* m_Name;
        const char** m_Strings;
        std::uint32_t m_StringCount;
        char m_Pad3[0x0C];
    };
    static_assert(sizeof(scrProgram) == 0x80);
}