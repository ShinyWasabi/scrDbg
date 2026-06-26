#pragma once

#if defined(_M_X64)

namespace rage::gta5
{
    class scrThreadGEN8;
    class scrThreadGEN9;
}

namespace scrDbgLib
{
    class DebuggerGTA5 : public Debugger
    {
    public:
        bool IsChainOpcode(uint8_t op) const override;

    protected:
        void PauseGame(bool pause) override;

        template <typename T>
        bool ProcessBreakpointsInternal(uint32_t scriptHash, uint32_t pc, uint32_t* state);

        template <typename T>
        bool ResumeBreakpointInternal();
    };

    class DebuggerGTA5_GEN8 : public DebuggerGTA5
    {
    public:
        bool ProcessBreakpoints(uint32_t scriptHash, uint32_t pc, uint32_t* state) override
        {
            return ProcessBreakpointsInternal<rage::gta5::scrThreadGEN8>(scriptHash, pc, state);
        }

        bool ResumeBreakpoint() override
        {
            return ResumeBreakpointInternal<rage::gta5::scrThreadGEN8>();
        }
    };

    class DebuggerGTA5_GEN9 : public DebuggerGTA5
    {
    public:
        bool ProcessBreakpoints(uint32_t scriptHash, uint32_t pc, uint32_t* state) override
        {
            return ProcessBreakpointsInternal<rage::gta5::scrThreadGEN9>(scriptHash, pc, state);
        }

        bool ResumeBreakpoint() override
        {
            return ResumeBreakpointInternal<rage::gta5::scrThreadGEN9>();
        }
    };
}

#endif