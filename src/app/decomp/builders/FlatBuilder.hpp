#pragma once
#include "ExpressionEvaluator.hpp"

namespace scrDbgApp::Decomp
{
    class FlatBuilder
    {
    public:
        static std::vector<AstPtr> Build(Function& func, ScriptContext& context);
    };
}