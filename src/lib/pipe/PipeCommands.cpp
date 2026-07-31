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

    static bool ReceiveStructField(PipeStructField& field)
    {
        uint8_t type = 0;
        if (!PipeServer::Receive(&type, sizeof(type)))
            return false;

        field.Type = static_cast<ScriptStruct::FieldType>(type);

        switch (field.Type)
        {
        case ScriptStruct::FieldType::INT:
            return PipeServer::Receive(&field.IntValue, sizeof(field.IntValue));
        case ScriptStruct::FieldType::BOOL:
            return PipeServer::Receive(&field.BoolValue, sizeof(field.BoolValue));
        case ScriptStruct::FieldType::FLOAT:
            return PipeServer::Receive(&field.FloatValue, sizeof(field.FloatValue));
        case ScriptStruct::FieldType::STRING:
        case ScriptStruct::FieldType::TEXT_LABEL_7:
        case ScriptStruct::FieldType::TEXT_LABEL_15:
        case ScriptStruct::FieldType::TEXT_LABEL_23:
        case ScriptStruct::FieldType::TEXT_LABEL_31:
        case ScriptStruct::FieldType::TEXT_LABEL_63:
        {
            uint32_t len = 0;
            if (!PipeServer::Receive(&len, sizeof(len)))
                return false;

            if (len > 0)
            {
                field.StringValue.resize(len);
                if (!PipeServer::Receive(field.StringValue.data(), len))
                    return false;
            }

            return true;
        }
        }

        return false;
    }

    static void SendStructField(const PipeStructField& field)
    {
        uint8_t type = static_cast<uint8_t>(field.Type);
        PipeServer::Send(&type, sizeof(type));

        switch (field.Type)
        {
        case ScriptStruct::FieldType::INT:
            PipeServer::Send(&field.IntValue, sizeof(field.IntValue));
            break;
        case ScriptStruct::FieldType::BOOL:
            PipeServer::Send(&field.BoolValue, sizeof(field.BoolValue));
            break;
        case ScriptStruct::FieldType::FLOAT:
            PipeServer::Send(&field.FloatValue, sizeof(field.FloatValue));
            break;
        case ScriptStruct::FieldType::STRING:
        case ScriptStruct::FieldType::TEXT_LABEL_7:
        case ScriptStruct::FieldType::TEXT_LABEL_15:
        case ScriptStruct::FieldType::TEXT_LABEL_23:
        case ScriptStruct::FieldType::TEXT_LABEL_31:
        case ScriptStruct::FieldType::TEXT_LABEL_63:
        {
            uint32_t len = static_cast<uint32_t>(field.StringValue.size());
            PipeServer::Send(&len, sizeof(len));
            if (len > 0)
                PipeServer::Send(field.StringValue.data(), len);
            break;
        }
        }
    }

    // owns the scrValue slots for a single reference argument's struct
    struct BuiltStruct
    {
        std::vector<rage::scrValue> Slots;
        std::vector<ScriptStruct::FieldType> FieldTypes;
        std::deque<std::string> StringStorage;
    };

    static BuiltStruct BuildStructBuffer(const std::vector<PipeStructField>& fields)
    {
        BuiltStruct built;
        built.FieldTypes.reserve(fields.size());

        size_t totalSlots = 0;
        for (const auto& field : fields)
        {
            totalSlots += ScriptStruct::GetSlotCount(field.Type);
            built.FieldTypes.push_back(field.Type);
        }

        built.Slots.resize(totalSlots);

        size_t slotOffset = 0;
        for (const auto& field : fields)
        {
            size_t fieldSlots = ScriptStruct::GetSlotCount(field.Type);
            rage::scrValue* slot = &built.Slots[slotOffset];

            switch (field.Type)
            {
            case ScriptStruct::FieldType::INT:
                slot->Int = field.IntValue;
                break;
            case ScriptStruct::FieldType::BOOL:
                slot->Int = field.BoolValue ? 1 : 0;
                break;
            case ScriptStruct::FieldType::FLOAT:
                slot->Float = static_cast<float>(field.FloatValue);
                break;
            case ScriptStruct::FieldType::STRING:
                built.StringStorage.push_back(field.StringValue);
                slot->String = built.StringStorage.back().c_str();
                break;
            case ScriptStruct::FieldType::TEXT_LABEL_7:
            case ScriptStruct::FieldType::TEXT_LABEL_15:
            case ScriptStruct::FieldType::TEXT_LABEL_23:
            case ScriptStruct::FieldType::TEXT_LABEL_31:
            case ScriptStruct::FieldType::TEXT_LABEL_63:
            {
                size_t dataSize = ScriptStruct::GetDataSize(field.Type);
                char* dst = reinterpret_cast<char*>(slot);
                std::memset(dst, 0, fieldSlots * sizeof(rage::scrValue));

                size_t copyLen = std::min(field.StringValue.size(), dataSize - 1);
                std::memcpy(dst, field.StringValue.data(), copyLen);
                break;
            }
            }

            slotOffset += fieldSlots;
        }

        return built;
    }

    static PipeStructField ExtractStructFieldOut(const rage::scrValue* slot, ScriptStruct::FieldType type)
    {
        PipeStructField out;
        out.Type = type;

        switch (type)
        {
        case ScriptStruct::FieldType::INT:
            out.IntValue = slot->Int;
            break;
        case ScriptStruct::FieldType::BOOL:
            out.BoolValue = (slot->Any & 0xFF) != 0;
            break;
        case ScriptStruct::FieldType::FLOAT:
            out.FloatValue = static_cast<double>(slot->Float);
            break;
        case ScriptStruct::FieldType::STRING:
            out.StringValue = slot->String ? std::string(slot->String) : std::string{};
            break;
        case ScriptStruct::FieldType::TEXT_LABEL_7:
        case ScriptStruct::FieldType::TEXT_LABEL_15:
        case ScriptStruct::FieldType::TEXT_LABEL_23:
        case ScriptStruct::FieldType::TEXT_LABEL_31:
        case ScriptStruct::FieldType::TEXT_LABEL_63:
        {
            size_t dataSize = ScriptStruct::GetDataSize(type);
            const char* data = reinterpret_cast<const char*>(slot);
            size_t len = strnlen(data, dataSize);
            out.StringValue.assign(data, len);
            break;
        }
        }

        return out;
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

        std::vector<BuiltStruct> structArgs;
        structArgs.reserve(argTypes.size());

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
                uint32_t fieldCount = 0;
                valid = PipeServer::Receive(&fieldCount, sizeof(fieldCount));

                std::vector<PipeStructField> fields;
                fields.reserve(fieldCount);

                for (uint32_t f = 0; f < fieldCount && valid; f++)
                {
                    PipeStructField field;
                    valid = ReceiveStructField(field);
                    if (valid)
                        fields.push_back(std::move(field));
                }

                if (!valid)
                    break;

                structArgs.push_back(BuildStructBuffer(fields));

                rage::scrValue ptrVal{};
                ptrVal.Reference = structArgs.back().Slots.data();
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

        auto promise = std::make_shared<std::promise<void>>();
        std::future<void> future = promise->get_future();

        g_Game->GetDebugger()->PushNativeInvoke(scriptHash, handler, ctx.get(), promise);
        if (future.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
        {
            bool success = false;
            PipeServer::Send(&success, sizeof(success));
            return;
        }

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

        size_t structIndex = 0;
        for (auto type : argTypes)
        {
            if (type != NativeDB::Types::REFERENCE)
                continue;

            const auto& built = structArgs[structIndex++];

            uint32_t fieldCount = static_cast<uint32_t>(built.FieldTypes.size());
            PipeServer::Send(&fieldCount, sizeof(fieldCount));

            size_t slotOffset = 0;
            for (auto fieldType : built.FieldTypes)
            {
                size_t fieldSlots = ScriptStruct::GetSlotCount(fieldType);
                PipeStructField outField = ExtractStructFieldOut(&built.Slots[slotOffset], fieldType);
                SendStructField(outField);
                slotOffset += fieldSlots;
            }
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