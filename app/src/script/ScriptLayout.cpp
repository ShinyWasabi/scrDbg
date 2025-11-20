#include "ScriptLayout.hpp"
#include "game/rage/scrOpcode.hpp"
#include "util/ScriptHelpers.hpp"

namespace scrDbg
{
    ScriptLayout::ScriptLayout(const rage::scrProgram& program)
        : m_Program(program)
    {
        Refresh();
    }

    void ScriptLayout::Refresh()
    {
        m_Code.clear();
        m_Instructions.clear();
        m_Functions.clear();

        m_Code = m_Program.GetFullCode();
        m_Hash = m_Program.GetHash();

        int strIndex = -1;
        int funcIndex = -1;

        uint32_t pc = 0;
        while (pc < m_Code.size())
        {
            uint8_t opcode = ScriptHelpers::ReadByte(m_Code, pc);
            if (opcode == rage::scrOpcode::ENTER)
            {
                auto info = ScriptDisassembler::GetFunctionInfo(m_Code, pc, ++funcIndex);
                m_Functions.push_back({info});
            }

            if (auto newIndex = ScriptDisassembler::UpdateStringIndex(m_Code, pc))
                strIndex = *newIndex;

            m_Instructions.push_back({pc, strIndex, std::max(funcIndex, 0)});
            pc += ScriptHelpers::GetInstructionSize(m_Code, pc);
        }
    }

    const rage::scrProgram& ScriptLayout::GetProgram() const
    {
        return m_Program;
    }

    const std::vector<uint8_t>& ScriptLayout::GetCode() const
    {
        return m_Code;
    }

    const int ScriptLayout::GetInstructionCount() const
    {
        return static_cast<int>(m_Instructions.size());
    }

    const int ScriptLayout::GetFunctionCount() const
    {
        return static_cast<int>(m_Functions.size());
    }

    const uint32_t ScriptLayout::GetHash() const
    {
        return m_Hash;
    }

    ScriptLayout::InstructionEntry ScriptLayout::GetInstruction(int index) const
    {
        if (index < 0 || index >= static_cast<int>(m_Instructions.size()))
            return {};

        return m_Instructions[index];
    }

    ScriptDisassembler::FunctionInfo ScriptLayout::GetFunction(int index) const
    {
        if (index < 0 || static_cast<size_t>(index) >= m_Functions.size())
            return {};

        return m_Functions[index];
    }

    int ScriptLayout::GetFunctionIndexForPc(uint32_t pc) const
    {
        for (size_t i = 0; i < m_Functions.size(); ++i)
        {
            if (pc >= m_Functions[i].Start && pc <= m_Functions[i].End)
                return static_cast<int>(i);
        }

        return -1;
    }
}