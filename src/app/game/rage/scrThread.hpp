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
    public:
        scrThread(uintptr_t base = 0)
            : m_Base(base)
        {
        }

        uint32_t GetId() const;
        uint32_t GetProgramHash() const;
        scrThreadState GetState() const;
        void SetState(scrThreadState state) const;
        uint32_t GetProgramCounter() const;
        uint32_t GetFramePointer() const;
        uint32_t GetStackPointer() const;
        uint32_t GetStackSize() const;
        scrThreadPriority GetPriority() const;
        uint8_t GetCallDepth() const;
        uint32_t GetCallStack(uint32_t index) const;
        uint64_t GetStack(uint32_t index) const;
        void SetStack(uint32_t index, uint64_t value) const;
        std::string GetCreateTime() const;
        std::string GetExitReason() const;
        uint32_t GetScriptHash() const;
        std::string GetScriptName() const;

        static std::vector<scrThread> GetThreads();
        static scrThread GetThread(uint32_t hash);

        operator bool() const
        {
            return m_Base != 0;
        }

    private:
        static constexpr size_t ID_GEN8 = 0x08;
        static constexpr size_t PROGRAM_HASH_GEN8 = 0x0C;
        static constexpr size_t STATE_GEN8 = 0x10;
        static constexpr size_t PROGRAM_COUNTER_GEN8 = 0x14;
        static constexpr size_t FRAME_POINTER_GEN8 = 0x18;
        static constexpr size_t STACK_POINTER_GEN8 = 0x1C;
        static constexpr size_t STACK_SIZE_GEN8 = 0x58;
        static constexpr size_t PRIORITY_GEN8 = 0x68;
        static constexpr size_t CALL_DEPTH_GEN8 = 0x6C;
        static constexpr size_t CALL_STACK_GEN8 = 0x70;
        static constexpr size_t STACK_GEN8 = 0xB0;
        static constexpr size_t CREATE_TIME_GEN8 = 0xC4;
        static constexpr size_t EXIT_REASON_GEN8 = 0xC8;
        static constexpr size_t SCRIPT_HASH_GEN8 = 0xD0;
        static constexpr size_t SCRIPT_NAME_GEN8 = 0xD4;

        static constexpr size_t ID_GEN9 = 0x08;
        static constexpr size_t PROGRAM_HASH_GEN9 = 0x10;
        static constexpr size_t STATE_GEN9 = 0x18;
        static constexpr size_t PROGRAM_COUNTER_GEN9 = 0x1C;
        static constexpr size_t FRAME_POINTER_GEN9 = 0x20;
        static constexpr size_t STACK_POINTER_GEN9 = 0x24;
        static constexpr size_t STACK_SIZE_GEN9 = 0x60;
        static constexpr size_t PRIORITY_GEN9 = 0x70;
        static constexpr size_t CALL_DEPTH_GEN9 = 0x74;
        static constexpr size_t CALL_STACK_GEN9 = 0x78;
        static constexpr size_t STACK_GEN9 = 0xB8;
        static constexpr size_t CREATE_TIME_GEN9 = 0xCC;
        static constexpr size_t EXIT_REASON_GEN9 = 0xD0;
        static constexpr size_t SCRIPT_HASH_GEN9 = 0x150;
        static constexpr size_t SCRIPT_NAME_GEN9 = 0x154;

        Pointer m_Base;
    };
}