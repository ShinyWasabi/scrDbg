#pragma once

namespace rage::shared
{
    union scrValue;

    class scrProgram
    {
    public:
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
        std::uint32_t m_StringsCount;
        char m_Pad3[0x0C];

        std::uint8_t* GetCode(std::uint32_t index) const
        {
            if (index < m_CodeSize)
                return &m_CodeBlocks[index >> 14][index & 0x3FFF];

            return nullptr;
        }

        static scrProgram* GetProgram(std::uint32_t hash);
        static void scrProgramConstructor1(scrProgram* program, std::uint8_t a1);
        static std::uint32_t LoadScriptProgramSCO(const char* path, const char* scriptName);
        static void scrProgramDestructor(scrProgram* program);
    };
    static_assert(sizeof(scrProgram) == 0x80);
}