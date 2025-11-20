#include "Misc.hpp"
#include "Pointers.hpp"

namespace scrDbg::Misc
{
    std::string GetGameBuild()
    {
        if (!g_Pointers.GameBuild || !g_Pointers.OnlineBuild)
            return {};

        char gameBuild[64]{};
        Process::ReadRaw(g_Pointers.GameBuild, gameBuild, sizeof(gameBuild));

        char onlineBuild[64]{};
        Process::ReadRaw(g_Pointers.OnlineBuild, onlineBuild, sizeof(onlineBuild));

        char result[128]{};
        std::snprintf(result, sizeof(result), "%s-%s", gameBuild, onlineBuild);
        return result;
    }
}