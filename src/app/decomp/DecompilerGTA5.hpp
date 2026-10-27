#pragma once
#include "Decompiler.hpp"

namespace scrDbgApp
{
    class DecompilerGTA5 : public Decompiler
    {
    public:
        using Decompiler::Decompiler;

    protected:
        std::vector<Decomp::Instruction> Lift(const Decomp::Function& func) const override;
    };
}