#include "StatementTreeBuilder.hpp"
#include "decomp/ir/Function.hpp"

namespace scrDbgApp::Decomp
{
    StatementTreeBuilder::StatementTreeBuilder(Function& func, ScriptContext& context)
        : m_Func(func),
          m_Evaluator(func, context, m_Stack),
          m_Steps(0),
          m_MaxSteps(func.GetInstructions().size() * 2) // Every instruction is visited a limited number of times.
    {
    }

    TreePtr StatementTreeBuilder::Build(Function& func, ScriptContext& context)
    {
        if (func.GetInstructions().empty())
            throw std::runtime_error("Function has no instructions");

        StatementTreeBuilder builder(func, context);
        builder.FindDoWhileLoops();
        auto main = std::make_shared<Main>(func);
        builder.Decode(*main);
        return main;
    }

    int32_t StatementTreeBuilder::IndexOf(uint32_t address, const char* what) const
    {
        int32_t index = m_Func.GetInstructionIndexAtOffset(address);
        if (index < 0)
        {
            char buf[128];
            std::snprintf(buf, sizeof(buf), "%s 0x%X does not land on an instruction boundary", what, address);
            throw std::runtime_error(buf);
        }
        return index;
    }

    void StatementTreeBuilder::FindDoWhileLoops()
    {
        const auto& instructions = m_Func.GetInstructions();

        for (int32_t i = 0; i < static_cast<int32_t>(instructions.size()); i++)
        {
            const Instruction& ins = instructions[i];
            if (!ins.IsConditionalJump() || ins.Target > ins.Offset)
                continue;

            int32_t head = m_Func.GetInstructionIndexAtOffset(ins.Target);
            if (head >= 0)
                m_DoWhileTails[head].push_back(i);
        }

        for (auto& [head, tails] : m_DoWhileTails)
            std::sort(tails.begin(), tails.end(), std::greater<>());
    }

    bool StatementTreeBuilder::EnterDoWhile(Tree*& tree, std::vector<Tree*>& treeStack)
    {
        auto it = m_DoWhileTails.find(tree->Offset);
        if (it == m_DoWhileTails.end())
            return false;

        const auto& instructions = m_Func.GetInstructions();
        std::vector<int32_t> tails = std::move(it->second);
        m_DoWhileTails.erase(it);

        for (int32_t tail : tails)
        {
            auto loop = std::make_shared<DoWhile>(m_Func, tree, tree->Offset, tail, instructions[tail].NextOffset());
            tree->Statements.emplace_back(loop);
            tree->Offset = tail + 1;

            treeStack.push_back(loop.get());
            tree = loop.get();
        }

        return true;
    }

    void StatementTreeBuilder::Decode(Tree& root)
    {
        auto& instructions = m_Func.GetInstructions();
        const int32_t count = static_cast<int32_t>(instructions.size());

        std::vector<Tree*> treeStack{&root};
        Tree* tree = &root;
        std::vector<AstPtr> emitted;

        while (true)
        {
            if (++m_Steps > m_MaxSteps)
                throw std::runtime_error("Control flow reconstruction did not converge");

            if (tree->Offset < 0 || tree->Offset >= count)
                throw std::runtime_error("Ran past the end of the function");

            EnterDoWhile(tree, treeStack);

            const Instruction& ins = instructions[tree->Offset];

            emitted.clear();
            if (m_Evaluator.Evaluate(ins, emitted))
            {
                for (auto& statement : emitted)
                    tree->Statements.emplace_back(std::move(statement));
            }
            else if (ins.Op == IrOp::SWITCH)
            {
                HandleSwitch(*tree);
            }
            else if (ins.Op == IrOp::J)
            {
                HandleJump(tree, treeStack, ins);
            }
            else
            {
                HandleConditionalJump(tree, treeStack, ins);
            }

            if (tree == treeStack.back())
                tree->Offset++;
            else
                tree = treeStack.back();

            while (tree->IsTreeEnd())
            {
                treeStack.pop_back();
                if (treeStack.empty())
                    return;

                tree = treeStack.back();
                TryOptimizeTree(*tree);
            }
        }
    }

    void StatementTreeBuilder::HandleJump(Tree* tree, std::vector<Tree*>& treeStack, const Instruction& ins)
    {
        auto& instructions = m_Func.GetInstructions();
        const int32_t count = static_cast<int32_t>(instructions.size());
        const uint32_t target = ins.Target;

        // A jump to the break point of an enclosing case/while is a break
        for (Tree* t = tree; t; t = t->Parent)
        {
            auto caseTree = dynamic_cast<Case*>(t);
            auto whileTree = dynamic_cast<While*>(t);
            auto doWhileTree = dynamic_cast<DoWhile*>(t);
            if ((caseTree && target == caseTree->BreakAddress) || (whileTree && target == whileTree->BreakAddress) || (doWhileTree && target == doWhileTree->BreakAddress))
            {
                auto breakStatement = MakeAst<Break>();
                breakStatement->Address = ins.Offset;
                tree->Statements.emplace_back(std::move(breakStatement));
                return;
            }
        }

        // An if-body ending in a forward jump over the code that directly follows it means that code is an else block ending at the jump target
        auto ifTree = dynamic_cast<If*>(tree);
        if (ifTree && tree->Offset + 1 < count && instructions[tree->Offset + 1].Offset == ifTree->EndAddress && target > ins.Offset && target != instructions[tree->Offset + 1].Offset)
        {
            int32_t resumeAt = IndexOf(target, "Else end");

            auto elseTree = std::make_shared<Else>(m_Func, tree, tree->Offset + 1, target);
            ifTree->ElseTree = elseTree;
            treeStack.push_back(elseTree.get());

            if (tree->Parent)
                tree->Parent->Offset = resumeAt;
            tree->Offset = resumeAt;
            return;
        }

        // Jumps over padding (NOPs) to the very next real instruction are no-ops, anything else can't be structured and is kept as goto
        for (int32_t index = tree->Offset + 1; index < count; index++)
        {
            const Instruction& next = instructions[index];
            if (next.Offset == target)
                return;

            if (next.Op == IrOp::NOP || (next.Op == IrOp::J && next.IsNullJump()))
                continue;

            auto jump = MakeAst<Jump>(target);
            jump->Address = ins.Offset;
            tree->Statements.emplace_back(std::move(jump));
            return;
        }
    }

    void StatementTreeBuilder::HandleConditionalJump(Tree* tree, std::vector<Tree*>& treeStack, const Instruction& ins)
    {
        auto& instructions = m_Func.GetInstructions();

        AstPtr condition = m_Evaluator.PopCondition(ins);
        const uint32_t target = ins.Target;

        if (auto loop = dynamic_cast<DoWhile*>(tree); loop && tree->Offset == loop->TailIndex)
        {
            loop->Condition = NegateCondition(condition);
            loop->TailAddresses = {ins.Offset};
            loop->Finished = true;
            return;
        }

        if (ins.IsNullJump() || m_Func.GetInstructionIndexAtOffset(target) == tree->Offset + 1)
        {
            if (condition->HasSideEffects())
            {
                auto drop = MakeAst<Drop>(condition);
                drop->Address = ins.Offset;
                tree->Statements.emplace_back(std::move(drop));
            }
            return;
        }

        for (Tree* t = tree; t; t = t->Parent)
        {
            auto whileTree = dynamic_cast<While*>(t);
            auto doWhileTree = dynamic_cast<DoWhile*>(t);
            if ((whileTree && target == whileTree->BreakAddress) || (doWhileTree && target == doWhileTree->BreakAddress))
            {
                auto condBreak = std::make_shared<If>(m_Func, tree, -1, NegateCondition(condition), 0);
                condBreak->HeaderAddresses = {ins.Offset}; // the JZ is the "if" line; "break;" has no instruction of its own
                condBreak->Statements.emplace_back(MakeAst<Break>());
                tree->Statements.emplace_back(std::move(condBreak));
                return;
            }
        }

        int32_t targetIndex = IndexOf(target, "Conditional jump target");

        if (targetIndex > 0)
        {
            Instruction& beforeTarget = instructions[targetIndex - 1];
            if (beforeTarget.IsBackwardJump() && beforeTarget.Target < ins.Offset)
            {
                if (targetIndex <= tree->Offset)
                    throw std::runtime_error("Loop exit is not after the loop condition");

                beforeTarget.Nop(); // implied by the while

                auto whileTree = std::make_shared<While>(m_Func, tree, tree->Offset + 1, condition, target);
                whileTree->HeaderAddresses = {ins.Offset, beforeTarget.Offset};
                treeStack.push_back(whileTree.get());
                tree->Statements.emplace_back(std::move(whileTree));
                tree->Offset = targetIndex;
                return;
            }
        }

        if (target <= ins.Offset)
            throw std::runtime_error("Backward conditional jump does not close an enclosing do-while");

        auto ifTree = std::make_shared<If>(m_Func, tree, tree->Offset + 1, condition, target);
        ifTree->HeaderAddresses = {ins.Offset};
        treeStack.push_back(ifTree.get());
        tree->Statements.emplace_back(std::move(ifTree));
        tree->Offset = targetIndex;
    }

    void StatementTreeBuilder::HandleSwitch(Tree& tree)
    {
        auto& instructions = m_Func.GetInstructions();
        const int32_t count = static_cast<int32_t>(instructions.size());
        const Instruction& switchIns = instructions[tree.Offset];

        // Group case labels by target, first-seen order
        std::vector<uint32_t> order;
        std::unordered_map<uint32_t, std::vector<AstPtr>> grouped;
        for (const auto& switchCase : switchIns.Cases)
        {
            if (IndexOf(switchCase.Target, "Switch case target") <= tree.Offset)
                throw std::runtime_error("Switch case target goes backwards");

            auto [it, inserted] = grouped.try_emplace(switchCase.Target);
            if (inserted)
                order.push_back(switchCase.Target);
            it->second.push_back(MakeAst<ConstantInt>(switchCase.Value));
        }

        std::vector<uint32_t> sorted = order;
        std::sort(sorted.begin(), sorted.end());

        // The jump right after the switch is taken when no case matches
        int32_t defaultIndex = tree.Offset + 1;
        while (defaultIndex < count && instructions[defaultIndex].OriginalOp == IrOp::NOP)
            defaultIndex++;

        if (defaultIndex >= count || instructions[defaultIndex].Op != IrOp::J)
            throw std::runtime_error("Switch is not followed by a default jump");

        const uint32_t defaultLoc = instructions[defaultIndex].Target;
        const uint32_t defaultJumpAddress = instructions[defaultIndex].Offset;
        const uint32_t switchAddress = switchIns.Offset;
        instructions[defaultIndex].Nop();

        uint32_t breakLoc = defaultLoc;
        bool useDefault = true;

        for (size_t i = 0; i <= sorted.size(); i++)
        {
            uint32_t address = (i == sorted.size()) ? defaultLoc : sorted[i];
            int32_t index = m_Func.GetInstructionIndexAtOffset(address) - 1;

            if (index - 1 == tree.Offset)
                continue;
            if (index < 0 || index >= count)
                continue;

            const Instruction& candidate = instructions[index];
            if (candidate.Op != IrOp::J)
                continue;

            if (candidate.Target == defaultLoc)
            {
                useDefault = false;
                breakLoc = defaultLoc;
                break;
            }

            breakLoc = candidate.Target;
        }

        if (useDefault)
        {
            auto [it, inserted] = grouped.try_emplace(defaultLoc);
            if (inserted)
                order.push_back(defaultLoc);
            it->second.push_back(MakeAst<Default>());
        }

        int32_t breakIndex = IndexOf(breakLoc, "Switch break target");
        if (breakIndex <= tree.Offset)
            throw std::runtime_error("Switch break target goes backwards");

        std::vector<std::pair<uint32_t, std::vector<AstPtr>>> cases;
        cases.reserve(order.size());
        for (uint32_t target : order)
            cases.emplace_back(target, std::move(grouped[target]));

        auto switchTree = std::make_shared<Switch>(m_Func, &tree, tree.Offset, std::move(cases), breakLoc, m_Stack.Pop());
        switchTree->HeaderAddresses = {switchAddress, defaultJumpAddress};

        for (auto& statement : switchTree->Statements)
            Decode(*std::get<TreePtr>(statement));

        tree.Statements.emplace_back(std::move(switchTree));
        tree.Offset = breakIndex - 1;
    }

    // if (a) {...} else { if (b) {...} else {...} } -> if (a) {...} else if (b) {...} else {...}
    void StatementTreeBuilder::TryOptimizeTree(Tree& tree)
    {
        if (tree.Statements.empty() || !std::holds_alternative<TreePtr>(tree.Statements.back()))
            return;

        auto lastIf = dynamic_cast<If*>(std::get<TreePtr>(tree.Statements.back()).get());
        if (!lastIf || !lastIf->ElseTree)
            return;

        auto& elseStatements = lastIf->ElseTree->Statements;
        if (elseStatements.size() != 1 || !std::holds_alternative<TreePtr>(elseStatements.back()))
            return;

        TreePtr inner = std::get<TreePtr>(elseStatements.back());
        auto insideIf = dynamic_cast<If*>(inner.get());
        if (!insideIf)
            return;

        auto elseIf = std::make_shared<ElseIf>(m_Func, lastIf, insideIf->Condition);
        elseIf->HeaderAddresses = insideIf->HeaderAddresses;
        elseIf->Statements = insideIf->Statements;
        lastIf->ElseIfTrees.push_back(elseIf);

        for (auto& nested : insideIf->ElseIfTrees)
        {
            nested->Parent = lastIf;
            lastIf->ElseIfTrees.push_back(nested);
        }

        lastIf->ElseTree = insideIf->ElseTree;
        if (lastIf->ElseTree)
            lastIf->ElseTree->Parent = lastIf;
    }
}