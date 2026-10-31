#pragma once
#include "Decompiler.hpp"

namespace scrDbgApp
{
    class DecompilerGTA4 : public Decompiler
    {
    public:
        using Decompiler::Decompiler;

    protected:
        std::vector<Decomp::Instruction> Lift(const Decomp::Function& func) const override;
        uint64_t GetNativeHash(uint32_t index) const override;

    public:
        uint32_t GetPointerSlotSize() const override
        {
            return 4;
        }
    };
}