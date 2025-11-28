#include "ScriptBreakpoint.hpp"
#include "Pointers.hpp"
#include "core/Memory.hpp"
#include "rage/enhanced/scrThread.hpp"
#include "rage/legacy/scrThread.hpp"

namespace scrDbgLib
{
    void ScriptBreakpoint::PauseGame(bool pause)
    {
        if (pause)
        {
            g_IsEnhanced ? * g_Pointers.PauseGameNowPatch = 0xEB : *reinterpret_cast<std::uint16_t*>(g_Pointers.PauseGameNowPatch) = 0x9090;
            g_Pointers.PauseGameNow();
        }
        else
        {
            g_Pointers.UnpauseGameNow();
            g_Pointers.TogglePausedRenderPhases(true, 0);
            g_IsEnhanced ? * g_Pointers.PauseGameNowPatch = 0x74 : *reinterpret_cast<std::uint16_t*>(g_Pointers.PauseGameNowPatch) = 0x0E75;
        }
    }

    bool ScriptBreakpoint::Add(std::uint32_t script, std::uint32_t pc)
    {
        if (Exists(script, pc))
            return false;

        m_Breakpoints.push_back({script, pc});
        return true;
    }

    bool ScriptBreakpoint::Remove(std::uint32_t script, std::uint32_t pc)
    {
        auto it = std::find_if(m_Breakpoints.begin(), m_Breakpoints.end(), [script, pc](const std::pair<std::uint32_t, std::uint32_t>& bp) { return bp.first == script && bp.second == pc; });

        if (it == m_Breakpoints.end())
            return false;

        if (m_ActiveBreakpoint.has_value() && m_ActiveBreakpoint->first == script && m_ActiveBreakpoint->second == pc)
            Resume();

        m_Breakpoints.erase(it);
        return true;
    }

    bool ScriptBreakpoint::Exists(std::uint32_t script, std::uint32_t pc)
    {
        return std::any_of(m_Breakpoints.begin(), m_Breakpoints.end(), [script, pc](const std::pair<std::uint32_t, std::uint32_t>& bp) { return bp.first == script && bp.second == pc; });
    }

    bool ScriptBreakpoint::Resume()
    {
        return g_IsEnhanced ? rage::enhanced::scrThread::ResumeActiveBreakpoint() : rage::legacy::scrThread::ResumeActiveBreakpoint();
    }

    void ScriptBreakpoint::SetPauseGame(bool pause)
    {
        m_BreakpointsPauseGame = pause;
    }

    std::optional<std::pair<std::uint32_t, std::uint32_t>> ScriptBreakpoint::GetActive()
    {
        return m_ActiveBreakpoint;
    }

    std::vector<std::pair<std::uint32_t, std::uint32_t>> ScriptBreakpoint::GetAll()
    {
        return m_Breakpoints;
    }

    void ScriptBreakpoint::RemoveAll()
    {
        Resume();
        m_Breakpoints.clear();
    }
}