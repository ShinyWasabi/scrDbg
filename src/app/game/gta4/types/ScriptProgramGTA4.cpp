#include "ScriptProgramGTA4.hpp"
#include "disasm/opcodes/OpcodesGTA4.hpp"

namespace scrDbgApp
{
    uint32_t ScriptProgramGTA4::GetNameHash() const
    {
        return m_Base.Add(NAME_HASH).Get<uint32_t>();
    }

    uint32_t ScriptProgramGTA4::GetCodeSize() const
    {
        return m_Base.Add(CODE_SIZE).Get<uint32_t>();
    }

    uint32_t ScriptProgramGTA4::GetStaticCount() const
    {
        return m_Base.Add(STATIC_COUNT).Get<uint16_t>();
    }

    uint32_t ScriptProgramGTA4::GetArgCount() const
    {
        return m_Base.Add(ARG_COUNT).Get<uint16_t>();
    }

    uint32_t ScriptProgramGTA4::GetRefCount() const
    {
        return m_Base.Add(REF_COUNT).Get<uint16_t>();
    }

    std::vector<uint8_t> ScriptProgramGTA4::GetCode() const
    {
        uint32_t codeSize = GetCodeSize();
        if (codeSize == 0)
            return {};

        std::vector<uint8_t> code(codeSize);
        m_Base.Add(CODE).Deref().GetBuffer(code.data(), codeSize);
        return code;
    }

    void ScriptProgramGTA4::SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const
    {
        if (index >= GetCodeSize())
            return;

        m_Base.Add(CODE).Deref().Add(index).SetBuffer(bytes.data(), bytes.size());
    }

    Pointer ScriptProgramGTA4::GetStatic(uint32_t index) const
    {
        if (index >= GetStaticCount())
            return {};

        return m_Base.Add(STATICS).Deref().Add(index * sizeof(uint32_t));
    }

    std::vector<std::string> ScriptProgramGTA4::GetStrings() const
    {
        std::vector<std::string> strings;
        std::unordered_set<std::string> seen;

        std::vector<uint8_t> code = GetCode();

        for (uint32_t pc = 0; pc + 1 < code.size();)
        {
            if (static_cast<OpcodesGTA4>(code[pc]) == OpcodesGTA4::STRING)
            {
                uint8_t len = code[pc + 1];
                if (pc + 2 + len > code.size())
                    break;

                std::string str(reinterpret_cast<const char*>(&code[pc + 2]), len);
                if (!str.empty() && str.back() == '\0')
                    str.pop_back();

                if (seen.insert(str).second)
                    strings.push_back(std::move(str));
            }

            pc += GetInsnSizeGTA4(code.data(), pc);
        }

        return strings;
    }
}