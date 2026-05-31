#include "scrProgram.hpp"
#include "game/GTA4.hpp"
#include "scrHash.hpp"

#if defined(_M_IX86)

namespace rage
{
    using namespace scrDbgLib;

    std::string scrProgram::GetFuncName(uint32_t pc) const
    {
        char name[32];
        std::snprintf(name, sizeof(name), "sub_%X", pc);
        return name;
    }

    scrProgram* scrProgram::GetProgram(uint32_t hash)
    {
        auto programs = GTA4::GetPointers().ScriptPrograms;
        if (!programs)
            return nullptr;

        return programs->Lookup(hash);
    }
}

#endif