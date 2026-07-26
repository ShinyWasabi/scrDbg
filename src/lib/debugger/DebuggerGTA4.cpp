#if defined(_M_IX86)

#include "DebuggerGTA4.hpp"
#include "game/GTA4.hpp"
#include "rage/gta4/scrThread.hpp"

namespace scrDbgLib
{
    void DebuggerGTA4::PauseGame(bool pause)
    {
        auto& pointers = GTA4::GetPointers();
        if (!pointers.TimerUserPause || !pointers.TimerScriptPause)
            return;

        *pointers.TimerUserPause = pause;
        *pointers.TimerScriptPause = pause;
    }

    bool DebuggerGTA4::ProcessBreakpoints(uint32_t scriptHash, uint32_t pc, uint32_t* state)
    {
        if (m_ActiveBreakpoint)
        {
            auto thread = rage::gta4::scrThread::GetByHash(m_ActiveBreakpoint->ScriptHash);
            if (!thread || thread->m_Context.m_State == rage::gta4::scrThread::State::KILLED)
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
            std::snprintf(message, sizeof(message), "Breakpoint hit, paused %s at 0x%X!", (*GTA4::GetPointers().CurrentScriptThread)->m_ScriptName, pc);
            MessageBoxA(0, message, "Breakpoint", MB_OK);

            m_ActiveBreakpoint = bp;

            if (m_PauseGameOnBreakpoint)
            {
                PauseGame(true);
                m_GamePausedByBreakpoint = true;
            }
            else
            {
                *state = static_cast<uint32_t>(rage::gta4::scrThread::State::PAUSED);
            }

            return true; // Return here to signal the VM that a BP is active, no need to check for others as there can only be one active at a time
        }

        return false;
    }

    bool DebuggerGTA4::ResumeBreakpoint()
    {
        if (!m_ActiveBreakpoint)
            return false;

        auto thread = rage::gta4::scrThread::GetByHash(m_ActiveBreakpoint->ScriptHash);
        if (!thread)
            return false;

        if (m_GamePausedByBreakpoint)
        {
            PauseGame(false);
            m_GamePausedByBreakpoint = false;
        }
        else
        {
            thread->m_Context.m_State = rage::gta4::scrThread::State::RUNNING;
        }

        m_StepOverBreakpoint = true;
        m_ActiveBreakpoint.reset();
        return true;
    }

    bool DebuggerGTA4::IsChainOpcode(uint8_t op) const
    {
        auto scrOp = static_cast<rage::gta4::scrOpcode>(op);

        switch (scrOp)
        {
        // continuation opcodes
        case rage::gta4::scrOpcode::PUSH_CONST_S16:
        case rage::gta4::scrOpcode::PUSH_CONST_U32:
        case rage::gta4::scrOpcode::IADD:
        case rage::gta4::scrOpcode::ARRAY:

        // finalizer opcodes
        case rage::gta4::scrOpcode::STORE:
        case rage::gta4::scrOpcode::ARRAY_STORE:
        case rage::gta4::scrOpcode::SETXPROTECT:
            return true;
        }

        // continuation opcodes
        if (scrOp >= rage::gta4::scrOpcode::PUSH_CONST_0 && scrOp <= rage::gta4::scrOpcode::PUSH_CONST_159)
            return true;

        // everything else breaks the chain
        return false;
    }
}

#endif