#include "DebuggerRDR2.hpp"
#include "game/RDR2.hpp"
#include "rage/rdr2/scrThread.hpp"

#if defined(_M_X64)

namespace scrDbgLib
{
    void DebuggerRDR2::PauseGame(bool pause)
    {
        auto& pointers = RDR2::GetPointers();
        if (!pointers.PauseGameFlags)
            return;

        *pointers.PauseGameFlags = pause ? 4 : 3; // TO-DO: use something else, this isn't reliable
    }

    bool DebuggerRDR2::ProcessBreakpoints(uint32_t scriptHash, uint32_t pc, uint32_t* state)
    {
        if (m_ActiveBreakpoint)
        {
            auto thread = rage::rdr2::scrThread::GetByHash(m_ActiveBreakpoint->ScriptHash);
            if (!thread || thread->m_Context.m_State == rage::rdr2::scrThread::State::KILLED)
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

            auto thread = *RDR2::GetPointers().CurrentScriptThread;

            const char* scriptName = "???";
            if (thread->m_Context.m_Tls[0].Any != 0)
                scriptName = reinterpret_cast<const char*>(thread->m_Context.m_Tls[0].Any + 112);

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
                *state = static_cast<uint32_t>(rage::rdr2::scrThread::State::PAUSED);
            }

            return true; // Return here to signal the VM that a BP is active, no need to check for others as there can only be one active at a time
        }

        return false;
    }

    bool DebuggerRDR2::ResumeBreakpoint()
    {
        if (!m_ActiveBreakpoint)
            return false;

        auto thread = rage::rdr2::scrThread::GetByHash(m_ActiveBreakpoint->ScriptHash);
        if (!thread)
            return false;

        if (m_GamePausedByBreakpoint)
        {
            PauseGame(false);
            m_GamePausedByBreakpoint = false;
        }
        else
        {
            thread->m_Context.m_State = rage::rdr2::scrThread::State::RUNNING;
        }

        m_StepOverBreakpoint = true;
        m_ActiveBreakpoint.reset();
        return true;
    }

    bool DebuggerRDR2::IsChainOpcode(uint8_t op) const
    {
        switch (static_cast<rage::rdr2::scrOpcode>(op))
        {
        // continuation opcodes
        case rage::rdr2::scrOpcode::IADD_U8:
        case rage::rdr2::scrOpcode::IADD_S16:
        case rage::rdr2::scrOpcode::ARRAY_U8:
        case rage::rdr2::scrOpcode::ARRAY_U16:
        case rage::rdr2::scrOpcode::PUSH_CONST_S16:
        case rage::rdr2::scrOpcode::PUSH_CONST_U24:

        // finalizer opcodes
        case rage::rdr2::scrOpcode::STORE:
        case rage::rdr2::scrOpcode::STORE_N:
        case rage::rdr2::scrOpcode::IOFFSET_U8_STORE:
        case rage::rdr2::scrOpcode::IOFFSET_S16_STORE:
        case rage::rdr2::scrOpcode::ARRAY_U8_STORE:
        case rage::rdr2::scrOpcode::ARRAY_U16_STORE:
        case rage::rdr2::scrOpcode::REF_STORE:
            return true;
        }

        // everything else breaks the chain
        return false;
    }
}

#endif