#pragma once
#include "decomp/ast/Stack.hpp"
#include "decomp/ir/Instruction.hpp"

namespace scrDbgApp::Decomp
{
    class Function;
    class ScriptContext;

    // Simulates every non-control-flow instruction on a ScriptStack
    class ExpressionEvaluator
    {
    public:
        ExpressionEvaluator(const Function& func, ScriptContext& context, ScriptStack& stack)
            : m_Func(func),
              m_Context(context),
              m_Stack(stack)
        {
        }

        bool Evaluate(const Instruction& ins, std::vector<AstPtr>& statements); // Returns false for control-flow instructions (like J, JZ, *_JZ, SWITCH)
        AstPtr PopCondition(const Instruction& ins);

    private:
        bool EvaluateInternal(const Instruction& ins, std::vector<AstPtr>& statements);
        void PushBinary(const char* symbol);
        void PushCompare(const char* op);
        void PushVectorBinary(const char* symbol);
        void PushCall(std::shared_ptr<FunctionCallBase> call, std::vector<AstPtr>& statements);
        int PopConstantCount(const char* opName);

        const Function& m_Func;
        ScriptContext& m_Context;
        ScriptStack& m_Stack;
    };
}