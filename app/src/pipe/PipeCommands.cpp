#include "PipeCommands.hpp"
#include "core/PipeClient.hpp"

namespace scrDbg::PipeCommands
{
    void SetBreakpoint(uint32_t script, uint32_t pc, bool set)
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_SET);
        PipeClient::Send(&cmd, sizeof(cmd));

        PipeBreakpointSet args{script, pc, set};
        PipeClient::Send(&args, sizeof(args));
    }

    bool BreakpointExists(uint32_t script, uint32_t pc)
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_EXISTS);
        PipeClient::Send(&cmd, sizeof(cmd));

        PipeBreakpoint args{script, pc};
        PipeClient::Send(&args, sizeof(args));

        bool result = false;
        PipeClient::Receive(&result, sizeof(result));

        return result;
    }

    void ResumeBreakpoint()
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_RESUME);
        PipeClient::Send(&cmd, sizeof(cmd));
    }

    void SetBreakpointPauseGame(bool pause)
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_PAUSE_GAME);
        PipeClient::Send(&cmd, sizeof(cmd));

        PipeClient::Send(&pause, sizeof(pause));
    }

    std::optional<std::pair<uint32_t, uint32_t>> GetActiveBreakpoint()
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_GET_ACTIVE);
        PipeClient::Send(&cmd, sizeof(cmd));

        bool active = false;
        PipeClient::Receive(&active, sizeof(active));

        if (!active)
            return std::nullopt;

        PipeBreakpoint breakpoint{};
        PipeClient::Receive(&breakpoint, sizeof(breakpoint));

        return std::make_pair(breakpoint.Script, breakpoint.Pc);
    }

    std::vector<std::pair<uint32_t, uint32_t>> GetAllBreakpoints()
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_GET_ALL);
        PipeClient::Send(&cmd, sizeof(cmd));

        uint32_t count = 0;
        PipeClient::Receive(&count, sizeof(count));

        if (count == 0)
            return {};

        std::vector<std::pair<uint32_t, uint32_t>> result;
        result.reserve(count);

        for (uint32_t i = 0; i < count; i++)
        {
            PipeBreakpoint entry{};
            if (!PipeClient::Receive(&entry, sizeof(entry)))
                break;

            result.emplace_back(entry.Script, entry.Pc);
        }

        return result;
    }

    void RemoveAllBreakpoints()
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_REMOVE_ALL);
        PipeClient::Send(&cmd, sizeof(cmd));
    }

    extern void SetLoggerType(int type)
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::LOGGER_SET_TYPE);
        PipeClient::Send(&cmd, sizeof(cmd));

        PipeClient::Send(&type, sizeof(type));
    }

    extern void SetLoggerScript(uint32_t hash)
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::LOGGER_SET_SCRIPT);
        PipeClient::Send(&cmd, sizeof(cmd));

        PipeClient::Send(&hash, sizeof(hash));
    }

    extern void ClearLoggerFile()
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::LOGGER_CLEAR_FILE);
        PipeClient::Send(&cmd, sizeof(cmd));
    }
}