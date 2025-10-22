#include "PipeCommands.hpp"
#include "PipeClient.hpp"

namespace scrDbg::PipeCommands
{
    void SetBreakpoint(uint32_t script, uint32_t pc, bool set)
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_SET);
        PipeClient::Send(&cmd, sizeof(cmd));

        PipeCommandSetBreakpoint args{ script, pc, set };
        PipeClient::Send(&args, sizeof(args));
    }

    bool BreakpointExists(uint32_t script, uint32_t pc)
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_EXISTS);
        PipeClient::Send(&cmd, sizeof(cmd));

        PipeCommandBreakpointExists args{ script, pc };
        PipeClient::Send(&args, sizeof(args));

        uint8_t result = 0;
        if (!PipeClient::Receive(&result, sizeof(result)))
            return false;

        return result != 0;
    }

    bool BreakpointActive()
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_ACTIVE);
        PipeClient::Send(&cmd, sizeof(cmd));

        uint8_t result = 0;
        if (!PipeClient::Receive(&result, sizeof(result)))
            return false;

        return result != 0;
    }

    void ResumeBreakpoint()
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_RESUME);
        PipeClient::Send(&cmd, sizeof(cmd));
    }

    std::vector<std::pair<uint32_t, uint32_t>> GetAllBreakpoints()
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::BREAKPOINT_GET_ALL);
        PipeClient::Send(&cmd, sizeof(cmd));

        uint32_t count = 0;
        if (!PipeClient::Receive(&count, sizeof(count)) || count == 0)
            return {};

        std::vector<std::pair<uint32_t, uint32_t>> result;
        result.reserve(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            PipeCommandBreakpointGetAll entry{};
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
}