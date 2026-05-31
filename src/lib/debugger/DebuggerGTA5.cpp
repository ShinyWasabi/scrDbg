#include "DebuggerGTA5.hpp"
#include "game/GTA5.hpp"
#include "rage/gta5/scrThread.hpp"

#if defined(_M_X64)

namespace scrDbgLib
{
    void DebuggerGTA5::PauseGame(bool pause)
    {
        auto& pointers = GTA5::GetPointers();
        if (!pointers.TimerUserPause || !pointers.TimerScriptPause)
            return;

        *pointers.TimerUserPause = pause;
        *pointers.TimerScriptPause = pause;
    }

    template <typename T>
    bool DebuggerGTA5::ProcessBreakpointsInternal(uint32_t scriptHash, uint32_t pc, uint32_t* state)
    {
        if (m_ActiveBreakpoint)
        {
            auto thread = T::GetByHash(m_ActiveBreakpoint->ScriptHash);
            if (!thread || thread->m_Context.m_State == rage::scrThreadState::KILLED)
            {
                // Script died when the breakpoint was active
                m_StepOverBreakpoint = false;
                m_ActiveBreakpoint.reset();
            }
        }

        for (auto& bp : m_Breakpoints)
        {
            if (scriptHash != bp.ScriptHash || pc != bp.Pc)
                continue;

            if (m_StepOverBreakpoint)
            {
                m_StepOverBreakpoint = false;
                return false;
            }

            // Show the message first so scrDbgApp doesn't attempt to resume when MessageBoxA is active
            char message[128];
            std::snprintf(message, sizeof(message), "Breakpoint hit, paused %s at 0x%X!", (*T::GetTLS().m_CurrentThread)->m_ScriptName, pc);
            MessageBoxA(0, message, "Breakpoint", MB_OK);

            m_ActiveBreakpoint = bp;

            if (m_PauseGameOnBreakpoint)
            {
                PauseGame(true);
                m_GamePausedByBreakpoint = true;
            }
            else
            {
                *state = static_cast<uint32_t>(rage::scrThreadState::PAUSED);
            }

            return true; // Return here to signal the VM that a BP is active, no need to check for others as there can only be one active at a time
        }

        return false;
    }

    template <typename T>
    bool DebuggerGTA5::ResumeBreakpointInternal()
    {
        if (!m_ActiveBreakpoint)
            return false;

        auto thread = T::GetByHash(m_ActiveBreakpoint->ScriptHash);
        if (!thread)
            return false;

        if (m_GamePausedByBreakpoint)
        {
            PauseGame(false);
            m_GamePausedByBreakpoint = false;
        }
        else
        {
            thread->m_Context.m_State = rage::scrThreadState::RUNNING;
        }

        m_StepOverBreakpoint = true;
        m_ActiveBreakpoint.reset();
        return true;
    }

    bool DebuggerGTA5::IsChainOpcode(uint8_t op) const
    {
        switch (static_cast<rage::scrOpcode>(op))
        {
        // continuation opcodes
        case rage::scrOpcode::IOFFSET:
        case rage::scrOpcode::IOFFSET_U8:
        case rage::scrOpcode::IOFFSET_S16:
        case rage::scrOpcode::ARRAY_U8:
        case rage::scrOpcode::ARRAY_U16:
        case rage::scrOpcode::PUSH_CONST_S16:
        case rage::scrOpcode::PUSH_CONST_U24:

        // finalizer opcodes
        case rage::scrOpcode::STORE:
        case rage::scrOpcode::STORE_N:
        case rage::scrOpcode::IOFFSET_U8_STORE:
        case rage::scrOpcode::IOFFSET_S16_STORE:
        case rage::scrOpcode::ARRAY_U8_STORE:
        case rage::scrOpcode::ARRAY_U16_STORE:
            return true;
        }

        // everything else breaks the chain
        return false;
    }

    template bool DebuggerGTA5::ProcessBreakpointsInternal<rage::scrThreadGEN8>(uint32_t, uint32_t, uint32_t*);
    template bool DebuggerGTA5::ResumeBreakpointInternal<rage::scrThreadGEN8>();

    template bool DebuggerGTA5::ProcessBreakpointsInternal<rage::scrThreadGEN9>(uint32_t, uint32_t, uint32_t*);
    template bool DebuggerGTA5::ResumeBreakpointInternal<rage::scrThreadGEN9>();
}

#endif