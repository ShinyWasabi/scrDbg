#pragma once

#if defined(_M_IX86)

namespace scrDbgLib
{
    class DebuggerPayne : public Debugger
    {
    public:
        bool ProcessBreakpoints(uint32_t scriptHash, uint32_t pc, uint32_t* state) override;
        bool ResumeBreakpoint() override;
        bool IsChainOpcode(uint8_t op) const override;

        std::string NativeLogFormat(rage::scrValue value, scrDbg::NativeDB::Types type, void* thread = nullptr, void* program = nullptr, rage::scrValue* globals = nullptr) const override;

        std::unique_ptr<NativeContext> CreateNativeContext() const override;

    protected:
        void PauseGame(bool pause) override;
    };
}

#endif