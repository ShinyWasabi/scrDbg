#include "scrThread.hpp"
#include "Pointers.hpp"

namespace rage
{
    uint32_t scrThread::GetId() const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? ID_GEN9 : ID_GEN8).Get<uint32_t>();
    }

    uint32_t scrThread::GetProgramHash() const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? PROGRAM_HASH_GEN9 : PROGRAM_HASH_GEN8).Get<uint32_t>();
    }

    scrThreadState scrThread::GetState() const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? STATE_GEN9 : STATE_GEN8).Get<scrThreadState>();
    }

    void scrThread::SetState(scrThreadState state) const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? STATE_GEN9 : STATE_GEN8).Set<scrThreadState>(state);
    }

    uint32_t scrThread::GetProgramCounter() const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? PROGRAM_COUNTER_GEN9 : PROGRAM_COUNTER_GEN8).Get<uint32_t>();
    }

    uint32_t scrThread::GetFramePointer() const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? FRAME_POINTER_GEN9 : FRAME_POINTER_GEN8).Get<uint32_t>();
    }

    uint32_t scrThread::GetStackPointer() const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? STACK_POINTER_GEN9 : STACK_POINTER_GEN8).Get<uint32_t>();
    }

    uint32_t scrThread::GetStackSize() const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? STACK_SIZE_GEN9 : STACK_SIZE_GEN8).Get<uint32_t>();
    }

    scrThreadPriority scrThread::GetPriority() const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? PRIORITY_GEN9 : PRIORITY_GEN8).Get<scrThreadPriority>();
    }

    uint8_t scrThread::GetCallDepth() const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? CALL_DEPTH_GEN9 : CALL_DEPTH_GEN8).Get<uint8_t>();
    }

    uint32_t scrThread::GetCallStack(uint32_t index) const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? CALL_STACK_GEN9 : CALL_STACK_GEN8).GetArray<uint32_t>(index);
    }

    uint64_t scrThread::GetStack(uint32_t index) const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? STACK_GEN9 : STACK_GEN8).Deref().GetArray<uint64_t>(index);
    }

    void scrThread::SetStack(uint32_t index, uint64_t value) const
    {
        m_Base.Add(scrDbgApp::g_IsEnhanced ? STACK_GEN9 : STACK_GEN8).Deref().SetArray<uint64_t>(index, value);
    }

    std::string scrThread::GetCreateTime() const
    {
        uint32_t createTime = m_Base.Add(scrDbgApp::g_IsEnhanced ? CREATE_TIME_GEN9 : CREATE_TIME_GEN8).Get<uint32_t>();

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

    std::string scrThread::GetExitReason() const
    {
        if (scrDbgApp::g_IsEnhanced)
        {
            char buffer[128];
            m_Base.Add(scrDbgApp::g_IsEnhanced ? EXIT_REASON_GEN9 : EXIT_REASON_GEN8).GetBuffer(buffer, 128);
            return std::string(buffer);
        }

        return m_Base.Add(scrDbgApp::g_IsEnhanced ? EXIT_REASON_GEN9 : EXIT_REASON_GEN8).GetString(128);
    }

    uint32_t scrThread::GetScriptHash() const
    {
        return m_Base.Add(scrDbgApp::g_IsEnhanced ? SCRIPT_HASH_GEN9 : SCRIPT_HASH_GEN8).Get<uint32_t>();
    }

    std::string scrThread::GetScriptName() const
    {
        char buffer[64];
        m_Base.Add(scrDbgApp::g_IsEnhanced ? SCRIPT_NAME_GEN9 : SCRIPT_NAME_GEN8).GetBuffer(buffer, 64);
        return std::string(buffer);
    }

    std::vector<scrThread> scrThread::GetThreads()
    {
        std::vector<scrThread> threads;

        auto base = scrDbgApp::g_Pointers.ScriptThreads.Deref();
        uint16_t count = scrDbgApp::g_Pointers.ScriptThreads.Add(10).Get<uint16_t>();

        for (uint16_t i = 0; i < count; ++i)
        {
            scrThread thread(base.GetArray<uintptr_t>(i));
            threads.push_back(thread);
        }

        return threads;
    }

    scrThread scrThread::GetThread(uint32_t hash)
    {
        if (!hash)
            return {};

        for (const auto& thread : GetThreads())
        {
            if (thread.GetScriptHash() == hash)
                return thread;
        }

        return {};
    }
}