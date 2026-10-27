#pragma once
#include "Ast.hpp"

namespace scrDbgApp::Decomp
{
    class ScriptStack
    {
    public:
        // Nodes pushed from now on that don't have an address yet get address. Re-pushed nodes (DUP) keep theirs.
        void SetCurrentAddress(uint32_t address)
        {
            m_CurrentAddress = address;
        }

        void Push(AstPtr token)
        {
            if (token->Address == AstToken::NO_ADDRESS)
                token->Address = m_CurrentAddress;
            m_Stack.push_back(std::move(token));
        }

        // allowMulti when false rejects popping a multi-slot node where a scalar is expected
        AstPtr Pop(bool allowMulti = false)
        {
            if (m_Stack.empty())
                return MakeAst<Unknown>();

            AstPtr value = std::move(m_Stack.back());
            m_Stack.pop_back();

            if (!allowMulti && value->GetStackCount() != 1)
                throw std::runtime_error("Popped a multi-slot value where a single value was expected");

            return value;
        }

        AstPtr Peek() const
        {
            return m_Stack.empty() ? MakeAst<Unknown>() : m_Stack.back();
        }

        // 0 = top of stack
        AstPtr PeekAt(size_t index) const
        {
            return m_Stack.at(m_Stack.size() - 1 - index);
        }

        AstPtr PopVector()
        {
            if (Peek()->GetStackCount() == 3)
                return Pop(true);

            if (m_Stack.size() >= 3 && PeekAt(0)->GetStackCount() == 1 && PeekAt(1)->GetStackCount() == 1 && PeekAt(2)->GetStackCount() == 1)
            {
                AstPtr z = Pop();
                AstPtr y = Pop();
                AstPtr x = Pop();
                return MakeAst<Vector>(std::move(x), std::move(y), std::move(z));
            }

            return MakeAst<Vector>(MakeAst<Unknown>(), MakeAst<Unknown>(), MakeAst<Unknown>());
        }

        std::vector<AstPtr> PopCount(int count)
        {
            std::vector<AstPtr> values;

            int popped = 0;
            while (popped < count)
            {
                AstPtr value = Pop(true);
                popped += std::max(1, value->GetStackCount());
                values.push_back(std::move(value));
            }

            std::reverse(values.begin(), values.end());
            return values;
        }

        int GetSlotCount() const
        {
            int count = 0;
            for (const auto& value : m_Stack)
                count += value->GetStackCount();
            return count;
        }

    private:
        std::vector<AstPtr> m_Stack;
        uint32_t m_CurrentAddress = AstToken::NO_ADDRESS;
    };
}