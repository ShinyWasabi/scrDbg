#pragma once

namespace scrDbg::PipeCommands
{
    enum class ePipeCommands : uint8_t
    {
        BREAKPOINT_SET,
        BREAKPOINT_EXISTS,
        BREAKPOINT_ACTIVE,
        BREAKPOINT_RESUME,
        BREAKPOINT_GET_ALL,
        BREAKPOINT_REMOVE_ALL
    };

    struct PipeCommandSetBreakpoint
    {
        uint32_t Script;
        uint32_t Pc;
        bool Set;
    };

    struct PipeCommandBreakpointExists
    {
        uint32_t Script;
        uint32_t Pc;
    };

    struct PipeCommandBreakpointGetAll
    {
        uint32_t Script;
        uint32_t Pc;
    };

    extern void SetBreakpoint(uint32_t script, uint32_t pc, bool set);
    extern bool BreakpointExists(uint32_t script, uint32_t pc);
    extern bool BreakpointActive();
    extern void ResumeBreakpoint();
    extern std::vector<std::pair<uint32_t, uint32_t>> GetAllBreakpoints();
    void RemoveAllBreakpoints();
}