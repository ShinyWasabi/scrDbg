#pragma once

namespace scrDbgApp
{
    class ScriptProgramGTA4 : public ScriptProgram
    {
    public:
        ScriptProgramGTA4(uintptr_t base = 0)
            : m_Base(base)
        {
        }

        uint32_t GetNameHash() const override;
        uint32_t GetCodeSize() const override;
        uint32_t GetStaticCount() const override;
        uint32_t GetArgCount() const override;
        uint32_t GetRefCount() const override;
        std::vector<uint8_t> GetCode() const override;
        void SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const override;
        Pointer GetStatic(uint32_t index) const override;
        std::vector<std::string> GetStrings() const override;

    private:
        static constexpr size_t NAME_HASH = 0x04;
        static constexpr size_t CODE = 0x08;
        static constexpr size_t STATICS = 0x0C;
        static constexpr size_t CODE_SIZE = 0x10;
        static constexpr size_t STATIC_COUNT = 0x14;
        static constexpr size_t ARG_COUNT = 0x16;
        static constexpr size_t REF_COUNT = 0x18;

        Pointer m_Base;
    };
}