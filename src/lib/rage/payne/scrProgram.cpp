#include "scrProgram.hpp"
#include "game/Payne.hpp"
#include "rage/shared/scrHash.hpp"

#if defined(_M_IX86)

namespace rage::payne
{
    using namespace scrDbgLib;

    std::string scrProgram::GetFuncName(uint32_t pc, uint32_t size) const
    {
        if (size > 0)
        {
            auto code = &m_Code[pc];

            std::string name(reinterpret_cast<const char*>(&code[5]));
            if (!name.empty())
                return name;
        }

        char name[32];
        std::snprintf(name, sizeof(name), "sub_%X", pc);
        return name;
    }

    scrProgram* scrProgram::GetProgram(uint32_t hash)
    {
        auto programs = Payne::GetPointers().ScriptPrograms;
        if (!programs)
            return nullptr;

        return programs->Lookup(hash);
    }
}

#endif