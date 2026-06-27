#include "ScriptThreadPayne.hpp"

namespace scrDbgApp
{
    uint32_t ScriptThreadPayne::GetId() const
    {
        return m_Base.Add(ID).Get<uint32_t>();
    }

    uint32_t ScriptThreadPayne::GetProgramHash() const
    {
        return m_Base.Add(PROGRAM_HASH).Get<uint32_t>();
    }

    ScriptThread::State ScriptThreadPayne::GetState() const
    {
        return m_Base.Add(STATE).Get<ScriptThread::State>();
    }

    void ScriptThreadPayne::SetState(ScriptThread::State state) const
    {
        return m_Base.Add(STATE).Set<ScriptThread::State>(state);
    }

    uint32_t ScriptThreadPayne::GetPc() const
    {
        return m_Base.Add(PC).Get<uint32_t>();
    }

    uint32_t ScriptThreadPayne::GetFp() const
    {
        return m_Base.Add(FP).Get<uint32_t>();
    }

    uint32_t ScriptThreadPayne::GetSp() const
    {
        return m_Base.Add(SP).Get<uint32_t>();
    }

    uint32_t ScriptThreadPayne::GetStackSize() const
    {
        return m_Base.Add(STACK_SIZE).Get<uint32_t>();
    }

    uint32_t ScriptThreadPayne::GetTypedFlags() const
    {
        return m_Base.Add(TYPED_FLAGS).Get<uint32_t>();
    }

    Pointer ScriptThreadPayne::GetStack(uint32_t index) const
    {
        return m_Base.Add(STACK).Deref().Add(index * sizeof(uint32_t));
    }

    std::string ScriptThreadPayne::GetKillReason() const
    {
        return m_Base.Add(KILL_REASON).GetString(128);
    }

    uint32_t ScriptThreadPayne::GetScriptHash() const
    {
        return GetProgramHash();
    }

    std::string ScriptThreadPayne::GetScriptName() const
    {
        char buffer[24];
        m_Base.Add(SCRIPT_NAME).GetBuffer(buffer, sizeof(buffer));
        return std::string(buffer);
    }
}