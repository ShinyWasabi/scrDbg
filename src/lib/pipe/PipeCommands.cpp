#include "PipeCommands.hpp"
#include "core/PipeServer.hpp"
#include "debugger/VMLogger.hpp"

namespace scrDbgLib::PipeCommands
{
    void SetBreakpoint()
    {
        PipeBreakpointSet args{};
        PipeServer::Receive(&args, sizeof(args));

        g_Game->GetDebugger()->SetBreakpoint(args.Script, args.Pc, args.Set);
    }

    void BreakpointExists()
    {
        PipeBreakpoint args{};
        PipeServer::Receive(&args, sizeof(args));

        bool result = g_Game->GetDebugger()->BreakpointExists(args.Script, args.Pc);
        PipeServer::Send(&result, sizeof(result));
    }

    void ResumeBreakpoint()
    {
        g_Game->GetDebugger()->ResumeBreakpoint();
    }

    void SetBreakpointPauseGame()
    {
        bool pause = false;
        PipeServer::Receive(&pause, sizeof(pause));

        g_Game->GetDebugger()->SetPauseGameOnBreakpoint(pause);
    }

    void GetActiveBreakpoint()
    {
        auto active = g_Game->GetDebugger()->GetActiveBreakpoint();

        bool hasActive = active.has_value();
        PipeServer::Send(&hasActive, sizeof(hasActive));

        if (active)
        {
            PipeBreakpoint bp{active->ScriptHash, active->Pc};
            PipeServer::Send(&bp, sizeof(bp));
        }
    }

    void GetAllBreakpoints()
    {
        auto bps = g_Game->GetDebugger()->GetAllBreakpoints();

        uint32_t count = static_cast<uint32_t>(bps.size());
        PipeServer::Send(&count, sizeof(count));

        for (const auto& bp : bps)
        {
            PipeBreakpoint entry{bp.ScriptHash, bp.Pc};
            PipeServer::Send(&entry, sizeof(entry));
        }
    }

    void RemoveAllBreakpoints()
    {
        g_Game->GetDebugger()->RemoveAllBreakpoints();
    }

    void InvokeNative()
    {
        uint32_t scriptHash = 0;
        if (!PipeServer::Receive(&scriptHash, sizeof(scriptHash)))
            return;

        uint64_t nativeHash = 0;
        if (!PipeServer::Receive(&nativeHash, sizeof(nativeHash)))
            return;

        auto argTypesPtr = NativeDB::GetArgsByHash(nativeHash);
        auto argTypes = argTypesPtr ? *argTypesPtr : std::vector<NativeDB::Types>{};

        std::vector<std::string> stringArgs;
        stringArgs.reserve(argTypes.size());

        std::vector<rage::scrValue> refArgs;
        refArgs.reserve(argTypes.size());

        auto ctx = g_Game->GetDebugger()->CreateNativeContext();
        ctx->Reset();

        bool valid = true;
        for (size_t i = 0; i < argTypes.size() && valid; i++)
        {
            auto type = argTypes[i];

            switch (type)
            {
            case NativeDB::Types::INT:
            {
                rage::scrValue val{};
                valid = PipeServer::Receive(&val, sizeof(int32_t));
                ctx->PushArg(val);
                break;
            }
            case NativeDB::Types::BOOL:
            {
                rage::scrValue val{};
                valid = PipeServer::Receive(&val, sizeof(bool));
                ctx->PushArg(val);
                break;
            }
            case NativeDB::Types::FLOAT:
            {
                double d = 0.0;
                valid = PipeServer::Receive(&d, sizeof(double));
                rage::scrValue val{};
                val.Float = static_cast<float>(d);
                ctx->PushArg(val);
                break;
            }
            case NativeDB::Types::STRING:
            {
                uint32_t len = 0;
                std::string str;
                valid = PipeServer::Receive(&len, sizeof(len));
                if (valid && len > 0)
                {
                    str.resize(len);
                    valid = PipeServer::Receive(str.data(), len);
                }
                stringArgs.push_back(std::move(str));
                rage::scrValue val{};
                val.String = stringArgs.back().c_str();
                ctx->PushArg(val);
                break;
            }
            case NativeDB::Types::REFERENCE:
            {
                rage::scrValue backing{};
                valid = PipeServer::Receive(&backing, sizeof(void*));
                refArgs.push_back(backing);
                rage::scrValue ptrVal{};
                ptrVal.Reference = &refArgs.back();
                ctx->PushArg(ptrVal);
                break;
            }
            }
        }

        if (!valid)
            return;

        auto handler = g_Game->GetNativeHandler(nativeHash);
        if (!handler)
        {
            bool success = false;
            PipeServer::Send(&success, sizeof(success));
            return;
        }

        auto donePromise = std::make_shared<std::promise<void>>();
        std::future<void> doneFuture = donePromise->get_future();

        g_Game->GetDebugger()->PushNativeInvoke(scriptHash, handler, ctx.get(), donePromise);

        doneFuture.wait();

        bool success = true;
        PipeServer::Send(&success, sizeof(success));

        auto retTypesPtr = NativeDB::GetRetsByHash(nativeHash);
        auto retTypes = retTypesPtr ? *retTypesPtr : std::vector<NativeDB::Types>{};

        const auto retSlots = ctx->GetRets();

        for (size_t i = 0; i < retTypes.size(); i++)
        {
            auto type = retTypes[i];
            auto raw = retSlots[i];

            switch (type)
            {
            case NativeDB::Types::INT:
            {
                PipeServer::Send(&raw.Int, sizeof(raw.Int));
                break;
            }
            case NativeDB::Types::BOOL:
            {
                bool val = static_cast<bool>(raw.Any & 0xFF);
                PipeServer::Send(&val, sizeof(val));
                break;
            }
            case NativeDB::Types::FLOAT:
            {
                double val = static_cast<double>(raw.Float);
                PipeServer::Send(&val, sizeof(val));
                break;
            }
            case NativeDB::Types::STRING:
            {
                uint32_t len = raw.String ? static_cast<uint32_t>(std::strlen(raw.String)) : 0;
                PipeServer::Send(&len, sizeof(len));
                if (len > 0)
                    PipeServer::Send(raw.String, len);
                break;
            }
            }
        }

        size_t refIndex = 0;
        for (auto type : argTypes)
        {
            if (type != NativeDB::Types::REFERENCE)
                continue;

            const auto& backing = refArgs[refIndex++];
            PipeServer::Send(&backing.Any, sizeof(backing.Any));
        }
    }

    void SetLoggerType()
    {
        int type = 0;
        PipeServer::Receive(&type, sizeof(type));

        VMLogger::SetLogType(static_cast<VMLogType>(type));
    }

    void SetLoggerScript()
    {
        uint32_t hash = 0;
        PipeServer::Receive(&hash, sizeof(hash));

        VMLogger::SetScriptHash(hash);
    }

    void ClearLoggerFile()
    {
        VMLogger::Clear();
    }
}