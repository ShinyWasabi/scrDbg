
#include "ScriptLayout.hpp"
#include "game/rage/Opcode.hpp"

namespace scrDbg
{
    ScriptLayout::ScriptLayout(const rage::scrProgram& program) :
        m_Program(program)
    {
        m_Code = m_Program.GetFullCode();

        int strIndex = -1;
        int funcIndex = -1;

        uint32_t pc = 0;
        while (pc < m_Code.size())
        {
            uint8_t opcode = ScriptDisassembler::ReadByte(m_Code, pc);
            if (opcode == rage::scrOpcode::ENTER)
            {
                m_FunctionStarts.push_back(pc);
                funcIndex++;

                auto info = ScriptDisassembler::GetFunctionInfo(m_Code, pc, funcIndex);
                m_Functions.push_back({ info });
            }

            strIndex = ScriptDisassembler::GetNextStringIndex(m_Code, pc, strIndex);
            m_Instructions.push_back({ pc, strIndex, std::max(funcIndex, 0) });

            pc += ScriptDisassembler::GetInstructionSize(m_Code, pc);
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

    const std::vector<ScriptDisassembler::FunctionInfo>& ScriptLayout::GetFunctions() const
    {
        return m_Functions;
    }

    const std::vector<ScriptLayout::InstructionEntry>& ScriptLayout::GetInstructions() const
    {
        return m_Instructions;
    }

    uint32_t ScriptLayout::GetInstructionPc(int row) const
    {
        if (row < 0 || row >= static_cast<int>(m_Instructions.size()))
            return 0;

        return m_Instructions[row].Pc;
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

    uint32_t ScriptLayout::GetFunctionStart(int row) const
    {
        if (row < 0 || row >= static_cast<int>(m_Functions.size()))
            return 0;

        return m_Functions[row].Start;
    }
}