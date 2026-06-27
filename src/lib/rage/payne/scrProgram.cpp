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
            std::string name(reinterpret_cast<const char*>(&m_Code[pc + 7])); // 7 instead of 5 to skip the profiler placeholders (MP3 uses � (FF) unlike GTA 5 which uses _)
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