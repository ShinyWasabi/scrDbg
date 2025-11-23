#include "ResourceLoader.hpp"
#include "scrDbg.h"

namespace scrDbgShared
{
    bool NativesBin::Load(HMODULE module)
    {
        HRSRC res = FindResource(module, MAKEINTRESOURCE(NATIVES_BIN), RT_RCDATA);
        if (!res)
            return false;

        HGLOBAL data = LoadResource(module, res);
        if (!data)
            return false;

        DWORD size = SizeofResource(module, res);
        const char* ptr = static_cast<const char*>(LockResource(data));
        if (!ptr || size < sizeof(uint32_t))
            return false;

        const char* end = ptr + size;

        uint32_t count = 0;
        memcpy(&count, ptr, sizeof(count));
        ptr += sizeof(count);

        Names.reserve(count);
        Map.reserve(count);
        for (uint32_t i = 0; i < count && ptr < end; ++i)
        {
            if (end - ptr < 10)
                break;

            uint64_t hash;
            uint16_t len;
            memcpy(&hash, ptr, sizeof(hash));
            ptr += sizeof(hash);
            memcpy(&len, ptr, sizeof(len));
            ptr += sizeof(len);

            if (end - ptr < len)
                break;

            std::string name(ptr, len);
            ptr += len;

            Names.push_back(std::move(name));
            Map.emplace(hash, std::string_view(Names.back()));
        }

        return true;
    }
}