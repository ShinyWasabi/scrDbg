#pragma once
#include "Disassembler.hpp"
#include "game/rage/scrProgram.hpp"

namespace scrDbg
{
    class ScriptLayout
    {
        struct InstructionEntry
        {
            uint32_t Pc;
            int StringIndex;
            int FuncIndex;
        };

        rage::scrProgram m_Program;
        std::vector<uint8_t> m_Code;
        std::vector<uint32_t> m_FunctionStarts;
        std::vector<ScriptDisassembler::FunctionInfo> m_Functions;
        std::vector<InstructionEntry> m_Instructions;

    public:
        ScriptLayout(const rage::scrProgram& program);

        const rage::scrProgram& GetProgram() const;
        const std::vector<uint8_t>& GetCode() const;
        const std::vector<ScriptDisassembler::FunctionInfo>& GetFunctions() const;
        const std::vector<InstructionEntry>& GetInstructions() const;
        uint32_t GetInstructionPc(int row) const;
        int GetFunctionIndexForPc(uint32_t pc) const;
        uint32_t GetFunctionStart(int row) const;
    };
}