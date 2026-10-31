#pragma once
#include "Instruction.hpp"
#include "disasm/Disassembler.hpp"

namespace scrDbgApp::Decomp
{
    class Function
    {
    public:
        Function(const Disassembler::FunctionInfo& info, uint32_t end, uint32_t reservedSlots);

        const Disassembler::FunctionInfo& GetInfo() const
        {
            return m_Info;
        }

        const std::string& GetName() const
        {
            return m_Info.Name;
        }

        uint32_t GetStart() const
        {
            return m_Info.Start;
        }

        uint32_t GetEnd() const // One past the last LEAVE
        {
            return m_End;
        }

        uint32_t GetArgCount() const
        {
            return m_Info.ArgCount;
        }

        uint32_t GetFrameSize() const
        {
            return m_Info.FrameSize;
        }

        uint32_t GetRetCount() const
        {
            return m_Info.RetCount;
        }

        uint32_t GetLocalCount() const
        {
            uint32_t used = m_Info.ArgCount + m_ReservedSlots;
            return m_Info.FrameSize > used ? m_Info.FrameSize - used : 0;
        }

        std::string GetFrameVarName(uint32_t index) const;

        void SetInstructions(std::vector<Instruction> instructions);

        std::vector<Instruction>& GetInstructions()
        {
            return m_Instructions;
        }

        const std::vector<Instruction>& GetInstructions() const
        {
            return m_Instructions;
        }

        int32_t GetInstructionIndexAtOffset(uint32_t offset) const;

    private:
        void CollapseShortCircuits();

        Disassembler::FunctionInfo m_Info;
        uint32_t m_End;
        uint32_t m_ReservedSlots;
        std::vector<Instruction> m_Instructions;
    };
}