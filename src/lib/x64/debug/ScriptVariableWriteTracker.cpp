#include "ScriptVariableWriteTracker.hpp"
#include "ScriptLogger.hpp"
#include "rage/scrOpcode.hpp"

namespace scrDbgLib
{
    void ScriptVariableWriteTracker::Begin(uint32_t hash, std::uint32_t index, bool isGlobal)
    {
        if (!ScriptLogger::ShouldLog(isGlobal ? ScriptLogger::LogType::LOG_TYPE_GLOBAL_WRITES : ScriptLogger::LogType::LOG_TYPE_STATIC_WRITES, hash))
            return;

        m_IsActive = true;
        m_IsGlobal = isGlobal;
        m_VariableIndex = index;
        m_PathLen = 0;
        m_PathBuf[0] = '\0';
    }

    void ScriptVariableWriteTracker::AddFieldOffset(std::uint32_t offset)
    {
        if (!m_IsActive)
            return;

        int written = std::snprintf(m_PathBuf + m_PathLen, sizeof(m_PathBuf) - m_PathLen, ".f_%u", offset);
        if (written > 0)
            m_PathLen += static_cast<std::size_t>(written);
    }

    void ScriptVariableWriteTracker::AddArrayIndex(std::uint32_t index, std::uint32_t size)
    {
        if (!m_IsActive)
            return;

        int written = std::snprintf(m_PathBuf + m_PathLen, sizeof(m_PathBuf) - m_PathLen, "[%u /*%u*/]", index, size);
        if (written > 0)
            m_PathLen += static_cast<std::size_t>(written);
    }

    void ScriptVariableWriteTracker::Finalize(const char* name, std::uint32_t pc, std::int32_t value, bool isStruct)
    {
        if (!m_IsActive)
            return;

        char valueBuf[32];
        if (isStruct)
            std::snprintf(valueBuf, sizeof(valueBuf), "struct<%d>", value);
        else
            std::snprintf(valueBuf, sizeof(valueBuf), "%d", value);

        std::snprintf(m_PathBuf + m_PathLen, sizeof(m_PathBuf) - m_PathLen, "%s", "");
        if (m_IsGlobal)
            ScriptLogger::Logf("[%s+0x%08X] Global_%u%s = %s", name, pc, m_VariableIndex, m_PathBuf, valueBuf);
        else
            ScriptLogger::Logf("[%s+0x%08X] Static_%u%s = %s", name, pc, m_VariableIndex, m_PathBuf, valueBuf);

        Break();
    }

    void ScriptVariableWriteTracker::Break()
    {
        m_IsActive = false;
        m_IsGlobal = false;
        m_VariableIndex = 0;
        m_PathLen = 0;
        m_PathBuf[0] = '\0';
    }

    bool ScriptVariableWriteTracker::ShouldBreak(std::uint8_t op)
    {
        if (!m_IsActive)
            return false;

        switch (op)
        {
        // continuation opcodes
        case rage::scrOpcode::IOFFSET:
        case rage::scrOpcode::IOFFSET_U8:
        case rage::scrOpcode::IOFFSET_S16:
        case rage::scrOpcode::ARRAY_U8:
        case rage::scrOpcode::ARRAY_U16:
        case rage::scrOpcode::PUSH_CONST_S16:
        case rage::scrOpcode::PUSH_CONST_U24:

        // finalizer opcodes
        case rage::scrOpcode::STORE:
        case rage::scrOpcode::STORE_N:
        case rage::scrOpcode::IOFFSET_U8_STORE:
        case rage::scrOpcode::IOFFSET_S16_STORE:
        case rage::scrOpcode::ARRAY_U8_STORE:
        case rage::scrOpcode::ARRAY_U16_STORE:
            return false;
        }

        // everything else breaks the chain
        return true;
    }
}