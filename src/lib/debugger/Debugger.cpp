#include "Debugger.hpp"
#include "VMLogger.hpp"

namespace scrDbgLib
{
    void Debugger::SetBreakpoint(uint32_t scriptHash, uint32_t pc, bool set)
    {
        auto it = std::find_if(m_Breakpoints.begin(), m_Breakpoints.end(), [&](const Breakpoint& bp) {
            return bp.ScriptHash == scriptHash && bp.Pc == pc;
        });

        if (set && it == m_Breakpoints.end())
        {
            m_Breakpoints.push_back({scriptHash, pc});
        }
        else if (!set && it != m_Breakpoints.end())
        {
            if (m_ActiveBreakpoint && m_ActiveBreakpoint->ScriptHash == scriptHash && m_ActiveBreakpoint->Pc == pc)
                ResumeBreakpoint();

            m_Breakpoints.erase(it);
        }
    }

    bool Debugger::BreakpointExists(uint32_t script, uint32_t pc) const
    {
        return std::any_of(m_Breakpoints.begin(), m_Breakpoints.end(), [script, pc](const Breakpoint& bp) {
            return bp.ScriptHash == script && bp.Pc == pc;
        });
    }

    void Debugger::SetPauseGameOnBreakpoint(bool set)
    {
        m_PauseGameOnBreakpoint = set;
    }

    std::optional<Debugger::Breakpoint> Debugger::GetActiveBreakpoint() const
    {
        return m_ActiveBreakpoint;
    }

    std::vector<Debugger::Breakpoint> Debugger::GetAllBreakpoints() const
    {
        return m_Breakpoints;
    }

    void Debugger::RemoveAllBreakpoints()
    {
        ResumeBreakpoint();
        m_Breakpoints.clear();
    }

    void Debugger::BeginTracking(uint32_t hash, uint32_t index, bool isGlobal)
    {
        if (!VMLogger::ShouldLog(isGlobal ? VMLogType::GLOBAL_WRITES : VMLogType::STATIC_WRITES, hash))
            return;

        m_TrackerActive = true;
        m_TrackingGlobal = isGlobal;
        m_TrackerVariableIndex = index;
        m_TrackerPathLen = 0;
        m_TrackerPathBuf[0] = '\0';
    }

    void Debugger::AddFieldOffset(uint32_t offset)
    {
        if (!m_TrackerActive)
            return;

        int written = std::snprintf(m_TrackerPathBuf + m_TrackerPathLen, sizeof(m_TrackerPathBuf) - m_TrackerPathLen, ".f_%u", offset);
        if (written > 0)
            m_TrackerPathLen += static_cast<std::size_t>(written);
    }

    void Debugger::AddArrayIndex(uint32_t index, uint32_t size)
    {
        if (!m_TrackerActive)
            return;

        int written = std::snprintf(m_TrackerPathBuf + m_TrackerPathLen, sizeof(m_TrackerPathBuf) - m_TrackerPathLen, "[%u /*%u*/]", index, size);
        if (written > 0)
            m_TrackerPathLen += static_cast<std::size_t>(written);
    }

    void Debugger::FinalizeTracking(const char* name, uint32_t pc, int32_t value, bool isStruct)
    {
        if (!m_TrackerActive)
            return;

        char valueBuf[32];
        if (isStruct)
            std::snprintf(valueBuf, sizeof(valueBuf), "struct<%d>", value);
        else
            std::snprintf(valueBuf, sizeof(valueBuf), "%d", value);

        std::snprintf(m_TrackerPathBuf + m_TrackerPathLen, sizeof(m_TrackerPathBuf) - m_TrackerPathLen, "%s", "");

        if (m_TrackingGlobal)
            VMLogger::Logf("[%s+0x%08X] Global_%u%s = %s", name, pc, m_TrackerVariableIndex, m_TrackerPathBuf, valueBuf);
        else
            VMLogger::Logf("[%s+0x%08X] Static_%u%s = %s", name, pc, m_TrackerVariableIndex, m_TrackerPathBuf, valueBuf);

        BreakTracking();
    }

    bool Debugger::ShouldBreakTracking(uint8_t op) const
    {
        if (!m_TrackerActive)
            return false;

        return !IsChainOpcode(op);
    }

    void Debugger::BreakTracking()
    {
        m_TrackerActive = false;
        m_TrackingGlobal = false;
        m_TrackerVariableIndex = 0;
        m_TrackerPathLen = 0;
        m_TrackerPathBuf[0] = '\0';
    }
}