#pragma once

namespace scrDbgApp
{
    class ScriptProgramGTA5 : public ScriptProgram
    {
    public:
        ScriptProgramGTA5(uintptr_t base = 0)
            : m_Base(base)
        {
        }

        uint32_t GetGlobalVersion() const override;
        uint32_t GetCodeSize() const override;
        uint32_t GetArgCount() const override;
        uint32_t GetStaticCount() const override;
        uint32_t GetGlobalCount() const override;
        uint32_t GetGlobalBlock() const override;
        uint32_t GetNativeCount() const override;
        uint32_t GetNameHash() const override;
        uint32_t GetRefCount() const override;
        uint32_t GetStringsSize() const override;
        std::vector<uint8_t> GetCode() const override;
        void SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const override;
        int32_t GetStatic(uint32_t index) const override;
        int32_t GetProgramGlobal(uint32_t index) const override;
        uint64_t GetNative(uint32_t index) const override;
        std::vector<std::string> GetStrings() const override;
        std::string GetString(uint32_t index) const override;
        std::vector<uint32_t> FindStringIndices(const std::string& string) const override;

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