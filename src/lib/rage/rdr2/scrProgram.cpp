#include "scrProgram.hpp"
#include "game/RDR2.hpp"
#include "rage/shared/scrHash.hpp"

#if defined(_M_X64)

namespace rage::rdr2
{
    using namespace scrDbgLib;

    std::string scrProgram::GetFuncName(uint32_t pc, uint32_t size) const
    {
        if (size > 0)
        {
            auto code = m_Data->GetCode(pc);

            std::string name(reinterpret_cast<const char*>(&code[7])); // 7 instead of 5 to skip the profiler placeholders
            if (!name.empty())
                return name;
        }

        char name[32];
        std::snprintf(name, sizeof(name), "sub_%X", pc);
        return name;
    }

    scrProgram* scrProgram::GetByHash(uint32_t hash)
    {
        auto programs = RDR2::GetPointers().ScriptPrograms;
        if (!programs)
            return nullptr;

        return programs->Lookup(hash);
    }
}

#endif