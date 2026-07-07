#include "scrProgram.hpp"
#include "core/Hooking.hpp"
#include "game/RDR3.hpp"

#if defined(_M_X64)

namespace rage::rdr3
{
    using namespace scrDbgLib;

    std::string scrProgram::GetFuncName(uint32_t pc, uint32_t size) const
    {
        if (size > 0)
        {
            auto code = GetCode(pc);

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
        return RDR3::GetPointers().ScriptProgramRegistry->Find(hash);
    }
}

#endif