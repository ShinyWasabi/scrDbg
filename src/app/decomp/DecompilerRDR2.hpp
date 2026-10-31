#pragma once
#include "Decompiler.hpp"

namespace scrDbgApp
{
    class DecompilerRDR2 : public Decompiler
    {
    public:
        using Decompiler::Decompiler;

        uint32_t GetPointerSlotSize() const override
        {
            return 4; // statics, globals and field offsets are in 4 byte units, even though the stack is 8 bytes
        }

        int GetVectorWidth() const override
        {
            return 2; // x and y packed in one 8 byte slot
        }

    protected:
        std::vector<Decomp::Instruction> Lift(const Decomp::Function& func) const override;
    };
}