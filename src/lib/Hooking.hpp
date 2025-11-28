#pragma once

namespace scrDbgLib
{
    class Hooking
    {
    public:
        struct Hook
        {
            std::uint32_t m_Hash;
            void* m_Target;
            void* m_Detour;
            void* m_Original = nullptr;
        };

        static bool Init();
        static void Destroy();

        template <typename T>
        static T GetOriginal(std::uint32_t hash)
        {
            for (auto& hook : m_Hooks)
            {
                if (hook.m_Hash == hash)
                    return reinterpret_cast<T>(hook.m_Original);
            }

            return nullptr;
        }

    private:
        template <typename T, typename D>
        static void AddHook(std::uint32_t hash, T target, D detour)
        {
            m_Hooks.push_back({hash, reinterpret_cast<void*>(target), reinterpret_cast<void*>(detour)});
        }

        static inline std::vector<Hook> m_Hooks;
    };
}