#include "FlatBuilder.hpp"
#include "decomp/ir/Function.hpp"

namespace scrDbgApp::Decomp
{
    std::vector<AstPtr> FlatBuilder::Build(Function& func, ScriptContext& context)
    {
        const auto& instructions = func.GetInstructions();

        std::unordered_set<uint32_t> targets;
        for (const auto& ins : instructions)
        {
            if (ins.Op >= IrOp::J && ins.Op <= IrOp::ILE_JZ)
                targets.insert(ins.Target);
            else if (ins.Op == IrOp::SWITCH)
                for (const auto& switchCase : ins.Cases)
                    targets.insert(switchCase.Target);
        }

        std::vector<AstPtr> statements;
        ScriptStack stack;
        ExpressionEvaluator evaluator(func, context, stack);

        for (const auto& ins : instructions)
        {
            if (targets.count(ins.Offset))
            {
                auto label = MakeAst<Label>(ins.Offset);
                label->Address = ins.Offset;
                statements.push_back(std::move(label));
            }

            if (evaluator.Evaluate(ins, statements))
                continue;

            const size_t firstNew = statements.size();

            if (ins.Op == IrOp::J)
            {
                statements.push_back(MakeAst<Jump>(ins.Target));
            }
            else if (ins.Op == IrOp::SWITCH)
            {
                std::vector<std::pair<int32_t, uint32_t>> cases;
                cases.reserve(ins.Cases.size());
                for (const auto& switchCase : ins.Cases)
                    cases.emplace_back(switchCase.Value, switchCase.Target);
                statements.push_back(MakeAst<SwitchJump>(stack.Pop(), std::move(cases)));
            }
            else
            {
                AstPtr condition = evaluator.PopCondition(ins);
                statements.push_back(MakeAst<ConditionalJump>(std::move(condition), ins.Target));
            }

            for (size_t i = firstNew; i < statements.size(); i++)
                statements[i]->Address = ins.Offset;
        }

        return statements;
    }
}