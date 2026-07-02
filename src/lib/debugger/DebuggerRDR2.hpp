#pragma once

#if defined(_M_X64)

namespace scrDbgLib
{
    class DebuggerRDR2 : public Debugger
    {
    public:
        bool ProcessBreakpoints(uint32_t scriptHash, uint32_t pc, uint32_t* state) override;
        bool ResumeBreakpoint() override;
        bool IsChainOpcode(uint8_t op) const override;

    protected:
        void PauseGame(bool pause) override;
    };
}

#endif