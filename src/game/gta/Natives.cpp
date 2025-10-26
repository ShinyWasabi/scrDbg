#include "Natives.hpp"
#include "Pointers.hpp"
#include "resource/scrDbg.h"

namespace gta
{
    bool Natives::InitImpl()
    {
        HMODULE module = GetModuleHandle(0);

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

        m_Names.reserve(count);
        m_Map.reserve(count);
        for (uint32_t i = 0; i < count && ptr < end; ++i)
        {
            if (end - ptr < 10)
                break;

            uint64_t hash;
            uint16_t len;
            memcpy(&hash, ptr, sizeof(hash)); ptr += sizeof(hash);
            memcpy(&len, ptr, sizeof(len)); ptr += sizeof(len);

            if (end - ptr < len)
                break;

            std::string name(ptr, len);
            ptr += len;

            m_Names.push_back(std::move(name));
            m_Map.emplace(hash, std::string_view(m_Names.back()));
        }

        return true;
    }

    uint64_t Natives::GetNativeHashByHandlerImpl(uint64_t handler)
    {
        if (!scrDbg::g_Pointers.NativeRegistrationTable || !handler)
            return 0;

        uint8_t buffer[0xC8]{}; // Enough to read it all I guess (scrNativeRegistration + hashes)
        for (int bucket = 0; bucket < 256; ++bucket)
        {
            uint64_t registration = scrDbg::Process::Read<uint64_t>(scrDbg::g_Pointers.NativeRegistrationTable + 8ULL * bucket);

            while (registration)
            {
                if (!scrDbg::Process::ReadRaw(registration, buffer, sizeof(buffer)))
                    break;

                uint32_t numEntries1 = *reinterpret_cast<uint32_t*>(buffer + 0x48);
                uint32_t numEntries2 = *reinterpret_cast<uint32_t*>(buffer + 0x4C);
                uint32_t numEntries = numEntries1 ^ static_cast<uint32_t>(registration + 0x48) ^ numEntries2;

                if (numEntries > 7)
                    numEntries = 7;

                auto handlers = reinterpret_cast<uint64_t*>(buffer + 0x10);
                for (uint32_t j = 0; j < numEntries; ++j)
                {
                    if (handlers[j] == handler)
                    {
                        uint64_t addr = registration + 0x54 + 16ULL * j;

                        uint32_t key = static_cast<uint32_t>(addr) ^ scrDbg::Process::Read<uint32_t>(addr + 8);
                        uint32_t low = key ^ scrDbg::Process::Read<uint32_t>(addr + 0);
                        uint32_t high = key ^ scrDbg::Process::Read<uint32_t>(addr + 4);
                        return (static_cast<uint64_t>(high) << 32) | low;
                    }
                }

                uint32_t key = static_cast<uint32_t>(registration) ^ *reinterpret_cast<uint32_t*>(buffer + 8);
                uint32_t low = key ^ *reinterpret_cast<uint32_t*>(buffer + 0);
                uint32_t high = key ^ *reinterpret_cast<uint32_t*>(buffer + 4);
                registration = (static_cast<uint64_t>(high) << 32) | low;
            }
        }

        return 0;
    }
}