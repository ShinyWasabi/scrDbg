#pragma once

namespace rage
{
    class scrProgram;
}

namespace scrDbgLib
{
    class ScriptFunctionNames
    {
    public:
        static void RemoveProgram(rage::scrProgram* program)
        {
            m_FunctionNames.erase(program);
        }

        static std::string* GetName(rage::scrProgram* program, std::uint32_t pc)
        {
            auto prog = m_FunctionNames.find(program);
            if (prog == m_FunctionNames.end())
                return nullptr;

            auto it = prog->second.find(pc);
            if (it != prog->second.end())
                return &it->second;

            return nullptr;
        }

        static void GenerateNamesForProgram(rage::scrProgram* program);
        static void GenerateNamesForAllPrograms();

    private:
        static inline std::unordered_map<rage::scrProgram*, std::unordered_map<std::uint32_t, std::string>> m_FunctionNames;
    };
}