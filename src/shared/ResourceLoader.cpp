#include "ResourceLoader.hpp"
#include "scrDbg.h"

namespace scrDbgShared
{
    bool NativesBin::Load(HMODULE module, int id)
    {
        HRSRC res = FindResource(module, MAKEINTRESOURCE(id), RT_RCDATA);
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

        Names.clear();
        Args.clear();
        Rets.clear();

        for (uint32_t i = 0; i < count && ptr < end; ++i)
        {
            if (end - ptr < sizeof(uint64_t) + sizeof(uint16_t))
                break;

            uint64_t hash;
            uint16_t nameLen;
            memcpy(&hash, ptr, sizeof(hash));
            ptr += sizeof(hash);
            memcpy(&nameLen, ptr, sizeof(nameLen));
            ptr += sizeof(nameLen);

            if (end - ptr < nameLen)
                break;

            std::string name(ptr, nameLen);
            ptr += nameLen;

            if (end - ptr < sizeof(uint16_t))
                break;
            uint16_t argCount;
            memcpy(&argCount, ptr, sizeof(argCount));
            ptr += sizeof(argCount);

            std::vector<NativeTypes> args;
            args.reserve(argCount);
            for (uint16_t j = 0; j < argCount && ptr < end; ++j)
            {
                if (end - ptr < 1)
                    break;
                args.push_back(static_cast<NativeTypes>(*ptr));
                ++ptr;
            }

            if (end - ptr < sizeof(uint16_t))
                break;
            uint16_t retCount;
            memcpy(&retCount, ptr, sizeof(retCount));
            ptr += sizeof(retCount);

            std::vector<NativeTypes> rets;
            rets.reserve(retCount);
            for (uint16_t j = 0; j < retCount && ptr < end; ++j)
            {
                if (end - ptr < 1)
                    break;
                rets.push_back(static_cast<NativeTypes>(*ptr));
                ++ptr;
            }

            Names.emplace(hash, std::move(name));
            Args.emplace(hash, std::move(args));
            Rets.emplace(hash, std::move(rets));
        }

        return true;
    }
}