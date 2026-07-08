#pragma once

namespace scrDbgApp
{
    class ScriptProgramRDR2 : public ScriptProgram
    {
    public:
        ScriptProgramRDR2(uintptr_t base = 0)
            : m_Base(base)
        {
        }

        std::string GetName() const override;
        uint32_t GetNameHash() const override;
        uint32_t GetGlobalVersion() const override;
        uint32_t GetCodeSize() const override;
        uint32_t GetArgCount() const override;
        uint32_t GetStaticCount() const override;
        uint32_t GetNativeCount() const override;
        uint32_t GetRefCount() const override;
        bool IsRsc() const override;
        std::vector<uint8_t> GetCode() const override;
        void SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const override;
        Pointer GetStatic(uint32_t index) const override;
        uint64_t GetNative(uint32_t index) const override;
        std::vector<std::string> GetStrings() const override;

    private:
        static constexpr size_t DATA_CODE_PAGES = 0x10;
        static constexpr size_t DATA_CODE_SIZE = 0x18;
        static constexpr size_t DATA_ARG_COUNT = 0x1C;
        static constexpr size_t DATA_STATIC_COUNT = 0x20;
        static constexpr size_t DATA_STATICS = 0x28;
        static constexpr size_t DATA_GLOBAL_VERSION = 0x30;
        static constexpr size_t DATA_NATIVE_COUNT = 0x34;
        static constexpr size_t DATA_NATIVES = 0x38;

        static constexpr size_t NAME = 0x00;
        static constexpr size_t NAME_HASH = 0x08;
        static constexpr size_t DATA = 0x10;
        static constexpr size_t REF_COUNT = 0x18;
        static constexpr size_t PATCH_DATA = 0x20; // I haven't seen any script that actually allocates this, so we're not handling it, at least for now...
        static constexpr size_t IS_RSC = 0x28;

        Pointer m_Base;
    };
}