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
        class Legacy
        {
        public:
            class Context
            {
            public:
                uint32_t m_ThreadId;
                uint32_t m_Program;
                uint32_t m_State;
                uint32_t m_ProgramCounter;
                uint32_t m_FramePointer;
                uint32_t m_StackPointer;
                float m_TimerA;
                float m_TimerB;
                float m_WaitTimer;
                char m_Pad1[0x2C];
                uint32_t m_StackSize;
                uint32_t m_CatchProgramCounter;
                uint32_t m_CatchFramePointer;
                uint32_t m_CatchStackPointer;
                uint32_t m_Priority;
                uint8_t m_CallDepth;
                uint32_t m_CallStack[16];
            };

            virtual ~Legacy() = default;
            virtual void Reset(uint32_t prog, void* args, uint32_t argCount) = 0;
            virtual uint32_t RunImpl() = 0;
            virtual uint32_t Run() = 0;
            virtual void Kill() = 0;

            Context m_Context;
            uint64_t* m_Stack;
            char m_Pad1[0x04];
            uint32_t m_ArgSize;
            uint32_t m_ArgLoc;
            char m_Pad2[0x04];
            const char* m_ErrorMessage;
            uint32_t m_ScriptHash;
            char m_ScriptName[64];
        };

        class Enhanced
        {
        public:
            class Context
            {
            public:
                uint32_t m_ThreadId;
                uint64_t m_Program;
                uint32_t m_State;
                uint32_t m_ProgramCounter;
                uint32_t m_FramePointer;
                uint32_t m_StackPointer;
                float m_TimerA;
                float m_TimerB;
                float m_WaitTimer;
                char m_Pad1[0x2C];
                uint32_t m_StackSize;
                uint32_t m_CatchProgramCounter;
                uint32_t m_CatchFramePointer;
                uint32_t m_CatchStackPointer;
                uint32_t m_Priority;
                uint8_t m_CallDepth;
                uint32_t m_CallStack[16];
            };

            virtual ~Enhanced() = default;
            virtual void Reset(uint64_t prog, void* args, uint32_t argCount) = 0;
            virtual uint32_t RunImpl() = 0;
            virtual uint32_t Run() = 0;
            virtual void Kill() = 0;
            virtual void GetInfo(void* info) = 0;

            Context m_Context;
            uint64_t* m_Stack;
            char m_Pad1[0x04];
            uint32_t m_ArgSize;
            uint32_t m_ArgLoc;
            char m_Pad2[0x04];
            char m_ErrorMessage[128];
            uint32_t m_ScriptHash;
            char m_ScriptName[64];
        };

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