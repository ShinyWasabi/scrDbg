#include "Natives.hpp"
#include "Pointers.hpp"

namespace gta::Natives
{
    uint64_t GetNativeHashByHandler(uint64_t handler)
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