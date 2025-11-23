#include "scrThread.hpp"
#include "Pointers.hpp"
#include "core/Process.hpp"

namespace rage
{
    uint32_t scrThread::GetId() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, ThreadId) : offsetof(_scrThreadLegacy, ThreadId);
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offset);
    }

    uint32_t scrThread::GetProgram() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, Program) : offsetof(_scrThreadLegacy, Program);
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offset);
    }

    scrThreadState scrThread::GetState() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, State) : offsetof(_scrThreadLegacy, State);
        return scrDbgApp::Process::Read<scrThreadState>(m_Address + offset);
    }

    void scrThread::SetState(scrThreadState state) const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, State) : offsetof(_scrThreadLegacy, State);
        scrDbgApp::Process::Write<scrThreadState>(m_Address + offset, state);
    }

    uint32_t scrThread::GetProgramCounter() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, ProgramCounter) : offsetof(_scrThreadLegacy, ProgramCounter);
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offset);
    }

    uint32_t scrThread::GetFramePointer() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, FramePointer) : offsetof(_scrThreadLegacy, FramePointer);
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offset);
    }

    uint32_t scrThread::GetStackPointer() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, StackPointer) : offsetof(_scrThreadLegacy, StackPointer);
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offset);
    }

    float scrThread::GetTimerA() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, TimerA) : offsetof(_scrThreadLegacy, TimerA);
        return scrDbgApp::Process::Read<float>(m_Address + offset);
    }

    float scrThread::GetTimerB() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, TimerB) : offsetof(_scrThreadLegacy, TimerB);
        return scrDbgApp::Process::Read<float>(m_Address + offset);
    }

    float scrThread::GetWaitTimer() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, WaitTimer) : offsetof(_scrThreadLegacy, WaitTimer);
        return scrDbgApp::Process::Read<float>(m_Address + offset);
    }

    uint32_t scrThread::GetStackSize() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, StackSize) : offsetof(_scrThreadLegacy, StackSize);
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offset);
    }

    scrThreadPriority scrThread::GetPriority() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, Priority) : offsetof(_scrThreadLegacy, Priority);
        return scrDbgApp::Process::Read<scrThreadPriority>(m_Address + offset);
    }

    uint8_t scrThread::GetCallDepth() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, CallDepth) : offsetof(_scrThreadLegacy, CallDepth);
        return scrDbgApp::Process::Read<uint8_t>(m_Address + offset);
    }

    uint32_t scrThread::GetCallStack(uint32_t index) const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, CallStack) : offsetof(_scrThreadLegacy, CallStack);
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offset + index * sizeof(uint32_t));
    }

    uint64_t scrThread::GetStack(uint64_t index) const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, Stack) : offsetof(_scrThreadLegacy, Stack);
        uint64_t base = scrDbgApp::Process::Read<uint64_t>(m_Address + offset);
        return scrDbgApp::Process::Read<uint64_t>(base + index * sizeof(uint64_t));
    }

    void scrThread::SetStack(uint64_t index, uint64_t value) const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, Stack) : offsetof(_scrThreadLegacy, Stack);
        uint64_t base = scrDbgApp::Process::Read<uint64_t>(m_Address + offset);
        scrDbgApp::Process::Write<uint64_t>(base + index * sizeof(uint64_t), value);
    }

    std::string scrThread::GetErrorMessage() const
    {
        std::string message;

        if (scrDbgApp::g_IsEnhanced)
        {
            size_t offset = offsetof(_scrThreadEnhanced, ErrorMessage);
            for (size_t i = 0; i < 128; i++)
            {
                char c = scrDbgApp::Process::Read<char>(m_Address + offset + i);
                if (!c)
                    break;
                message.push_back(c);
            }
        }
        else
        {
            size_t offset = offsetof(_scrThreadLegacy, ErrorMessage);
            uintptr_t ptr = scrDbgApp::Process::Read<uintptr_t>(m_Address + offset);
            if (!ptr)
                return message;

            for (size_t i = 0; i < 128; i++)
            {
                char c = scrDbgApp::Process::Read<char>(ptr + i);
                if (!c)
                    break;
                message.push_back(c);
            }
        }

        return message;
    }

    uint32_t scrThread::GetHash() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, ScriptHash) : offsetof(_scrThreadLegacy, ScriptHash);
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offset);
    }

    std::string scrThread::GetName() const
    {
        size_t offset = scrDbgApp::g_IsEnhanced ? offsetof(_scrThreadEnhanced, ScriptName) : offsetof(_scrThreadLegacy, ScriptName);

        std::string name;
        for (size_t i = 0; i < 64; i++)
        {
            char c = scrDbgApp::Process::Read<char>(m_Address + offset + i);
            if (!c)
                break;
            name.push_back(c);
        }

        return name;
    }

    std::vector<scrThread> scrThread::GetThreads()
    {
        std::vector<scrThread> threads;

        uint64_t base = scrDbgApp::g_Pointers.ScriptThreads.Read<uint64_t>();
        uint16_t count = scrDbgApp::g_Pointers.ScriptThreads.Add(10).Read<uint16_t>();

        for (uint16_t i = 0; i < count; ++i)
        {
            scrThread thread(scrDbgApp::Process::Read<uint64_t>(base + i * sizeof(uint64_t)));
            threads.push_back(thread);
        }

        return threads;
    }

    scrThread scrThread::GetThread(uint32_t hash)
    {
        if (!hash)
            return scrThread();

        uint64_t base = scrDbgApp::g_Pointers.ScriptThreads.Read<uint64_t>();
        uint16_t count = scrDbgApp::g_Pointers.ScriptThreads.Add(10).Read<uint16_t>();

        for (int i = 0; i < count; i++)
        {
            scrThread thread(scrDbgApp::Process::Read<uint64_t>(base + i * sizeof(uint64_t)));
            if (thread.GetHash() == hash)
                return thread;
        }

        return scrThread();
    }
}