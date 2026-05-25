#include "ScriptThreadGTA5_GEN9.hpp"

namespace scrDbgApp
{
    uint32_t ScriptThreadGTA5_GEN9::GetId() const
    {
        return m_Base.Add(ID).Get<uint32_t>();
    }

    uint32_t ScriptThreadGTA5_GEN9::GetProgramHash() const
    {
        return m_Base.Add(PROGRAM_HASH).Get<uint32_t>();
    }

    ScriptThread::State ScriptThreadGTA5_GEN9::GetState() const
    {
        return m_Base.Add(STATE).Get<ScriptThread::State>();
    }

    void ScriptThreadGTA5_GEN9::SetState(ScriptThread::State state) const
    {
        return m_Base.Add(STATE).Set<ScriptThread::State>(state);
    }

    uint32_t ScriptThreadGTA5_GEN9::GetPc() const
    {
        return m_Base.Add(PC).Get<uint32_t>();
    }

    uint32_t ScriptThreadGTA5_GEN9::GetFp() const
    {
        return m_Base.Add(FP).Get<uint32_t>();
    }

    uint32_t ScriptThreadGTA5_GEN9::GetSp() const
    {
        return m_Base.Add(SP).Get<uint32_t>();
    }

    uint32_t ScriptThreadGTA5_GEN9::GetStackSize() const
    {
        return m_Base.Add(STACK_SIZE).Get<uint32_t>();
    }

    ScriptThread::Priority ScriptThreadGTA5_GEN9::GetPriority() const
    {
        return m_Base.Add(PRIORITY).Get<ScriptThread::Priority>();
    }

    uint8_t ScriptThreadGTA5_GEN9::GetCallDepth() const
    {
        return m_Base.Add(CALL_DEPTH).Get<uint8_t>();
    }

    uint32_t ScriptThreadGTA5_GEN9::GetCallStack(uint32_t index) const
    {
        return m_Base.Add(CALL_STACK).GetArray<uint32_t>(index);
    }

    int32_t ScriptThreadGTA5_GEN9::GetStack(uint32_t index) const
    {
        return m_Base.Add(STACK).Deref().GetArray<int32_t>(index);
    }

    void ScriptThreadGTA5_GEN9::SetStack(uint32_t index, int32_t value) const
    {
        m_Base.Add(STACK).Deref().SetArray<int32_t>(index, value);
    }

    std::string ScriptThreadGTA5_GEN9::GetCreateTime() const
    {
        uint32_t createTime = m_Base.Add(CREATE_TIME).Get<uint32_t>();

        uint32_t now = timeGetTime();
        uint32_t ms = now - createTime;

        uint32_t totalSeconds = ms / 1000;

        uint32_t hours = totalSeconds / 3600;
        totalSeconds %= 3600;

        uint32_t minutes = totalSeconds / 60;
        uint32_t seconds = totalSeconds % 60;

        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
        return std::string(buffer);
    }

    std::string ScriptThreadGTA5_GEN9::GetExitReason() const
    {
        char buffer[128];
        m_Base.Add(EXIT_REASON).GetBuffer(buffer, 128);
        return std::string(buffer);
    }

    uint32_t ScriptThreadGTA5_GEN9::GetScriptHash() const
    {
        return m_Base.Add(SCRIPT_HASH).Get<uint32_t>();
    }

    std::string ScriptThreadGTA5_GEN9::GetScriptName() const
    {
        char buffer[64];
        m_Base.Add(SCRIPT_NAME).GetBuffer(buffer, 64);
        return std::string(buffer);
    }
}