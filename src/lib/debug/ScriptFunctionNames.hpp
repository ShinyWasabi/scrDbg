#pragma once

namespace rage::shared
{
    class scrProgram;
}

namespace scrDbgLib
{
    struct ScriptFunctionNames
    {
    private:
        static inline std::unordered_map<rage::shared::scrProgram*, std::unordered_map<std::uint32_t, std::string>> m_FunctionNames;

    public:
        static void GenerateNamesForProgram(rage::shared::scrProgram* program);
        static void GenerateNamesForAllPrograms();

        static void RemoveProgram(rage::shared::scrProgram* program)
        {
            m_FunctionNames.erase(program);
        }

        static std::string* GetName(rage::shared::scrProgram* program, std::uint32_t pc)
        {
            auto prog = m_FunctionNames.find(program);
            if (prog == m_FunctionNames.end())
                return nullptr;

            auto it = prog->second.find(pc);
            if (it != prog->second.end())
                return &it->second;

            return nullptr;
        }
    };
}