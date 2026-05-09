#pragma once

namespace rage
{
    class scrProgram
    {
    public:
        scrProgram(uintptr_t base = 0)
            : m_Base(base)
        {
        }

        uint32_t GetGlobalVersion() const;
        uint32_t GetCodeSize() const;
        uint32_t GetArgCount() const;
        uint32_t GetStaticCount() const;
        uint32_t GetGlobalCount() const;
        uint32_t GetGlobalBlock() const;
        uint32_t GetNativeCount() const;
        uint32_t GetNameHash() const;
        uint32_t GetRefCount() const;
        uint32_t GetStringsSize() const;

        std::vector<uint8_t> GetFullCode() const;
        void SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const;
        uint64_t GetStatic(uint32_t index) const;
        uint64_t GetProgramGlobal(uint32_t index) const;
        uintptr_t GetNative(uint32_t index) const;
        std::vector<std::string> GetAllStrings() const;
        std::string GetString(uint32_t index) const;
        std::vector<uint32_t> FindStringIndices(const std::string& string) const;

        static uint64_t GetGlobal(uint32_t index);
        static void SetGlobal(uint32_t index, uint64_t value);
        static uint32_t GetGlobalBlockCount(uint32_t block);
        static scrProgram GetProgram(uint32_t hash);

        operator bool() const
        {
            return m_Base != 0;
        }

    private:
        static constexpr size_t CODE_PAGES = 0x10;
        static constexpr size_t GLOBAL_VERSION = 0x18;
        static constexpr size_t CODE_SIZE = 0x1C;
        static constexpr size_t ARG_COUNT = 0x20;
        static constexpr size_t STATIC_COUNT = 0x24;
        static constexpr size_t GLOBAL_COUNT_AND_BLOCK = 0x28;
        static constexpr size_t NATIVE_COUNT = 0x2C;
        static constexpr size_t STATICS = 0x30;
        static constexpr size_t GLOBAL_PAGES = 0x38;
        static constexpr size_t NATIVES = 0x40;
        static constexpr size_t NAME_HASH = 0x58;
        static constexpr size_t REF_COUNT = 0x5C;
        static constexpr size_t STRING_PAGES = 0x68;
        static constexpr size_t STRINGS_SIZE = 0x70;

        Pointer m_Base;
    };
}