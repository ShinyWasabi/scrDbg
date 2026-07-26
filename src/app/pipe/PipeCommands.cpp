#include "PipeCommands.hpp"
#include "core/PipeClient.hpp"

namespace scrDbgApp::PipeCommands
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

    static void SendNativeArg(const PipeNativeArg& arg)
    {
        switch (arg.Type)
        {
        case NativeDB::Types::INT:
            PipeClient::Send(&arg.IntValue, sizeof(arg.IntValue));
            break;
        case NativeDB::Types::BOOL:
            PipeClient::Send(&arg.BoolValue, sizeof(arg.BoolValue));
            break;
        case NativeDB::Types::FLOAT:
            PipeClient::Send(&arg.FloatValue, sizeof(arg.FloatValue));
            break;
        case NativeDB::Types::STRING:
        {
            uint32_t len = static_cast<uint32_t>(arg.StringValue.size());
            PipeClient::Send(&len, sizeof(len));
            if (len > 0)
                PipeClient::Send(arg.StringValue.data(), len);
            break;
        }
        case NativeDB::Types::REFERENCE:
        {
            if (g_Game->Is64Bit())
                PipeClient::Send(&arg.RefValue64, sizeof(arg.RefValue64));
            else
                PipeClient::Send(&arg.RefValue32, sizeof(arg.RefValue32));
            break;
        }
        }
    }

    static bool ReceiveNativeRet(PipeNativeReturn& out)
    {
        switch (out.Type)
        {
        case NativeDB::Types::INT:
            return PipeClient::Receive(&out.IntValue, sizeof(out.IntValue));
        case NativeDB::Types::BOOL:
            return PipeClient::Receive(&out.BoolValue, sizeof(out.BoolValue));
        case NativeDB::Types::FLOAT:
            return PipeClient::Receive(&out.FloatValue, sizeof(out.FloatValue));
        case NativeDB::Types::STRING:
        {
            uint32_t len = 0;
            if (!PipeClient::Receive(&len, sizeof(len)))
                return false;
            if (len > 0)
            {
                out.StringValue.resize(len);
                if (!PipeClient::Receive(out.StringValue.data(), len))
                    return false;
            }
            return true;
        }
        }

        return true;
    }

    PipeNativeInvokeResult InvokeNative(uint32_t scriptHash, uint64_t nativeHash, const std::vector<PipeNativeArg>& args)
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::NATIVE_INVOKE);
        PipeClient::Send(&cmd, sizeof(cmd));

        PipeClient::Send(&scriptHash, sizeof(scriptHash));

        PipeClient::Send(&nativeHash, sizeof(nativeHash));

        for (const auto& arg : args)
            SendNativeArg(arg);

        // check if the native was successfully invoked
        bool success = false;
        if (!PipeClient::Receive(&success, sizeof(success)) || !success)
            return {};

        PipeNativeInvokeResult result;

        auto retTypesPtr = NativeDB::GetRetsByHash(nativeHash);
        auto retTypes = retTypesPtr ? *retTypesPtr : std::vector<NativeDB::Types>{};

        result.Returns.reserve(retTypes.size());

        for (auto type : retTypes)
        {
            PipeNativeReturn ret{};
            ret.Type = type;
            if (!ReceiveNativeRet(ret))
                break;

            result.Returns.push_back(std::move(ret));
        }

        for (const auto& arg : args)
        {
            if (arg.Type != NativeDB::Types::REFERENCE)
                continue;

            PipeNativeOutValue outValue{};
            bool ok = false;
            if (g_Game->Is64Bit())
                ok = PipeClient::Receive(&outValue.RefValue64, sizeof(outValue.RefValue64));
            else
                ok = PipeClient::Receive(&outValue.RefValue32, sizeof(outValue.RefValue32));

            if (!ok)
                break;

            result.OutValues.push_back(outValue);
        }

        return result;
    }

    void SetLoggerType(int type)
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::LOGGER_SET_TYPE);
        PipeClient::Send(&cmd, sizeof(cmd));

        PipeClient::Send(&type, sizeof(type));
    }

    void SetLoggerScript(uint32_t hash)
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::LOGGER_SET_SCRIPT);
        PipeClient::Send(&cmd, sizeof(cmd));

        PipeClient::Send(&hash, sizeof(hash));
    }

    void ClearLoggerFile()
    {
        uint8_t cmd = static_cast<uint8_t>(ePipeCommands::LOGGER_CLEAR_FILE);
        PipeClient::Send(&cmd, sizeof(cmd));
    }
}