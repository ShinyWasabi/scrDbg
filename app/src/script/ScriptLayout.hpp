#pragma once
#include "ScriptDisassembler.hpp"
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
        uint32_t m_Hash;
        std::vector<InstructionEntry> m_Instructions;
        std::vector<ScriptDisassembler::FunctionInfo> m_Functions;

    public:
        ScriptLayout(const rage::scrProgram& program);

        void Refresh();

        const rage::scrProgram& GetProgram() const;
        const std::vector<uint8_t>& GetCode() const;
        const int GetInstructionCount() const;
        const int GetFunctionCount() const;
        const uint32_t GetHash() const;
        InstructionEntry GetInstruction(int index) const;
        ScriptDisassembler::FunctionInfo GetFunction(int index) const;
        int GetFunctionIndexForPc(uint32_t pc) const;
    };
}