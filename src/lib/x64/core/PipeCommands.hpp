#pragma once

namespace scrDbgLib
{
    enum class ePipeCommands : std::uint8_t
    {
        BREAKPOINT_SET,
        BREAKPOINT_EXISTS,
        BREAKPOINT_RESUME,
        BREAKPOINT_PAUSE_GAME,
        BREAKPOINT_GET_ACTIVE,
        BREAKPOINT_GET_ALL,
        BREAKPOINT_REMOVE_ALL,

        LOGGER_SET_TYPE,
        LOGGER_SET_SCRIPT,
        LOGGER_CLEAR_FILE
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
}