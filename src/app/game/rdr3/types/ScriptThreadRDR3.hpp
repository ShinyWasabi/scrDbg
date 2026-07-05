#pragma once

namespace scrDbgApp
{
    class ScriptThreadRDR3 : public ScriptThread
    {
    public:
        ScriptThreadRDR3(uintptr_t base = 0)
            : m_Base(base)
        {
        }

        uint32_t GetId() const override;
        uint32_t GetProgramHash() const override;
        State GetState() const override;
        void SetState(State state) const override;
        uint32_t GetPc() const override;
        uint32_t GetFp() const override;
        uint32_t GetSp() const override;
        uint32_t GetStackSize() const override;
        Priority GetPriority() const override;
        uint8_t GetCallDepth() const override;
        uint32_t GetCallStack(uint32_t index) const override;
        Pointer GetStack(uint32_t index) const override;
        std::string GetKillReason() const override;
        uint32_t GetScriptHash() const override;
        std::string GetScriptName() const;

    private:
        static constexpr size_t ID = 0x08;
        static constexpr size_t PROGRAM_HASH = 0x0C;
        static constexpr size_t STATE = 0x10;
        static constexpr size_t PC = 0x14;
        static constexpr size_t FP = 0x18;
        static constexpr size_t SP = 0x1C;
        static constexpr size_t STACK_SIZE = 0x58;
        static constexpr size_t PRIORITY = 0x68;
        static constexpr size_t CALL_DEPTH = 0x6C;
        static constexpr size_t CALL_STACK = 0x70;
        static constexpr size_t STACK = 0x6B8;
        static constexpr size_t KILL_REASON = 0x6D0;
        static constexpr size_t SCRIPT_HASH = 0x6D8;

        Pointer m_Base;
    };
}