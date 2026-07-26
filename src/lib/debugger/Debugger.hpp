#pragma once
#include "NativeContext.hpp"

namespace scrDbgLib
{
    class Debugger
    {
    public:
        struct Breakpoint
        {
            uint32_t ScriptHash;
            uint32_t Pc;
        };

        virtual ~Debugger() = default;

        virtual bool ProcessBreakpoints(uint32_t scriptHash, uint32_t pc, uint32_t* state) = 0;
        virtual bool ResumeBreakpoint() = 0;
        virtual bool IsChainOpcode(uint8_t op) const = 0;

        void SetBreakpoint(uint32_t scriptHash, uint32_t pc, bool set);
        bool BreakpointExists(uint32_t scriptHash, uint32_t pc) const;
        void SetPauseGameOnBreakpoint(bool set);
        std::optional<Breakpoint> GetActiveBreakpoint() const;
        std::vector<Breakpoint> GetAllBreakpoints() const;
        void RemoveAllBreakpoints();

        // thread/program/globals required for MP3
        virtual std::string NativeLogFormat(rage::scrValue value, scrDbg::NativeDB::Types type, void* thread = nullptr, void* program = nullptr, rage::scrValue* globals = nullptr) const;
        void NativeLogBegin(uint32_t scriptHash, void* handler, rage::scrValue* args, uint32_t argCount, void* thread = nullptr, void* program = nullptr, rage::scrValue* globals = nullptr);
        void NativeLogEnd(const char* name, uint32_t pc, rage::scrValue* rets, uint32_t retCount, void* thread = nullptr, void* program = nullptr, rage::scrValue* globals = nullptr);
        void NativeLogClear();

        virtual std::unique_ptr<NativeContext> CreateNativeContext() const;
        virtual void PushNativeInvoke(uint32_t scriptHash, void* handler, NativeContext* ctx, std::shared_ptr<std::promise<void>> donePromise);

        void BeginTracking(uint32_t hash, uint32_t index, bool isGlobal);
        void AddFieldOffset(uint32_t offset);
        void AddArrayIndex(uint32_t index, uint32_t size);
        void FinalizeTracking(const char* name, uint32_t pc, int32_t value, bool isStruct = false);
        bool ShouldBreakTracking(uint8_t op) const;
        void BreakTracking();

    protected:
        virtual void PauseGame(bool pause) = 0;

        std::optional<Breakpoint> m_ActiveBreakpoint;
        std::vector<Breakpoint> m_Breakpoints;
        bool m_StepOverBreakpoint = false;
        bool m_PauseGameOnBreakpoint = false;
        bool m_GamePausedByBreakpoint = false;

        bool m_ShouldLogNative = false;
        uint64_t m_NativeLogHash = 0;
        std::string_view m_NativeLogName = {};
        std::string m_NativeLogArgs = {};
        std::string m_NativeLogRets = {};

        bool m_TrackerActive = false;
        bool m_TrackingGlobal = false;
        uint32_t m_TrackerVariableIndex = 0;
        char m_TrackerPathBuf[256] = {};
        std::size_t m_TrackerPathLen = 0;
    };
}