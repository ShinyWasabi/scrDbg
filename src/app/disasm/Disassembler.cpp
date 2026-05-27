#include "Disassembler.hpp"

namespace scrDbgApp
{
    Disassembler::Disassembler(std::unique_ptr<ScriptProgram> program)
        : m_Program(std::move(program)),
          m_Hash(0)
    {
    }

    void Disassembler::Refresh()
    {
        m_Code.clear();
        m_Instructions.clear();
        m_Functions.clear();

        m_Code = m_Program->GetCode();
        m_Hash = m_Program->GetNameHash();

        std::optional<uint32_t> funcIndex = std::nullopt;
        std::optional<uint32_t> stringIndex = std::nullopt;

        uint32_t pc = 0;
        while (pc < m_Code.size())
        {
            if (SupportsFunctions())
            {
                uint32_t nextIndex = funcIndex.has_value() ? *funcIndex + 1 : 0;
                if (auto func = BuildFunction(pc, nextIndex))
                {
                    funcIndex = nextIndex;
                    m_Functions.push_back(*func);
                }
            }

            if (UsesStringsTable())
            {
                if (auto newStrIndex = UpdateStringIndex(pc))
                    stringIndex = newStrIndex;
            }

            m_Instructions.push_back({pc, funcIndex, stringIndex});

            pc += GetInstructionSize(pc);
        }
    }

    const uint32_t Disassembler::GetHash() const
    {
        return m_Hash;
    }

    const std::vector<uint8_t>& Disassembler::GetCode() const
    {
        return m_Code;
    }

    const ScriptProgram* Disassembler::GetProgram() const
    {
        return m_Program.get();
    }

    const int Disassembler::GetFunctionCount() const
    {
        return static_cast<int>(m_Functions.size());
    }

    Disassembler::FunctionInfo Disassembler::GetFunction(int index) const
    {
        if (index < 0 || static_cast<size_t>(index) >= m_Functions.size())
            return {};

        return m_Functions[index];
    }

    int Disassembler::GetFunctionIndexForPc(uint32_t pc) const
    {
        for (size_t i = 0; i < m_Functions.size(); ++i)
        {
            if (pc >= m_Functions[i].Start && pc <= m_Functions[i].End)
                return static_cast<int>(i);
        }

        return -1;
    }

    const int Disassembler::GetInstructionCount() const
    {
        return static_cast<int>(m_Instructions.size());
    }

    Disassembler::InstructionInfo Disassembler::GetInstruction(int index) const
    {
        if (index < 0 || index >= static_cast<int>(m_Instructions.size()))
            return {};

        return m_Instructions[index];
    }

    Disassembler::DecodedInstruction Disassembler::DecodeInstruction(int index) const
    {
        if (index < 0 || index >= static_cast<int>(m_Instructions.size()))
            return {};

        auto& insn = m_Instructions[index];

        DecodedInstruction result{};

        std::ostringstream addr;
        addr << "0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << insn.Pc;
        result.Address = addr.str();

        std::ostringstream bytes;
        bytes << std::hex << std::uppercase << std::setfill('0');
        int size = GetInstructionSize(insn.Pc);
        for (int i = 0; i < size; i++)
            bytes << std::setw(2) << static_cast<int>(GetU8(insn.Pc + i)) << " ";
        result.Bytes = bytes.str();

        result.Instruction = DecodeInstructionInternal(insn);
        return result;
    }

    std::vector<uint32_t> Disassembler::ScanPattern(const std::vector<std::optional<uint8_t>>& pattern) const
    {
        std::vector<uint32_t> results;

        int codeSize = static_cast<int>(m_Code.size());
        int patSize = static_cast<int>(pattern.size());
        if (patSize == 0 || codeSize < patSize)
            return results;

        for (int i = 0; i + patSize <= codeSize; ++i)
        {
            bool match = true;
            for (int j = 0; j < patSize; ++j)
            {
                if (pattern[j] && *pattern[j] != m_Code[i + j])
                {
                    match = false;
                    break;
                }
            }

            if (match)
                results.push_back(i);
        }

        return results;
    }
}