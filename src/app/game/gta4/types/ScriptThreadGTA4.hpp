#pragma once

namespace scrDbgApp
{
    class ScriptThreadGTA4 : public ScriptThread
    {
    public:
        ScriptThreadGTA4(uintptr_t base = 0)
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
        Pointer GetStack(uint32_t index) const override;
        std::string GetKillReason() const override;
        uint32_t GetScriptHash() const override;
        std::string GetScriptName() const override;

    private:
        static constexpr size_t ID = 0x04;
        static constexpr size_t PROGRAM_HASH = 0x08;
        static constexpr size_t STATE = 0x0C;
        static constexpr size_t PC = 0x10;
        static constexpr size_t FP = 0x14;
        static constexpr size_t SP = 0x18;
        static constexpr size_t STACK_SIZE = 0x44;
        static constexpr size_t STACK = 0x58;
        static constexpr size_t KILL_REASON = 0x6C;
        static constexpr size_t SCRIPT_NAME = 0x70;

        Pointer m_Base;
    };
}