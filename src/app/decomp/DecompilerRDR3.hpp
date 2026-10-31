#pragma once
#include "Decompiler.hpp"

namespace scrDbgApp
{
    class DecompilerRDR3 : public Decompiler
    {
    public:
        using Decompiler::Decompiler;

    protected:
        std::vector<Decomp::Instruction> Lift(const Decomp::Function& func) const override;
    };
}