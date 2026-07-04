#pragma once

namespace scrDbgApp
{
    class ScriptProgram
    {
    public:
        virtual std::string GetName() const
        {
            return {};
        }

        virtual uint32_t GetGlobalVersion() const
        {
            return 0;
        }

        virtual uint32_t GetCodeSize() const
        {
            return 0;
        }

        virtual uint32_t GetArgCount() const
        {
            return 0;
        }

        virtual uint32_t GetStaticCount() const
        {
            return 0;
        }

        virtual uint32_t GetGlobalCount() const
        {
            return 0;
        }

        virtual uint32_t GetGlobalBlock() const
        {
            return 0;
        }

        virtual uint32_t GetNativeCount() const
        {
            return 0;
        }

        virtual uint32_t GetNameHash() const
        {
            return 0;
        }

        virtual uint32_t GetRefCount() const
        {
            return 0;
        }

        virtual uint32_t GetStringsSize() const
        {
            return 0;
        }

        virtual std::vector<uint8_t> GetCode() const
        {
            return {};
        }

        virtual void SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const
        {
        }

        virtual Pointer GetStatic(uint32_t index) const
        {
            return {};
        }

        virtual Pointer GetProgramGlobal(uint32_t index) const
        {
            return {};
        }

        virtual uint64_t GetNative(uint32_t index) const
        {
            return 0;
        }

        virtual std::vector<std::string> GetStrings() const
        {
            return {};
        }

        virtual std::string GetString(uint32_t index) const
        {
            return {};
        }

        virtual std::vector<uint32_t> FindStringIndices(const std::string& string) const
        {
            return {};
        }

        virtual bool IsRsc() const
        {
            return false;
        }

        virtual bool IsPTScript() const
        {
            return false;
        }
    };

    class ScriptThread
    {
    public:
        enum State : uint32_t
        {
            RUNNING,
            WAITING,
            KILLED,
            PAUSED,
            REFRESH,
            THROW
        };

        enum Priority : uint32_t
        {
            HIGHEST,
            NORMAL,
            LOWEST,
            MANUAL_UPDATE = 100
        };

        virtual uint32_t GetId() const
        {
            return 0;
        }

        virtual uint32_t GetProgramHash() const
        {
            return 0;
        }

        virtual State GetState() const
        {
            return State::RUNNING;
        }

        virtual void SetState(State state) const
        {
        }

        virtual uint32_t GetPc() const
        {
            return 0;
        }

        virtual uint32_t GetFp() const
        {
            return 0;
        }

        virtual uint32_t GetSp() const
        {
            return 0;
        }

        virtual uint32_t GetStackSize() const
        {
            return 0;
        }

        virtual uint32_t GetTypedFlags() const
        {
            return 0;
        }

        virtual Priority GetPriority() const
        {
            return Priority::HIGHEST;
        }

        virtual uint8_t GetCallDepth() const
        {
            return 0;
        }

        virtual uint32_t GetCallStack(uint32_t index) const
        {
            return 0;
        }

        virtual bool IsPatched() const
        {
            return false;
        }

        virtual Pointer GetStack(uint32_t index) const
        {
            return {};
        }

        virtual std::string GetCreateTime() const
        {
            return {};
        }

        virtual std::string GetKillReason() const
        {
            return {};
        }

        virtual uint32_t GetScriptHash() const
        {
            return 0;
        }

        virtual std::string GetScriptName() const
        {
            return {};
        }
    };
}