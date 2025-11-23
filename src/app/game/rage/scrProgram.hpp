#pragma once

namespace rage
{
    class scrProgram
    {
        struct _scrProgram
        {
            char Pad1[0x10];
            uint8_t** CodeBlocks;
            uint32_t GlobalVersion;
            uint32_t CodeSize;
            uint32_t ArgCount;
            uint32_t StaticCount;
            uint32_t GlobalCountAndBlock;
            uint32_t NativeCount;
            uint64_t* Statics;
            uint64_t** Globals;
            uint64_t* Natives;
            uint32_t ProcCount;
            char Pad2[0x04];
            const char** ProcNames;
            uint32_t NameHash;
            uint32_t RefCount;
            const char* Name;
            const char** Strings;
            uint32_t StringCount;
            char Pad3[0x0C];
        };
        static_assert(sizeof(_scrProgram) == 0x80);

    public:
        scrProgram(uint64_t address = 0)
            : m_Address(address)
        {
        }

        std::vector<uint8_t> GetFullCode() const;
        uint8_t GetCode(uint32_t index) const;
        void SetCode(uint32_t index, uint8_t byte) const;
        uint32_t GetGlobalVersion() const;
        uint32_t GetCodeSize() const;
        uint32_t GetArgCount() const;
        uint32_t GetStaticCount() const;
        uint32_t GetGlobalCount() const;
        uint32_t GetGlobalBlockIndex() const;
        uint32_t GetNativeCount() const;
        uint64_t GetStatic(uint32_t index) const;
        uint64_t GetProgramGlobal(uint32_t index) const;
        uint64_t GetNative(uint32_t index) const;
        uint32_t GetHash() const;
        uint32_t GetRefCount() const;
        std::vector<std::string> GetAllStrings() const;
        std::string GetString(uint32_t index) const;
        std::vector<uint32_t> FindScriptIndex(const std::string& string) const;
        uint32_t GetStringCount() const;

        static uint64_t GetGlobal(uint32_t index);
        static void SetGlobal(uint32_t index, uint64_t value);
        static int GetGlobalBlockCount(uint32_t block);
        static scrProgram GetProgram(uint32_t hash);

        operator bool() const
        {
            return m_Address != 0;
        }

    private:
        uint64_t m_Address;
    };
}