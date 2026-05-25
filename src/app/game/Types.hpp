#pragma once

namespace scrDbgApp
{
    class ScriptProgram
    {
    public:
        virtual uint32_t GetGlobalVersion() const = 0;
        virtual uint32_t GetCodeSize() const = 0;
        virtual uint32_t GetArgCount() const = 0;
        virtual uint32_t GetStaticCount() const = 0;
        virtual uint32_t GetGlobalCount() const = 0;
        virtual uint32_t GetGlobalBlock() const = 0;
        virtual uint32_t GetNativeCount() const = 0;
        virtual uint32_t GetNameHash() const = 0;
        virtual uint32_t GetRefCount() const = 0;
        virtual uint32_t GetStringsSize() const = 0;
        virtual std::vector<uint8_t> GetCode() const = 0;
        virtual void SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const = 0;
        virtual int32_t GetStatic(uint32_t index) const = 0;
        virtual int32_t GetProgramGlobal(uint32_t index) const = 0;
        virtual uint64_t GetNative(uint32_t index) const = 0;
        virtual std::vector<std::string> GetStrings() const = 0;
        virtual std::string GetString(uint32_t index) const = 0;
        virtual std::vector<uint32_t> FindStringIndices(const std::string& string) const = 0;
    };

    class ScriptThread
    {
    public:
        enum State : uint32_t
        {
            RUNNING,
            IDLE,
            KILLED,
            PAUSED,
            REFRESH
        };

        enum Priority : uint32_t
        {
            HIGHEST,
            NORMAL,
            LOWEST,
            MANUAL_UPDATE = 100
        };

        virtual uint32_t GetId() const = 0;
        virtual uint32_t GetProgramHash() const = 0;
        virtual State GetState() const = 0;
        virtual void SetState(State state) const = 0;
        virtual uint32_t GetPc() const = 0;
        virtual uint32_t GetFp() const = 0;
        virtual uint32_t GetSp() const = 0;
        virtual uint32_t GetStackSize() const = 0;
        virtual Priority GetPriority() const = 0;
        virtual uint8_t GetCallDepth() const = 0;
        virtual uint32_t GetCallStack(uint32_t index) const = 0;
        virtual int32_t GetStack(uint32_t index) const = 0;
        virtual void SetStack(uint32_t index, int32_t value) const = 0;
        virtual std::string GetCreateTime() const = 0;
        virtual std::string GetExitReason() const = 0;
        virtual uint32_t GetScriptHash() const = 0;
        virtual std::string GetScriptName() const = 0;
    };
}