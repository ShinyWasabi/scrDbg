#pragma once
#include "ExpressionEvaluator.hpp"
#include "decomp/ast/Tree.hpp"

namespace scrDbgApp::Decomp
{
    // Reconstructs if/else/else-if/while/switch from the jump structure while building expressions through ExpressionEvaluator.
    class StatementTreeBuilder
    {
    public:
        static TreePtr Build(Function& func, ScriptContext& context);

    private:
        StatementTreeBuilder(Function& func, ScriptContext& context);

        void Decode(Tree& root);
        void HandleJump(Tree* tree, std::vector<Tree*>& treeStack, const Instruction& ins);
        void HandleConditionalJump(Tree* tree, std::vector<Tree*>& treeStack, const Instruction& ins);
        void HandleSwitch(Tree& tree);
        void TryOptimizeTree(Tree& tree);
        void FindDoWhileLoops();
        bool EnterDoWhile(Tree*& tree, std::vector<Tree*>& treeStack);

        int32_t IndexOf(uint32_t address, const char* what) const;

        Function& m_Func;
        ScriptStack m_Stack;
        ExpressionEvaluator m_Evaluator;
        size_t m_Steps;
        size_t m_MaxSteps;

        std::unordered_map<int32_t, std::vector<int32_t>> m_DoWhileTails; // Instruction index of a do-while's first body instruction
    };
}