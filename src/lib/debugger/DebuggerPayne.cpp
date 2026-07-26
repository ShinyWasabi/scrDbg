#if defined(_M_IX86)

#include "DebuggerPayne.hpp"
#include "game/Payne.hpp"
#include "rage/payne/scrThread.hpp"

namespace scrDbgLib
{
    void DebuggerPayne::PauseGame(bool pause)
    {
        auto& pointers = Payne::GetPointers();
        if (!pointers.TimerUserPause)
            return;

        *pointers.TimerUserPause = pause;
    }

    bool DebuggerPayne::ProcessBreakpoints(uint32_t scriptHash, uint32_t pc, uint32_t* state)
    {
        if (m_ActiveBreakpoint)
        {
            auto thread = rage::payne::scrThread::GetByHash(m_ActiveBreakpoint->ScriptHash);
            if (!thread || thread->m_Context.m_State == rage::payne::scrThread::State::KILLED)
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
            std::snprintf(message, sizeof(message), "Breakpoint hit, paused %s at 0x%X!", (*Payne::GetPointers().CurrentScriptThread)->m_ScriptName, pc);
            MessageBoxA(0, message, "Breakpoint", MB_OK);

            m_ActiveBreakpoint = bp;

            if (m_PauseGameOnBreakpoint)
            {
                PauseGame(true);
                m_GamePausedByBreakpoint = true;
            }
            else
            {
                *state = static_cast<uint32_t>(rage::payne::scrThread::State::PAUSED);
            }

            return true; // Return here to signal the VM that a BP is active, no need to check for others as there can only be one active at a time
        }

        return false;
    }

    bool DebuggerPayne::ResumeBreakpoint()
    {
        if (!m_ActiveBreakpoint)
            return false;

        auto thread = rage::payne::scrThread::GetByHash(m_ActiveBreakpoint->ScriptHash);
        if (!thread)
            return false;

        if (m_GamePausedByBreakpoint)
        {
            PauseGame(false);
            m_GamePausedByBreakpoint = false;
        }
        else
        {
            thread->m_Context.m_State = rage::payne::scrThread::State::RUNNING;
        }

        m_StepOverBreakpoint = true;
        m_ActiveBreakpoint.reset();
        return true;
    }

    std::string DebuggerPayne::NativeLogFormat(rage::scrValue value, NativeDB::Types type, void* thread, void* program, rage::scrValue* globals) const
    {
        auto _thread = reinterpret_cast<rage::payne::scrThread*>(thread);
        auto _program = reinterpret_cast<rage::payne::scrProgram*>(program);

        switch (type)
        {
        case NativeDB::Types::INT:
            return std::to_string(value.Int);
        case NativeDB::Types::BOOL:
            return value.Int ? "TRUE" : "FALSE";
        case NativeDB::Types::FLOAT:
            return std::to_string(value.Float);
        case NativeDB::Types::STRING:
        {
            auto str = reinterpret_cast<const char*>(_thread->ResolveAddress(&value, _program, globals));
            return str ? "\"" + std::string(str) + "\"" : "NULL";
        }
        case NativeDB::Types::REFERENCE:
        {
            auto ref = _thread->ResolveAddress(&value, _program, globals)->Reference;
            return "0x" + std::to_string(reinterpret_cast<uintptr_t>(ref));
        }
        }

        return std::to_string(value.Any);
    }

    std::unique_ptr<NativeContext> DebuggerPayne::CreateNativeContext() const
    {
        return std::make_unique<NativeContextPayne>();
    }

    bool DebuggerPayne::IsChainOpcode(uint8_t op) const
    {
        auto scrOp = static_cast<rage::payne::scrOpcode>(op);

        switch (scrOp)
        {
        // continuation opcodes
        case rage::payne::scrOpcode::PUSH_CONST_S16:
        case rage::payne::scrOpcode::PUSH_CONST_U32:
        case rage::payne::scrOpcode::IADD:
        case rage::payne::scrOpcode::ARRAY:

        // finalizer opcodes
        case rage::payne::scrOpcode::STORE:
        case rage::payne::scrOpcode::STORE_N:
            return true;
        }

        // continuation opcodes
        if (scrOp >= rage::payne::scrOpcode::PUSH_CONST_0 && scrOp <= rage::payne::scrOpcode::PUSH_CONST_159)
            return true;

        // everything else breaks the chain
        return false;
    }
}

#endif