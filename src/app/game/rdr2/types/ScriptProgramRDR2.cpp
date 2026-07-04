#include "ScriptProgramRDR2.hpp"

namespace scrDbgApp
{
    std::string ScriptProgramRDR2::GetName() const
    {
        return m_Base.Add(NAME).Deref().GetString();
    }

    uint32_t ScriptProgramRDR2::GetNameHash() const
    {
        return m_Base.Add(NAME_HASH).Get<uint32_t>();
    }

    uint32_t ScriptProgramRDR2::GetGlobalVersion() const
    {
        return m_Base.Add(DATA).Deref().Add(DATA_GLOBAL_VERSION).Get<uint32_t>();
    }

    uint32_t ScriptProgramRDR2::GetCodeSize() const
    {
        return m_Base.Add(DATA).Deref().Add(DATA_CODE_SIZE).Get<uint32_t>();
    }

    uint32_t ScriptProgramRDR2::GetArgCount() const
    {
        return m_Base.Add(DATA).Deref().Add(DATA_ARG_COUNT).Get<uint32_t>();
    }

    uint32_t ScriptProgramRDR2::GetStaticCount() const
    {
        return m_Base.Add(DATA).Deref().Add(DATA_STATIC_COUNT).Get<uint32_t>();
    }

    uint32_t ScriptProgramRDR2::GetNativeCount() const
    {
        return m_Base.Add(DATA).Deref().Add(DATA_NATIVE_COUNT).Get<uint32_t>();
    }

    uint32_t ScriptProgramRDR2::GetRefCount() const
    {
        return m_Base.Add(REF_COUNT).Get<uint16_t>();
    }

    bool ScriptProgramRDR2::IsRsc() const
    {
        return m_Base.Add(IS_RSC).Get<bool>();
    }

    std::vector<uint8_t> ScriptProgramRDR2::GetCode() const
    {
        uint32_t codeSize = GetCodeSize();
        if (!codeSize)
            return {};

        std::vector<uint8_t> code(codeSize);
        uint32_t pageCount = (codeSize + 0x3FFF) >> 14;

        std::vector<uintptr_t> pages(pageCount);
        m_Base.Add(DATA).Deref().Add(DATA_CODE_PAGES).Deref().GetBuffer(pages.data(), pageCount * sizeof(uintptr_t));
        for (uint32_t i = 0, offset = 0; i < pageCount; i++)
        {
            size_t pageSize = std::min<size_t>(codeSize - offset, 0x4000);
            Pointer(pages[i]).GetBuffer(code.data() + offset, pageSize);
            offset += pageSize;
        }

        return code;
    }

    void ScriptProgramRDR2::SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const
    {
        if (index >= GetCodeSize())
            return;

        uintptr_t page = m_Base.Add(DATA).Deref().Add(DATA_CODE_PAGES).Deref().GetArray<uintptr_t>(index >> 14);
        Pointer(page).Add(index & 0x3FFF).SetBuffer(bytes.data(), bytes.size());
    }

    Pointer ScriptProgramRDR2::GetStatic(uint32_t index) const
    {
        if (index >= GetStaticCount())
            return {};

        return m_Base.Add(DATA).Deref().Add(DATA_STATICS).Deref().Add(4 * index);
    }

    uint64_t ScriptProgramRDR2::GetNative(uint32_t index) const
    {
        if (index >= GetNativeCount())
            return {};

        return m_Base.Add(DATA).Deref().Add(DATA_NATIVES).Deref().GetArray<uint64_t>(index);
    }
}