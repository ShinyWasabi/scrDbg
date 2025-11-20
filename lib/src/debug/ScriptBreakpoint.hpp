#pragma once

class ScriptBreakpoint
{
public:
    static inline std::optional<std::pair<std::uint32_t, std::uint32_t>> m_ActiveBreakpoint;
    static inline std::vector<std::pair<std::uint32_t, std::uint32_t>> m_Breakpoints;
    static inline bool m_BreakpointsPauseGame;
    static inline bool m_WasGamePausedByBreakpoint;
    static inline bool m_SkipThisBreakpointHit;

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