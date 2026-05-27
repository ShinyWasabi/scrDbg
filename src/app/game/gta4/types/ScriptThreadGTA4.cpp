#include "ScriptThreadGTA4.hpp"

namespace scrDbgApp
{
    uint32_t ScriptThreadGTA4::GetId() const
    {
        return m_Base.Add(ID).Get<uint32_t>();
    }

    uint32_t ScriptThreadGTA4::GetProgramHash() const
    {
        return m_Base.Add(PROGRAM_HASH).Get<uint32_t>();
    }

    ScriptThread::State ScriptThreadGTA4::GetState() const
    {
        return m_Base.Add(STATE).Get<ScriptThread::State>();
    }

    void ScriptThreadGTA4::SetState(ScriptThread::State state) const
    {
        return m_Base.Add(STATE).Set<ScriptThread::State>(state);
    }

    uint32_t ScriptThreadGTA4::GetPc() const
    {
        return m_Base.Add(PC).Get<uint32_t>();
    }

    uint32_t ScriptThreadGTA4::GetFp() const
    {
        return m_Base.Add(FP).Get<uint32_t>();
    }

    uint32_t ScriptThreadGTA4::GetSp() const
    {
        return m_Base.Add(SP).Get<uint32_t>();
    }

    uint32_t ScriptThreadGTA4::GetStackSize() const
    {
        return m_Base.Add(STACK_SIZE).Get<uint32_t>();
    }

    int32_t ScriptThreadGTA4::GetStack(uint32_t index) const
    {
        return m_Base.Add(STACK).Deref32().GetArray<int32_t>(index);
    }

    void ScriptThreadGTA4::SetStack(uint32_t index, int32_t value) const
    {
        m_Base.Add(STACK).Deref32().SetArray<int32_t>(index, value);
    }

    std::string ScriptThreadGTA4::GetKillReason() const
    {
        return m_Base.Add(KILL_REASON).GetString(128);
    }

    uint32_t ScriptThreadGTA4::GetScriptHash() const
    {
        return GetProgramHash();
    }

    std::string ScriptThreadGTA4::GetScriptName() const
    {
        char buffer[24];
        m_Base.Add(SCRIPT_NAME).GetBuffer(buffer, sizeof(buffer));
        return std::string(buffer);
    }
}