#include "DebuggerRDR3.hpp"
#include "game/RDR3.hpp"
#include "rage/rdr3/scrProgram.hpp"
#include "rage/rdr3/scrThread.hpp"

#if defined(_M_X64)

namespace scrDbgLib
{
    void DebuggerRDR3::PauseGame(bool pause)
    {
        auto& pointers = RDR3::GetPointers();
        if (!pointers.TimerUserPause || !pointers.TimerScriptPause)
            return;

        *pointers.TimerUserPause = pause;
        //*pointers.TimerScriptPause = pause;
    }

    bool DebuggerRDR3::ProcessBreakpoints(uint32_t scriptHash, uint32_t pc, uint32_t* state)
    {
        if (m_ActiveBreakpoint)
        {
            auto thread = rage::rdr3::scrThread::GetByHash(m_ActiveBreakpoint->ScriptHash);
            if (!thread || thread->m_Context.m_State == rage::rdr3::scrThread::State::KILLED)
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

            auto progHash = (*RDR3::GetPointers().CurrentScriptThread)->m_Context.m_ProgramHash;
            auto scriptName = rage::rdr3::scrProgram::GetByHash(progHash)->m_Name;

            // Show the message first so scrDbgApp doesn't attempt to resume when MessageBoxA is active
            char message[128];
            std::snprintf(message, sizeof(message), "Breakpoint hit, paused %s at 0x%X!", scriptName, pc);
            MessageBoxA(0, message, "Breakpoint", MB_OK);

            m_ActiveBreakpoint = bp;

            if (m_PauseGameOnBreakpoint)
            {
                PauseGame(true);
                m_GamePausedByBreakpoint = true;
            }
            else
            {
                *state = static_cast<uint32_t>(rage::rdr3::scrThread::State::PAUSED);
            }

            return true; // Return here to signal the VM that a BP is active, no need to check for others as there can only be one active at a time
        }

        return false;
    }

    bool DebuggerRDR3::ResumeBreakpoint()
    {
        if (!m_ActiveBreakpoint)
            return false;

        auto thread = rage::rdr3::scrThread::GetByHash(m_ActiveBreakpoint->ScriptHash);
        if (!thread)
            return false;

        if (m_GamePausedByBreakpoint)
        {
            PauseGame(false);
            m_GamePausedByBreakpoint = false;
        }
        else
        {
            thread->m_Context.m_State = rage::rdr3::scrThread::State::RUNNING;
        }

        m_StepOverBreakpoint = true;
        m_ActiveBreakpoint.reset();
        return true;
    }

    bool DebuggerRDR3::IsChainOpcode(uint8_t op) const
    {
        switch (static_cast<rage::rdr3::scrOpcode>(op))
        {
        // continuation opcodes
        case rage::rdr3::scrOpcode::IOFFSET:
        case rage::rdr3::scrOpcode::IOFFSET_U8:
        case rage::rdr3::scrOpcode::IOFFSET_S16:
        case rage::rdr3::scrOpcode::ARRAY_U8:
        case rage::rdr3::scrOpcode::ARRAY_U16:
        case rage::rdr3::scrOpcode::PUSH_CONST_S16:
        case rage::rdr3::scrOpcode::PUSH_CONST_U24:

        // finalizer opcodes
        case rage::rdr3::scrOpcode::STORE:
        case rage::rdr3::scrOpcode::STORE_N:
        case rage::rdr3::scrOpcode::IOFFSET_U8_STORE:
        case rage::rdr3::scrOpcode::IOFFSET_S16_STORE:
        case rage::rdr3::scrOpcode::ARRAY_U8_STORE:
        case rage::rdr3::scrOpcode::ARRAY_U16_STORE:
        case rage::rdr3::scrOpcode::LOCAL_STORE_S:
        case rage::rdr3::scrOpcode::LOCAL_STORE_SR:
        case rage::rdr3::scrOpcode::STATIC_STORE_S:
        case rage::rdr3::scrOpcode::STATIC_STORE_SR:
        case rage::rdr3::scrOpcode::GLOBAL_STORE_S:
        case rage::rdr3::scrOpcode::GLOBAL_STORE_SR:
        case rage::rdr3::scrOpcode::STORE_N_S:
        case rage::rdr3::scrOpcode::STORE_N_SR:
            return true;
        }

        // everything else breaks the chain
        return false;
    }
}

#endif