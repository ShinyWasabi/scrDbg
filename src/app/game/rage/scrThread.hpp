#pragma once

namespace rage
{
    enum scrThreadState : uint32_t
    {
        RUNNING,
        IDLE,
        KILLED,
        PAUSED
    };

    enum scrThreadPriority : uint32_t
    {
        HIGHEST,
        NORMAL,
        LOWEST,
        MANUAL_UPDATE = 100
    };

    class scrThread
    {
        struct _scrThreadLegacy
        {
            char VTablePtr[0x08];
            uint32_t ThreadId;
            uint32_t Program;
            uint32_t State;
            uint32_t ProgramCounter;
            uint32_t FramePointer;
            uint32_t StackPointer;
            float TimerA;
            float TimerB;
            float WaitTimer;
            char Pad1[0x2C];
            uint32_t StackSize;
            uint32_t CatchProgramCounter;
            uint32_t CatchFramePointer;
            uint32_t CatchStackPointer;
            uint32_t Priority;
            uint8_t CallDepth;
            uint32_t CallStack[16];
            uint64_t* Stack;
            char Pad2[0x04];
            uint32_t ArgSize;
            uint32_t ArgLoc;
            char Pad3[0x04];
            const char* ErrorMessage;
            uint32_t ScriptHash;
            char ScriptName[64];
        };
        static_assert(sizeof(_scrThreadLegacy) == 0x118);

        struct _scrThreadEnhanced
        {
            char VTablePtr[0x08];
            uint32_t ThreadId;
            uint64_t Program;
            uint32_t State;
            uint32_t ProgramCounter;
            uint32_t FramePointer;
            uint32_t StackPointer;
            float TimerA;
            float TimerB;
            float WaitTimer;
            char Pad1[0x2C];
            uint32_t StackSize;
            uint32_t CatchProgramCounter;
            uint32_t CatchFramePointer;
            uint32_t CatchStackPointer;
            uint32_t Priority;
            uint8_t CallDepth;
            uint32_t CallStack[16];
            uint64_t* Stack;
            char Pad2[0x04];
            uint32_t ArgSize;
            uint32_t ArgLoc;
            char Pad3[0x04];
            char ErrorMessage[128];
            uint32_t ScriptHash;
            char ScriptName[64];
        };
        static_assert(sizeof(_scrThreadEnhanced) == 0x198);

    public:
        scrThread(uint64_t address = 0)
            : m_Address(address)
        {
        }

        uint32_t GetId() const;
        uint32_t GetProgram() const;
        scrThreadState GetState() const;
        void SetState(scrThreadState state) const;
        uint32_t GetProgramCounter() const;
        uint32_t GetFramePointer() const;
        uint32_t GetStackPointer() const;
        float GetTimerA() const;
        float GetTimerB() const;
        float GetWaitTimer() const;
        uint32_t GetStackSize() const;
        scrThreadPriority GetPriority() const;
        uint8_t GetCallDepth() const;
        uint32_t GetCallStack(uint32_t index) const;
        uint64_t GetStack(uint64_t index) const;
        void SetStack(uint64_t index, uint64_t value) const;
        std::string GetErrorMessage() const;
        uint32_t GetHash() const;
        std::string GetName() const;

        static std::vector<scrThread> GetThreads();
        static scrThread GetThread(uint32_t hash);

        operator bool()
        {
            return m_Address != 0;
        }

    private:
        uint64_t m_Address;
    };
}