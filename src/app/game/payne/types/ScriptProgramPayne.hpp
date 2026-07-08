#pragma once

namespace scrDbgApp
{
    class ScriptProgramPayne : public ScriptProgram
    {
    public:
        ScriptProgramPayne(uintptr_t base = 0)
            : m_Base(base)
        {
        }

        uint32_t GetNameHash() const override;
        uint32_t GetCodeSize() const override;
        uint32_t GetStaticCount() const override;
        uint32_t GetArgCount() const override;
        uint32_t GetRefCount() const override;
        bool IsPTScript() const override;
        std::vector<uint8_t> GetCode() const override;
        void SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const override;
        Pointer GetStatic(uint32_t index) const override;
        std::vector<std::string> GetStrings() const override;

    private:
        static constexpr size_t NAME_HASH = 0x04;
        static constexpr size_t CODE = 0x0C;
        static constexpr size_t STATICS = 0x10;
        static constexpr size_t CODE_SIZE = 0x14;
        static constexpr size_t STATIC_COUNT = 0x18;
        static constexpr size_t ARG_COUNT = 0x1A;
        static constexpr size_t REF_COUNT = 0x1C;
        static constexpr size_t IS_PT_SCRIPT = 0x1E;

        Pointer m_Base;
    };
}