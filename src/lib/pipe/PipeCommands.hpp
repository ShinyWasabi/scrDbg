#pragma once

namespace scrDbgLib::PipeCommands
{
    enum class ePipeCommands : uint8_t
    {
        BREAKPOINT_SET,
        BREAKPOINT_EXISTS,
        BREAKPOINT_RESUME,
        BREAKPOINT_PAUSE_GAME,
        BREAKPOINT_GET_ACTIVE,
        BREAKPOINT_GET_ALL,
        BREAKPOINT_REMOVE_ALL,

        NATIVE_INVOKE,

        LOGGER_SET_TYPE,
        LOGGER_SET_SCRIPT,
        LOGGER_CLEAR_FILE
    };

    struct PipeBreakpoint
    {
        uint32_t Script = 0;
        uint32_t Pc = 0;
    };

    struct PipeBreakpointSet : PipeBreakpoint
    {
        bool Set = 0;
    };

    extern void SetBreakpoint();
    extern void BreakpointExists();
    extern void ResumeBreakpoint();
    extern void SetBreakpointPauseGame();
    extern void GetActiveBreakpoint();
    extern void GetAllBreakpoints();
    extern void RemoveAllBreakpoints();

    extern void InvokeNative();

    extern void SetLoggerType();
    extern void SetLoggerScript();
    extern void ClearLoggerFile();
}