#include "ScriptFunctionNames.hpp"
#include "Pointers.hpp"
#include "rage/shared/scrOpcode.hpp"
#include "rage/shared/scrProgram.hpp"

namespace scrDbgLib
{
    void ScriptFunctionNames::GenerateNamesForProgram(rage::shared::scrProgram* program)
    {
        if (program && m_FunctionNames.find(program) != m_FunctionNames.end())
            return;

        std::unordered_map<std::uint32_t, std::string> map;

        std::uint32_t pc = 0;
        while (pc < program->m_CodeSize)
        {
            auto code = program->GetCode(pc);
            if (!code)
                break;

            if (*code == rage::shared::scrOpcode::ENTER)
            {
                std::string name;
                if (code[4] > 0)
                    name = std::string(reinterpret_cast<const char*>(&code[7]));
                else
                    name = "func_" + std::to_string(map.size());

                map.emplace(pc, std::move(name));
            }

            pc += rage::shared::GetInstructionSize(code);
        }

        m_FunctionNames.emplace(program, std::move(map));
    }

    void ScriptFunctionNames::GenerateNamesForAllPrograms()
    {
        for (int i = 0; i < 176; i++)
        {
            if (g_Pointers.ScriptPrograms[i])
                GenerateNamesForProgram(g_Pointers.ScriptPrograms[i]);
        }
    }
}