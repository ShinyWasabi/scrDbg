#include "ScriptProgramPayne.hpp"

namespace scrDbgApp
{
    uint32_t ScriptProgramPayne::GetNameHash() const
    {
        return m_Base.Add(NAME_HASH).Get<uint32_t>();
    }

    uint32_t ScriptProgramPayne::GetCodeSize() const
    {
        return m_Base.Add(CODE_SIZE).Get<uint32_t>();
    }

    uint32_t ScriptProgramPayne::GetStaticCount() const
    {
        return m_Base.Add(STATIC_COUNT).Get<uint16_t>();
    }

    uint32_t ScriptProgramPayne::GetArgCount() const
    {
        return m_Base.Add(ARG_COUNT).Get<uint16_t>();
    }

    uint32_t ScriptProgramPayne::GetRefCount() const
    {
        return m_Base.Add(REF_COUNT).Get<uint16_t>();
    }

    bool ScriptProgramPayne::IsPTScript() const
    {
        return m_Base.Add(IS_PT_SCRIPT).Get<bool>();
    }

    std::vector<uint8_t> ScriptProgramPayne::GetCode() const
    {
        uint32_t codeSize = GetCodeSize();
        if (codeSize == 0)
            return {};

        std::vector<uint8_t> code(codeSize);
        m_Base.Add(CODE).Deref().GetBuffer(code.data(), codeSize);
        return code;
    }

    void ScriptProgramPayne::SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const
    {
        if (index >= GetCodeSize())
            return;

        m_Base.Add(CODE).Deref().Add(index).SetBuffer(bytes.data(), bytes.size());
    }

    Pointer ScriptProgramPayne::GetStatic(uint32_t index) const
    {
        if (index >= GetStaticCount())
            return {};

        return m_Base.Add(STATICS).Deref().Add(index * sizeof(uint32_t));
    }
}