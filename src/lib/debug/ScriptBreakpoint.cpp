#include "ScriptBreakpoint.hpp"
#include "Pointers.hpp"
#include "core/Memory.hpp"
#include "rage/scrThread.hpp"

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

    bool ScriptBreakpoint::Process(uint32_t* hash, uint32_t* pc, rage::scrThreadState* state)
    {
        if (m_ActiveBreakpoint.has_value())
        {
            auto thread = rage::scrThread::GetThread(m_ActiveBreakpoint->first);
            if (!thread || *thread->Context()->State() == rage::scrThreadState::KILLED)
            {
                // Script died when the breakpoint was active
                m_SkipThisBreakpointHit = false;
                m_ActiveBreakpoint.reset();
            }
        }

        for (auto& bp : m_Breakpoints)
        {
            if (*hash != bp.first || *pc != bp.second)
                continue;

            if (m_SkipThisBreakpointHit)
            {
                m_SkipThisBreakpointHit = false;
                return false;
            }

            // Show the message first so scrDbg doesn't attempt to resume when MessageBoxA is active
            char message[128];
            std::snprintf(message, sizeof(message), "Breakpoint hit, paused %s at 0x%X!", rage::scrThread::GetCurrentThread()->ScriptName(), *pc);
            MessageBoxA(0, message, "Breakpoint", MB_OK);

            m_ActiveBreakpoint = bp;

            if (m_BreakpointsPauseGame)
            {
                PauseGame(true);
                m_WasGamePausedByBreakpoint = true;
            }
            else
            {
                *state = rage::scrThreadState::PAUSED;
            }

            return true; // Return here to signal the VM that a BP is active, no need to check for others as there can only be one active at a time.
        }

        return false;
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
        auto it = std::find_if(m_Breakpoints.begin(), m_Breakpoints.end(), [script, pc](const std::pair<std::uint32_t, std::uint32_t>& bp) {
            return bp.first == script && bp.second == pc;
        });

        if (it == m_Breakpoints.end())
            return false;

        if (m_ActiveBreakpoint.has_value() && m_ActiveBreakpoint->first == script && m_ActiveBreakpoint->second == pc)
            Resume();

        m_Breakpoints.erase(it);
        return true;
    }

    bool ScriptBreakpoint::Exists(std::uint32_t script, std::uint32_t pc)
    {
        return std::any_of(m_Breakpoints.begin(), m_Breakpoints.end(), [script, pc](const std::pair<std::uint32_t, std::uint32_t>& bp) {
            return bp.first == script && bp.second == pc;
        });
    }

    bool ScriptBreakpoint::Resume()
    {
        if (!m_ActiveBreakpoint.has_value())
            return false;

        auto thread = rage::scrThread::GetThread(m_ActiveBreakpoint->first);
        if (!thread)
            return false;

        if (m_WasGamePausedByBreakpoint)
        {
            PauseGame(false);
            m_WasGamePausedByBreakpoint = false;
        }
        else
        {
            *thread->Context()->State() = rage::scrThreadState::RUNNING;
        }

        m_SkipThisBreakpointHit = true;
        m_ActiveBreakpoint.reset();
        return true;
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