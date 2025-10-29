
#include "ScriptLayout.hpp"
#include "game/rage/Opcode.hpp"

namespace scrDbg
{
    ScriptLayout::ScriptLayout(const rage::scrProgram& program) :
        m_Program(program)
    {
        m_Code = m_Program.GetFullCode();

        // Scan function starts
        uint32_t pc = 0;
        while (pc < m_Code.size())
        {
            if (ScriptDisassembler::ReadByte(m_Code, pc) == rage::scrOpcode::ENTER)
                m_FunctionStarts.push_back(pc);

            pc += ScriptDisassembler::GetInstructionSize(m_Code, pc);
        }

        // Precompute functions info
        for (size_t i = 0; i < m_FunctionStarts.size(); ++i)
        {
            auto info = ScriptDisassembler::GetFunctionInfo(m_Code, m_FunctionStarts[i], i);
            m_Functions.push_back({ info });
        }

        // Precompute instructions
        int strIndex = -1;
        int funcIndex = 0;
        pc = 0;
        while (pc < m_Code.size())
        {
            if (funcIndex + 1 < m_FunctionStarts.size() && pc >= m_FunctionStarts[funcIndex + 1])
                funcIndex++;

            strIndex = ScriptDisassembler::GetNextStringIndex(m_Code, pc, strIndex);
            m_Instructions.push_back({ pc, strIndex, funcIndex });

            pc += ScriptDisassembler::GetInstructionSize(m_Code, pc);
        }
    }

    const rage::scrProgram& ScriptLayout::GetProgram() const
    {
        return m_Program;
    }

    std::vector<uint8_t>& ScriptLayout::GetCode()
    {
        return m_Code;
    }

    const std::vector<ScriptDisassembler::FunctionInfo>& ScriptLayout::GetFunctions() const
    {
        return m_Functions;
    }

    const std::vector<ScriptLayout::InstructionEntry>& ScriptLayout::GetInstructions() const
    {
        return m_Instructions;
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