#include "Debugger.hpp"
#include "VMLogger.hpp"
#include "core/ScriptFiber.hpp"

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

    std::string Debugger::NativeLogFormat(rage::scrValue value, NativeDB::Types type, void* thread, void* program, rage::scrValue* globals) const
    {
        switch (type)
        {
        case NativeDB::Types::INT:
            return std::to_string(value.Int);
        case NativeDB::Types::BOOL:
            return value.Int ? "TRUE" : "FALSE";
        case NativeDB::Types::FLOAT:
            return std::to_string(value.Float);
        case NativeDB::Types::STRING:
            return value.String ? "\"" + std::string(value.String) + "\"" : "NULL";
        case NativeDB::Types::REFERENCE:
            return "0x" + std::to_string(reinterpret_cast<uintptr_t>(value.Reference));
        }

        return std::to_string(value.Any);
    }

    void Debugger::NativeLogBegin(uint32_t scriptHash, void* handler, rage::scrValue* args, uint32_t argCount, void* thread, void* program, rage::scrValue* globals)
    {
        NativeLogClear();

        m_ShouldLogNative = VMLogger::ShouldLog(VMLogType::NATIVE_CALLS, scriptHash);
        if (!m_ShouldLogNative)
            return;

        m_NativeLogHash = g_Game->GetNativeHash(handler);
        m_NativeLogName = NativeDB::GetNameByHash(m_NativeLogHash);

        auto argTypes = NativeDB::GetArgsByHash(m_NativeLogHash);
        if (argCount > 0 && args && argTypes && !argTypes->empty())
        {
            for (int32_t i = 0; i < argCount; i++)
            {
                auto type = (i < argTypes->size()) ? (*argTypes)[i] : NativeDB::Types::NONE;
                m_NativeLogArgs += NativeLogFormat(args[i], type, thread, program, globals);

                if (i < argCount - 1)
                    m_NativeLogArgs += ", ";
            }
        }
    }

    void Debugger::NativeLogEnd(const char* name, uint32_t pc, rage::scrValue* rets, uint32_t retCount, void* thread, void* program, rage::scrValue* globals)
    {
        if (!m_ShouldLogNative)
            return;

        auto retTypes = NativeDB::GetRetsByHash(m_NativeLogHash);
        if (retCount > 0 && rets && retTypes && !retTypes->empty())
        {
            m_NativeLogRets += " -> (";
            for (int32_t i = 0; i < retCount; i++)
            {
                auto type = (i < retTypes->size()) ? (*retTypes)[i] : NativeDB::Types::NONE;
                m_NativeLogRets += NativeLogFormat(rets[i], type, thread, program, globals);

                if (i < retCount - 1)
                    m_NativeLogRets += ", ";
            }
            m_NativeLogRets += ")";
        }

        if (!m_NativeLogName.empty())
            VMLogger::Logf("[%s+0x%08X] %s(%s)%s", name, pc, m_NativeLogName.data(), m_NativeLogArgs.c_str(), m_NativeLogRets.c_str());
        else
            VMLogger::Logf("[%s+0x%08X] unk_0x%08X(%s)%s", name, pc, m_NativeLogHash, m_NativeLogArgs.c_str(), m_NativeLogRets.c_str());

        NativeLogClear();
    }

    void Debugger::NativeLogClear()
    {
        m_NativeLogHash = 0;
        m_NativeLogName = {};
        m_NativeLogArgs.clear();
        m_NativeLogRets.clear();
        m_ShouldLogNative = false;
    }

    std::unique_ptr<NativeContext> Debugger::CreateNativeContext() const
    {
        return std::make_unique<NativeContextDefault>();
    }

    void Debugger::PushNativeInvoke(uint32_t scriptHash, void* handler, NativeContext* ctx, std::shared_ptr<std::promise<void>> promise)
    {
        ScriptFiber::PushJob([ctx, handler, promise]() {
            ctx->Invoke(handler);
            promise->set_value();
        },
            scriptHash);
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