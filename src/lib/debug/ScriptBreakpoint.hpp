#pragma once

namespace scrDbgLib
{
    class ScriptBreakpoint
    {
        static inline void (*m_PauseGameNow)();
        static inline void (*m_UnpauseGameNow)();
        static inline void (*m_TogglePausedRenderPhases)(bool, int);
        static inline std::uint8_t* m_PauseGameNowPatch;

    public:
        static inline std::optional<std::pair<std::uint32_t, std::uint32_t>> m_ActiveBreakpoint;
        static inline std::vector<std::pair<std::uint32_t, std::uint32_t>> m_Breakpoints;
        static inline bool m_BreakpointsPauseGame;
        static inline bool m_WasGamePausedByBreakpoint;
        static inline bool m_SkipThisBreakpointHit;

        static bool Init();

        static void PauseGame(bool pause);

        static bool Add(std::uint32_t script, std::uint32_t pc);
        static bool Remove(std::uint32_t script, std::uint32_t pc);
        static bool Exists(std::uint32_t script, std::uint32_t pc);
        static bool Resume();
        static void SetPauseGame(bool pause);
        static std::optional<std::pair<std::uint32_t, std::uint32_t>> GetActive();
        static std::vector<std::pair<std::uint32_t, std::uint32_t>> GetAll();
        static void RemoveAll();
    };
}