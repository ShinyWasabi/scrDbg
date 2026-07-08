#pragma once
#include "rage/shared/scrNativeContext.hpp"

#if defined(_M_X64)

namespace rage::rdr3
{
    class scrNativeRegistration
    {
    public:
        scrNativeRegistration* m_Next;
        scrNativeContext::Handler m_Handlers[7];
        uint32_t m_Count;
        uint64_t m_Keys[7];

        scrNativeContext::Handler GetHandlerByHash(uint64_t hash, uint64_t seed) const
        {
            uint64_t key = ScrambleKey(hash, seed);

            for (auto node = this; node; node = node->m_Next)
            {
                for (uint32_t i = 0; i < node->m_Count; i++)
                {
                    if (node->m_Keys[i] == key)
                        return node->m_Handlers[i];
                }
            }

            return nullptr;
        }

        uint64_t GetHashByHandler(scrNativeContext::Handler handler, uint64_t seed) const
        {
            static std::unordered_map<scrNativeContext::Handler, uint64_t> cache;
            if (auto it = cache.find(handler); it != cache.end())
                return it->second;

            for (auto node = this; node; node = node->m_Next)
            {
                for (uint32_t i = 0; i < node->m_Count; i++)
                {
                    if (node->m_Handlers[i] == handler)
                    {
                        uint64_t hash = UnscrambleKey(node->m_Keys[i], seed);
                        cache[handler] = hash;
                        return hash;
                    }
                }
            }

            return 0;
        }

    private:
        static uint64_t RotL64(uint64_t value, int shift)
        {
            shift &= 63;
            return (value << shift) | (value >> (64 - shift));
        }

        static uint64_t RotR64(uint64_t value, int shift)
        {
            shift &= 63;
            return (value >> shift) | (value << (64 - shift));
        }

        static uint64_t ScrambleKey(uint64_t hash, uint64_t seed)
        {
            uint64_t v2 = RotL64(RotR64(seed, 5), 32);
            return v2 ^ RotR64(RotL64(~hash, 32), (v2 & 0x1F) + 1);
        }

        static uint64_t UnscrambleKey(uint64_t key, uint64_t seed)
        {
            uint64_t v2 = RotL64(RotR64(seed, 5), 32);
            uint64_t rotated = RotL64(key ^ v2, (v2 & 0x1F) + 1);
            return ~RotR64(rotated, 32);
        }
    };
}

#endif