#pragma once

enum class ePipeCommands : std::uint8_t
{
    BREAKPOINT_SET,
    BREAKPOINT_EXISTS,
    BREAKPOINT_RESUME,
    BREAKPOINT_PAUSE_GAME,
    BREAKPOINT_GET_ACTIVE,
    BREAKPOINT_GET_ALL,
    BREAKPOINT_REMOVE_ALL
};

struct PipeBreakpoint
{
    std::uint32_t Script;
    std::uint32_t Pc;
};

struct PipeBreakpointSet : PipeBreakpoint
{
    bool Set;
};