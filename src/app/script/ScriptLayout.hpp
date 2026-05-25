#pragma once
#include "ScriptDisassembler.hpp"

namespace scrDbgApp
{
    class ScriptLayout
    {
        struct InstructionEntry
        {
            uint32_t Pc;
            int StringIndex;
            int FuncIndex;
        };

        std::unique_ptr<ScriptProgram> m_Program;
        std::vector<uint8_t> m_Code;
        uint32_t m_Hash;
        std::vector<InstructionEntry> m_Instructions;
        std::vector<ScriptDisassembler::FunctionInfo> m_Functions;

    public:
        ScriptLayout(std::unique_ptr<ScriptProgram> program);

        void Refresh();

        const ScriptProgram* GetProgram() const;
        const std::vector<uint8_t>& GetCode() const;
        const int GetInstructionCount() const;
        const int GetFunctionCount() const;
        const uint32_t GetHash() const;
        InstructionEntry GetInstruction(int index) const;
        ScriptDisassembler::FunctionInfo GetFunction(int index) const;
        int GetFunctionIndexForPc(uint32_t pc) const;
    };
}