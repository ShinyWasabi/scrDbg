#pragma once
#include "rage/shared/scrNativeContext.hpp"

#if defined(_M_X64)

namespace rage
{
    class scrNativeRegistration
    {
    public:
        struct RegistrationNode
        {
            template <typename T>
            struct NodeEnc
            {
                uint32_t m_A;
                uint32_t m_B;
                uint32_t m_C;
                char m_Pad1[0x04];

                T Decrypt() const
                {
                    const auto nonce = reinterpret_cast<uintptr_t>(this) ^ m_C;
                    const auto value = static_cast<uint32_t>(nonce ^ m_A) | (nonce ^ m_B) << 32U;
                    if constexpr (std::is_integral_v<T>)
                        return value;
                    else
                        return reinterpret_cast<T>(value);
                }
            };

            NodeEnc<RegistrationNode*> m_Next;
            scrNativeContext::Handler m_Handlers[7];
            uint32_t numEntries1;
            uint32_t numEntries2;
            char m_Pad1[0x04];
            NodeEnc<uint64_t> m_Hashes[7];
            char m_Pad2[0x38]; // rest is unused

            uint32_t GetCount() const
            {
                return numEntries1 ^ numEntries2 ^ static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&numEntries1));
            }
        };

        scrNativeContext::Handler GetHandlerByHash(uint64_t hash) const
        {
            for (auto node = m_Nodes[hash & 255]; node; node = node->m_Next.Decrypt())
            {
                const auto count = node->GetCount();
                for (uint32_t i = 0; i < count; i++)
                {
                    if (node->m_Hashes[i].Decrypt() == hash)
                        return node->m_Handlers[i];
                }
            }

            return nullptr;
        }

        uint64_t GetHashByHandler(scrNativeContext::Handler handler) const
        {
            static std::unordered_map<scrNativeContext::Handler, uint64_t> cache;

            if (auto it = cache.find(handler); it != cache.end())
                return it->second;

            uint64_t hash = 0;
            for (int i = 0; i < 256; i++)
            {
                for (auto node = m_Nodes[i]; node; node = node->m_Next.Decrypt())
                {
                    const auto count = node->GetCount();
                    for (uint32_t j = 0; j < count; j++)
                    {
                        if (node->m_Handlers[j] == handler)
                        {
                            hash = node->m_Hashes[j].Decrypt();
                            break;
                        }
                    }
                }
            }

            cache[handler] = hash;
            return hash;
        }

        RegistrationNode* m_Nodes[256];
        uint32_t m_Unk; // unused
        bool m_Initialized;
        char m_Pad1[0x03];
    };
}

#endif