#include "Tree.hpp"
#include "decomp/ir/Function.hpp"
#include "decomp/output/CodeWriter.hpp"

namespace scrDbgApp::Decomp
{
    template <typename... T>
    static bool IsLeafOf(const StatementItem& item)
    {
        if (!std::holds_alternative<AstPtr>(item))
            return false;

        auto token = std::get<AstPtr>(item).get();
        return ((dynamic_cast<T*>(token) != nullptr) || ...);
    }

    bool Tree::ReachedAddress(uint32_t address) const
    {
        const auto& instructions = Func.GetInstructions();
        return Offset >= 0 && Offset < static_cast<int32_t>(instructions.size()) && instructions[Offset].Offset >= address;
    }

    void Tree::EmitBody(CodeWriter& writer, int indent) const
    {
        bool lastIsTree = false;
        bool first = true;

        for (const auto& item : Statements)
        {
            if (std::holds_alternative<AstPtr>(item))
            {
                const auto& token = std::get<AstPtr>(item);

                if (lastIsTree && !SuppressBlankBeforeLeaf(token))
                    writer.Blank();

                std::string text = token->Render();
                if (!text.empty())
                    writer.Line(indent, text);

                lastIsTree = false;
            }
            else
            {
                if (!first)
                    writer.Blank();

                std::get<TreePtr>(item)->Emit(writer, indent);
                lastIsTree = true;
            }

            first = false;
        }
    }

    void Tree::EmitBlock(CodeWriter& writer, int indent, bool skipBraces) const
    {
        if (skipBraces)
        {
            EmitBody(writer, indent + 1);
            return;
        }

        writer.Line(indent, "{");
        EmitBody(writer, indent + 1);
        writer.Line(indent, "}");
    }

    bool Main::IsTreeEnd()
    {
        return !Statements.empty() && IsLeafOf<Return>(Statements.back());
    }

    bool If::IsTreeEnd()
    {
        if (ElseTree)
            return true;
        if (ReachedAddress(EndAddress))
            return true;
        return !Statements.empty() && IsLeafOf<Break, Jump>(Statements.back());
    }

    bool If::CanSkipBraces() const
    {
        auto singleSimple = [](const std::vector<StatementItem>& statements) {
            if (statements.size() != 1)
                return false;
            if (std::holds_alternative<AstPtr>(statements[0]))
                return true;
            auto nested = dynamic_cast<If*>(std::get<TreePtr>(statements[0]).get());
            return nested && nested->CanSkipBraces();
        };

        if (!singleSimple(Statements))
            return false;

        if (ElseTree && !singleSimple(ElseTree->Statements))
            return false;

        for (const auto& elseIf : ElseIfTrees)
        {
            if (!singleSimple(elseIf->Statements))
                return false;
        }

        return true;
    }

    void If::Emit(CodeWriter& writer, int indent) const
    {
        writer.Line(indent, "if (" + Condition->Render() + ")", HeaderAddresses);
        EmitBlock(writer, indent, CanSkipBraces());

        for (const auto& elseIf : ElseIfTrees)
            elseIf->Emit(writer, indent);

        if (ElseTree)
            ElseTree->Emit(writer, indent);
    }

    bool Else::IsTreeEnd()
    {
        if (ReachedAddress(EndAddress))
            return true;
        return !Statements.empty() && IsLeafOf<Break, Jump>(Statements.back());
    }

    void Else::Emit(CodeWriter& writer, int indent) const
    {
        writer.Line(indent, "else");
        EmitBlock(writer, indent, static_cast<If*>(Parent)->CanSkipBraces());
    }

    void ElseIf::Emit(CodeWriter& writer, int indent) const
    {
        writer.Line(indent, "else if (" + Condition->Render() + ")", HeaderAddresses);
        EmitBlock(writer, indent, static_cast<If*>(Parent)->CanSkipBraces());
    }

    bool While::IsTreeEnd()
    {
        if (ReachedAddress(BreakAddress))
            return true;
        return !Statements.empty() && IsLeafOf<Break, Jump>(Statements.back());
    }

    void While::Emit(CodeWriter& writer, int indent) const
    {
        writer.Line(indent, "while (" + Condition->Render() + ")", HeaderAddresses);
        EmitBlock(writer, indent, false);
    }

    void DoWhile::Emit(CodeWriter& writer, int indent) const
    {
        writer.Line(indent, "do");
        writer.Line(indent, "{");
        EmitBody(writer, indent + 1);
        writer.Line(indent, "} while (" + Condition->Render() + ");", TailAddresses);
    }

    Switch::Switch(Function& function, Tree* parent, int32_t offset, std::vector<std::pair<uint32_t, std::vector<AstPtr>>> cases, uint32_t breakAddress, AstPtr value)
        : Tree(function, parent, offset),
          BreakAddress(breakAddress),
          Value(std::move(value))
    {
        Case* lastCase = nullptr;

        for (auto& [target, labels] : cases)
        {
            int32_t caseStart = function.GetInstructionIndexAtOffset(target);
            if (caseStart < 0)
                throw std::runtime_error("Switch case target does not land on an instruction boundary");

            if (lastCase)
                lastCase->EndIndex = caseStart;

            auto caseTree = std::make_shared<Case>(function, this, caseStart, std::move(labels), breakAddress);
            lastCase = caseTree.get();
            Statements.emplace_back(std::move(caseTree));
        }
    }

    void Switch::Emit(CodeWriter& writer, int indent) const
    {
        writer.Line(indent, "switch (" + Value->Render() + ")", HeaderAddresses);
        EmitBlock(writer, indent, false);
    }

    bool Case::IsTreeEnd()
    {
        if (ReachedAddress(BreakAddress))
            return true;

        if (!Statements.empty() && IsLeafOf<Break, Return, Jump>(Statements.back()))
            return true;

        if (Offset == EndIndex)
        {
            Statements.emplace_back(MakeAst<Attribute>("fallthrough"));
            return true;
        }

        return false;
    }

    bool Case::SuppressBlankBeforeLeaf(const AstPtr& statement) const
    {
        return dynamic_cast<Break*>(statement.get()) != nullptr;
    }

    void Case::Emit(CodeWriter& writer, int indent) const
    {
        for (const auto& label : Labels)
        {
            bool isDefault = dynamic_cast<Default*>(label.get()) != nullptr;
            writer.Line(indent, (isDefault ? "" : "case ") + label->ToString() + ":");
        }

        EmitBody(writer, indent + 1);
    }
}