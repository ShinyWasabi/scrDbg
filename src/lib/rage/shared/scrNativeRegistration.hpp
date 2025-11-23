#pragma once
#include "scrNativeCallContext.hpp"

namespace rage::shared
{
    // https://www.unknowncheats.me/forum/3036413-post862.html

    class scrNativeRegistration
    {
    public:
        struct RegistrationNode
        {
            template <typename T>
            struct NodeEnc
            {
                std::uint32_t A;
                std::uint32_t B;
                std::uint32_t C;
                char Pad[0x04];

                T Decrypt() const
                {
                    const auto nonce = reinterpret_cast<std::uintptr_t>(this) ^ C;
                    const auto value = static_cast<std::uint32_t>(nonce ^ A) | (nonce ^ B) << 32U;
                    if constexpr (std::is_integral_v<T>)
                        return value;
                    else
                        return reinterpret_cast<T>(value);
                }
            };

            NodeEnc<RegistrationNode*> Next;
            scrNativeHandler Handlers[7];
            std::uint32_t numEntries1;
            std::uint32_t numEntries2;
            char Pad[0x04];
            NodeEnc<std::uint64_t> Hashes[7];

            std::uint32_t GetCount() const
            {
                return numEntries1 ^ numEntries2 ^ static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&numEntries1));
            }
        };

        RegistrationNode* m_Nodes[256];

        scrNativeHandler GetHandlerByHash(std::uint64_t hash) const
        {
            for (auto node = m_Nodes[hash & 255]; node; node = node->Next.Decrypt())
            {
                const auto count = node->GetCount();
                for (std::uint32_t i = 0; i < count; i++)
                {
                    if (node->Hashes[i].Decrypt() == hash)
                        return node->Handlers[i];
                }
            }

            return nullptr;
        }

        std::uint64_t GetHashByHandler(scrNativeHandler handler) const
        {
            static std::unordered_map<scrNativeHandler, std::uint64_t> cache;

            if (auto it = cache.find(handler); it != cache.end())
                return it->second;

            std::uint64_t hash = 0;
            for (int i = 0; i < 256; i++)
            {
                for (auto node = m_Nodes[i]; node; node = node->Next.Decrypt())
                {
                    const auto count = node->GetCount();
                    for (std::uint32_t j = 0; j < count; j++)
                    {
                        if (node->Handlers[j] == handler)
                        {
                            hash = node->Hashes[j].Decrypt();
                            break;
                        }
                    }
                }
            }

            cache[handler] = hash;
            return hash;
        }

        static bool Init();

        static inline scrNativeRegistration* m_NativeRegistrationTable;
    };
}