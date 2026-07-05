#include "ScriptThreadRDR3.hpp"

namespace scrDbgApp
{
    uint32_t ScriptThreadRDR3::GetId() const
    {
        return m_Base.Add(ID).Get<uint32_t>();
    }

    uint32_t ScriptThreadRDR3::GetProgramHash() const
    {
        return m_Base.Add(PROGRAM_HASH).Get<uint32_t>();
    }

    ScriptThread::State ScriptThreadRDR3::GetState() const
    {
        return m_Base.Add(STATE).Get<ScriptThread::State>();
    }

    void ScriptThreadRDR3::SetState(ScriptThread::State state) const
    {
        return m_Base.Add(STATE).Set<ScriptThread::State>(state);
    }

    uint32_t ScriptThreadRDR3::GetPc() const
    {
        return m_Base.Add(PC).Get<uint32_t>();
    }

    uint32_t ScriptThreadRDR3::GetFp() const
    {
        return m_Base.Add(FP).Get<uint32_t>();
    }

    uint32_t ScriptThreadRDR3::GetSp() const
    {
        return m_Base.Add(SP).Get<uint32_t>();
    }

    uint32_t ScriptThreadRDR3::GetStackSize() const
    {
        return m_Base.Add(STACK_SIZE).Get<uint32_t>();
    }

    ScriptThread::Priority ScriptThreadRDR3::GetPriority() const
    {
        return m_Base.Add(PRIORITY).Get<ScriptThread::Priority>();
    }

    uint8_t ScriptThreadRDR3::GetCallDepth() const
    {
        return m_Base.Add(CALL_DEPTH).Get<uint8_t>();
    }

    uint32_t ScriptThreadRDR3::GetCallStack(uint32_t index) const
    {
        return m_Base.Add(CALL_STACK).GetArray<uint32_t>(index);
    }

    Pointer ScriptThreadRDR3::GetStack(uint32_t index) const
    {
        return m_Base.Add(STACK).Deref().Add(index * sizeof(uint64_t));
    }

    std::string ScriptThreadRDR3::GetKillReason() const
    {
        return m_Base.Add(KILL_REASON).GetString(128);
    }

    uint32_t ScriptThreadRDR3::GetScriptHash() const
    {
        return m_Base.Add(SCRIPT_HASH).Get<uint32_t>();
    }

    // This is not good, but RDR2 doesn't store the script name in scrThread, so yeah
    std::string ScriptThreadRDR3::GetScriptName() const
    {
        auto prog = g_Game->GetProgram(GetProgramHash());
        if (!prog)
            return {};

        return prog->GetName();
    }
}