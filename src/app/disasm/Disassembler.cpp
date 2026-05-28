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

        uint32_t pc = 0;
        while (pc < m_Code.size())
        {
            BuildFunction(pc);

            m_Instructions.push_back(pc);

            pc += GetInstructionSize(pc);
        }
    }

    uint32_t Disassembler::GetHash() const
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

    int Disassembler::GetFunctionCount() const
    {
        return static_cast<int>(m_Functions.size());
    }

    Disassembler::FunctionInfo Disassembler::GetFunction(int index) const
    {
        if (index < 0 || static_cast<size_t>(index) >= m_Functions.size())
            return {};

        return m_Functions[index];
    }

    std::optional<Disassembler::FunctionInfo> Disassembler::GetFunctionForPc(uint32_t pc) const
    {
        for (const auto& func : m_Functions)
        {
            if (pc >= func.Start && pc <= func.End)
                return func;
        }

        return std::nullopt;
    }

    int Disassembler::GetInstructionCount() const
    {
        return static_cast<int>(m_Instructions.size());
    }

    uint32_t Disassembler::GetInstruction(int index) const
    {
        if (index < 0 || index >= static_cast<int>(m_Instructions.size()))
            return 0;

        return m_Instructions[index];
    }

    Disassembler::DecodedInstruction Disassembler::DecodeInstruction(int index) const
    {
        if (index < 0 || index >= static_cast<int>(m_Instructions.size()))
            return {};

        uint32_t pc = m_Instructions[index];

        DecodedInstruction result{};

        std::ostringstream addr;
        addr << "0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << pc;
        result.Address = addr.str();

        std::ostringstream bytes;
        bytes << std::hex << std::uppercase << std::setfill('0');
        int size = GetInstructionSize(pc);
        for (int i = 0; i < size; i++)
            bytes << std::setw(2) << static_cast<int>(GetU8(pc + i)) << " ";
        result.Bytes = bytes.str();

        result.Instruction = DecodeInstructionInternal(index);
        return result;
    }

    std::vector<uint32_t> Disassembler::ScanPattern(const std::vector<std::optional<uint8_t>>& pattern) const
    {
        std::vector<uint32_t> results;

        int codeSize = static_cast<int>(m_Code.size());
        int patSize = static_cast<int>(pattern.size());
        if (patSize == 0 || codeSize < patSize)
            return results;

        for (int i = 0; i + patSize <= codeSize; i++)
        {
            bool match = true;
            for (int j = 0; j < patSize; j++)
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