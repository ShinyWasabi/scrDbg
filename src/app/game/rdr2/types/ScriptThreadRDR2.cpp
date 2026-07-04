#include "ScriptThreadRDR2.hpp"

namespace scrDbgApp
{
    uint32_t ScriptThreadRDR2::GetId() const
    {
        return m_Base.Add(ID).Get<uint32_t>();
    }

    uint32_t ScriptThreadRDR2::GetProgramHash() const
    {
        return m_Base.Add(PROGRAM_HASH).Get<uint32_t>();
    }

    ScriptThread::State ScriptThreadRDR2::GetState() const
    {
        return m_Base.Add(STATE).Get<ScriptThread::State>();
    }

    void ScriptThreadRDR2::SetState(ScriptThread::State state) const
    {
        return m_Base.Add(STATE).Set<ScriptThread::State>(state);
    }

    uint32_t ScriptThreadRDR2::GetPc() const
    {
        return m_Base.Add(PC).Get<uint32_t>();
    }

    uint32_t ScriptThreadRDR2::GetFp() const
    {
        return m_Base.Add(FP).Get<uint32_t>();
    }

    uint32_t ScriptThreadRDR2::GetSp() const
    {
        return m_Base.Add(SP).Get<uint32_t>();
    }

    uint32_t ScriptThreadRDR2::GetStackSize() const
    {
        return m_Base.Add(STACK_SIZE).Get<uint32_t>();
    }

    bool ScriptThreadRDR2::IsPatched() const
    {
        return m_Base.Add(IS_PATCHED).Get<bool>();
    }

    Pointer ScriptThreadRDR2::GetStack(uint32_t index) const
    {
        return m_Base.Add(STACK).Deref().Add(4 * index);
    }

    std::string ScriptThreadRDR2::GetKillReason() const
    {
        return m_Base.Add(KILL_REASON).GetString(128);
    }

    uint32_t ScriptThreadRDR2::GetScriptHash() const
    {
        return GetProgramHash();
    }

    std::string ScriptThreadRDR2::GetScriptName() const
    {
        uint64_t base = m_Base.Add(TLS).GetArray<uint64_t>(0);
        if (base == 0)
            return {};

        std::string name = Pointer(base).Add(0x70).GetString(64);

        std::filesystem::path path(name);
        return path.stem().string();
    }
}