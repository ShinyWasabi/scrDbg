#include "scrProgram.hpp"
#include "Hooking.hpp"
#include "Joaat.hpp"
#include "Pointers.hpp"
#include "debug/ScriptFunctionNames.hpp"

namespace rage
{
    scrProgram* scrProgram::GetProgram(std::uint32_t hash)
    {
        for (int i = 0; i < 176; i++)
        {
            if (scrDbgLib::g_Pointers.ScriptPrograms[i] && scrDbgLib::g_Pointers.ScriptPrograms[i]->m_NameHash == hash)
                return scrDbgLib::g_Pointers.ScriptPrograms[i];
        }

        return nullptr;
    }

    void scrProgram::scrProgramConstructor(scrProgram* program, std::uint8_t a1)
    {
        scrDbgLib::Hooking::GetOriginal<decltype(&scrProgramConstructor)>("scrProgramConstructor"_J)(program, a1);

        scrDbgLib::ScriptFunctionNames::GenerateNamesForProgram(program);
    }

    void scrProgram::scrProgramDestructor(scrProgram* program)
    {
        scrDbgLib::ScriptFunctionNames::RemoveProgram(program);

        scrDbgLib::Hooking::GetOriginal<decltype(&scrProgramDestructor)>("ScriptProgramDestructor"_J)(program);
    }

    std::uint32_t scrProgram::LoadScriptProgramSCO(const char* path, const char* scriptName)
    {
        auto hash = scrDbgLib::Hooking::GetOriginal<decltype(&LoadScriptProgramSCO)>("LoadScriptProgramSCO"_J)(path, scriptName);

        if (auto program = GetProgram(hash))
            scrDbgLib::ScriptFunctionNames::GenerateNamesForProgram(program);

        return hash;
    }
}