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
        bool Set = false;
    };

    struct PipeStructField
    {
        ScriptStruct::FieldType Type = ScriptStruct::FieldType::INT;
        int32_t IntValue = 0;
        bool BoolValue = false;
        double FloatValue = 0.0;
        std::string StringValue; // used for string and text label
    };

    struct PipeNativeArg
    {
        NativeDB::Types Type = NativeDB::Types::NONE;
        int32_t IntValue = 0;
        double FloatValue = 0.0;
        bool BoolValue = false;
        std::string StringValue;
        std::vector<PipeStructField> StructFields; // used when type is ref
    };

    struct PipeNativeReturn
    {
        NativeDB::Types Type = NativeDB::Types::NONE;
        int32_t IntValue = 0;
        double FloatValue = 0.0;
        bool BoolValue = false;
        std::string StringValue;
    };

    struct PipeNativeOutValue
    {
        std::vector<PipeStructField> Fields; // updated struct values after the call
    };

    struct PipeNativeInvokeResult
    {
        std::vector<PipeNativeReturn> Returns;
        std::vector<PipeNativeOutValue> OutValues;
    };

    extern void SetBreakpoint(uint32_t script, uint32_t pc, bool set);
    extern bool BreakpointExists(uint32_t script, uint32_t pc);
    extern void ResumeBreakpoint();
    extern void SetBreakpointPauseGame(bool pause);
    extern std::optional<std::pair<uint32_t, uint32_t>> GetActiveBreakpoint();
    extern std::vector<std::pair<uint32_t, uint32_t>> GetAllBreakpoints();
    extern void RemoveAllBreakpoints();

    extern PipeNativeInvokeResult InvokeNative(uint32_t scriptHash, uint64_t nativeHash, const std::vector<PipeNativeArg>& args);

    extern void SetLoggerType(int type);
    extern void SetLoggerScript(uint32_t hash);
    extern void ClearLoggerFile();
}