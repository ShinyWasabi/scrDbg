#pragma once
#include "decomp/ir/ScriptContext.hpp"
#include "decomp/output/DecompiledFunction.hpp"
#include "disasm/Disassembler.hpp"

namespace scrDbgApp
{
    namespace Decomp
    {
        class Function;
        struct Instruction;
    }

    // base class for per-game decompilers
    class Decompiler : public Decomp::ScriptContext
    {
    public:
        explicit Decompiler(const Disassembler& disassembler);
        ~Decompiler() override;

        int GetFunctionCount();
        std::string GetFunctionName(int index);

        int GetFunctionIndexForPc(uint32_t pc);

        Decomp::DecompiledFunction DecompileFunction(int index); // Full pseudo C for one function

        uint32_t GetFunctionStart(int index);

        std::string GetScriptHeader();

        const Decomp::Function* FindCallTarget(uint32_t address) override;
        std::string GetNativeName(uint32_t index) override;
        std::string GetString(uint32_t index) override;
        std::string GetStringComment(const std::string& value) override;
        const std::vector<uint32_t>* GetIndirectCallTargets(uint32_t callSite) override;

    protected:
        virtual std::vector<Decomp::Instruction> Lift(const Decomp::Function& func) const = 0; // Decodes [func.GetStart(), func.GetEnd()) into IR

        virtual uint32_t GetReservedFrameSlots() const
        {
            return 2;
        }

        virtual uint64_t GetNativeHash(uint32_t index) const;

        // CALLINDIRECT targets observed by scrDbgLib for this script
        virtual std::unordered_map<uint32_t, std::vector<uint32_t>> FetchIndirectCalls() const;

        const Disassembler& m_Disassembler;

    private:
        void EnsureFunctions();

        std::vector<std::unique_ptr<Decomp::Function>> m_Functions;
        bool m_FunctionsBuilt;
        std::unordered_map<uint32_t, std::string> m_NativeNames;
        std::unordered_map<uint32_t, std::string> m_Strings;
        std::unordered_map<std::string, std::string> m_StringComments;
        std::optional<std::unordered_map<uint32_t, std::vector<uint32_t>>> m_IndirectCalls; // fetched once, on first use
    };
}