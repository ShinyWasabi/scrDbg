#pragma once

namespace rage::enhanced
{
    enum scrThreadState : std::uint32_t
    {
        RUNNING,
        IDLE,
        KILLED,
        PAUSED
    };

    enum scrThreadPriority : std::uint32_t
    {
        HIGHEST,
        NORMAL,
        LOWEST,
        MANUAL_UPDATE = 100
    };

    class scrThreadContext
    {
    public:
        std::uint32_t m_ThreadId;
        std::uint64_t m_ScriptHash;
        scrThreadState m_State;
        std::uint32_t m_ProgramCounter;
        std::uint32_t m_FramePointer;
        std::uint32_t m_StackPointer;
        float m_TimerA;
        float m_TimerB;
        float m_WaitTimer;
        char m_Pad1[0x2C];
        std::uint32_t m_StackSize;
        std::uint32_t m_CatchProgramCounter;
        std::uint32_t m_CatchFramePointer;
        std::uint32_t m_CatchStackPointer;
        scrThreadPriority m_Priority;
        std::uint8_t m_CallDepth;
        std::uint32_t m_CallStack[16];
    };
    static_assert(sizeof(scrThreadContext) == 0xB0);
}