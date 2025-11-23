#pragma once

namespace scrDbgApp::PipeCommands
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

        LOGGER_SET_TYPE,
        LOGGER_SET_SCRIPT,
        LOGGER_CLEAR_FILE
    };

    struct PipeBreakpoint
    {
        uint32_t Script;
        uint32_t Pc;
    };

    struct PipeBreakpointSet : PipeBreakpoint
    {
        bool Set;
    };

    extern void SetBreakpoint(uint32_t script, uint32_t pc, bool set);
    extern bool BreakpointExists(uint32_t script, uint32_t pc);
    extern void ResumeBreakpoint();
    extern void ResumeBreakpoint();
    extern void SetBreakpointPauseGame(bool pause);
    std::optional<std::pair<uint32_t, uint32_t>> GetActiveBreakpoint();
    extern std::vector<std::pair<uint32_t, uint32_t>> GetAllBreakpoints();
    extern void RemoveAllBreakpoints();

    extern void SetLoggerType(int type);
    extern void SetLoggerScript(uint32_t hash);
    extern void ClearLoggerFile();
}