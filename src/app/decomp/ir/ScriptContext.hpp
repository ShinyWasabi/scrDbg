#pragma once

namespace scrDbgApp::Decomp
{
    class Function;

    // Implemented by Decompiler, which resolves these against scrProgram
    class ScriptContext
    {
    public:
        virtual ~ScriptContext() = default;

        virtual const Function* FindCallTarget(uint32_t address) = 0;
        virtual std::string GetNativeName(uint32_t index) = 0;
        virtual std::string GetString(uint32_t index) = 0;
        virtual std::string GetStringComment(const std::string& value) = 0;                 // Optional comment for a string literal (for example its GXT text)
        virtual const std::vector<uint32_t>* GetIndirectCallTargets(uint32_t callSite) = 0; // Target addresses observed at runtime for CALLINDIRECT

        virtual int GetVectorWidth() const
        {
            return 3;
        }

        virtual uint32_t GetPointerSlotSize() const
        {
            return 0;
        }
    };
}