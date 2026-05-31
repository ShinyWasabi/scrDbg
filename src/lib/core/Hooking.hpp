#pragma once

namespace scrDbgLib
{
    class Hooking
    {
    public:
        struct Hook
        {
            void* m_Target;
            void* m_Detour;
            void* m_Original = nullptr;
        };

        static bool Init();
        static void Destroy();

        template <typename T, typename D>
        static void AddHook(T target, D detour)
        {
            m_Hooks.push_back({reinterpret_cast<void*>(target), reinterpret_cast<void*>(detour)});
        }

        template <typename T>
        static T GetOriginal(T detour)
        {
            for (auto& hook : m_Hooks)
            {
                if (hook.m_Detour == reinterpret_cast<void*>(detour))
                    return reinterpret_cast<T>(hook.m_Original);
            }

            return nullptr;
        }

    private:
        static inline std::vector<Hook> m_Hooks;
    };
}