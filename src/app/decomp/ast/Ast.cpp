#include "Ast.hpp"
#include "decomp/ir/Function.hpp"

namespace scrDbgApp::Decomp
{
    static std::string WrapNode(const AstToken* node, const std::string& text)
    {
        if (node->Address == AstToken::NO_ADDRESS)
            return text;

        char addr[16];
        std::snprintf(addr, sizeof(addr), "%x", node->Address);
        return Marker::NODE_BEGIN + std::string(addr) + Marker::SEPARATOR + text + Marker::END;
    }

    std::string Marker::Link(uint32_t address, const std::string& text)
    {
        char addr[16];
        std::snprintf(addr, sizeof(addr), "%x", address);
        return LINK_BEGIN + std::string(addr) + SEPARATOR + text + END;
    }

    std::string AstToken::Render() const
    {
        return WrapNode(this, ToString());
    }

    std::string AstToken::RenderPointer() const
    {
        return WrapNode(this, ToPointerString());
    }

    std::string JoinTokens(const std::vector<AstPtr>& tokens, const char* separator)
    {
        std::string result;
        for (size_t i = 0; i < tokens.size(); i++)
        {
            if (i)
                result += separator;
            result += tokens[i]->Render();
        }
        return result;
    }

    AstPtr NegateCondition(const AstPtr& condition)
    {
        if (auto notToken = dynamic_cast<LogicalNot*>(condition.get()))
            return notToken->GetValue();

        if (auto compare = dynamic_cast<Compare*>(condition.get()))
            return compare->Invert();

        return MakeAst<LogicalNot>(condition);
    }

    AstPtr Compare::Invert() const
    {
        static constexpr std::pair<const char*, const char*> inverses[] = {{"==", "!="}, {"!=", "=="}, {">", "<="}, {">=", "<"}, {"<", ">="}, {"<=", ">"}};

        for (auto& [op, inverse] : inverses)
        {
            if (m_Op == op)
                return MakeAst<Compare>(m_Lhs, m_Rhs, inverse);
        }

        return MakeAst<LogicalNot>(MakeAst<Compare>(m_Lhs, m_Rhs, m_Op));
    }

    std::string FormatLabel(uint32_t address)
    {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "loc_%X", address);
        return buf;
    }

    // Shortest representation that round-trips, always with a decimal point or exponent so it can't be mistaken for an int
    std::string ConstantFloat::ToString() const
    {
        if (std::isnan(m_Value))
            return "NAN";
        if (std::isinf(m_Value))
            return m_Value < 0 ? "-INFINITY" : "INFINITY";

        char buf[64];
        auto [end, ec] = std::to_chars(buf, buf + sizeof(buf), m_Value);
        std::string str(buf, ec == std::errc() ? end : buf);
        if (str.empty())
            str = "0";

        if (str.find_first_of(".eE") == std::string::npos)
            str += ".0";

        return str + "f";
    }

    std::string LogicalNot::ToString() const
    {
        bool wrap = dynamic_cast<BitwiseAndOr*>(m_Value.get()) || dynamic_cast<Compare*>(m_Value.get()) || dynamic_cast<BinaryArithmetic*>(m_Value.get()) || dynamic_cast<BitwiseXor*>(m_Value.get());
        return wrap ? "!(" + m_Value->Render() + ")" : "!" + m_Value->Render();
    }

    std::string BitwiseAndOr::ToString() const
    {
        bool logical = IsLogical();

        // Mixed && or || chains need grouping to keep the evaluation order readable
        auto wrap = [&](const AstPtr& operand) {
            auto other = dynamic_cast<BitwiseAndOr*>(operand.get());
            bool needsParens = other && (other->m_IsAnd != m_IsAnd || other->IsLogical() != logical);
            return needsParens ? "(" + operand->Render() + ")" : operand->Render();
        };

        const char* op = logical ? (m_IsAnd ? " && " : " || ") : (m_IsAnd ? " & " : " | ");
        return wrap(m_Lhs) + op + wrap(m_Rhs);
    }

    std::string Cast::ToString() const
    {
        bool wrap = dynamic_cast<BinaryArithmetic*>(m_Value.get()) || dynamic_cast<Compare*>(m_Value.get()) || dynamic_cast<BitwiseAndOr*>(m_Value.get()) || dynamic_cast<BitwiseXor*>(m_Value.get()) || dynamic_cast<Negate*>(m_Value.get());
        return std::string("(") + m_Type + ")" + (wrap ? "(" + m_Value->Render() + ")" : m_Value->Render());
    }

    std::string Local::ToString() const
    {
        return "&" + m_Func.GetFrameVarName(m_Index);
    }

    std::string Local::ToPointerString() const
    {
        return m_Func.GetFrameVarName(m_Index);
    }

    std::string LocalLoad::ToString() const
    {
        return m_Func.GetFrameVarName(m_Index);
    }

    std::string LocalStore::ToString() const
    {
        return m_Func.GetFrameVarName(m_Index) + " = " + m_Value->Render() + ";";
    }

    std::string Array::ToPointerString() const
    {
        std::string sep = m_Pointer->IsPointer() ? "" : "->";
        std::string size = m_Size == 1 ? "" : " /*" + std::to_string(m_Size) + "*/";
        return m_Pointer->RenderPointer() + sep + "[" + m_Index->Render() + size + "]";
    }

    std::string Offset::ToPointerString() const
    {
        std::string sep = m_Base->IsPointer() ? "." : "->";
        auto c = dynamic_cast<ConstantInt*>(m_Offset.get());
        if (c && c->GetValue() >= 0)
            return m_Base->RenderPointer() + sep + "f_" + std::to_string(c->GetValue());

        return m_Base->RenderPointer() + sep + "f_[" + m_Offset->Render() + "]";
    }

    std::string FunctionCall::GetName() const
    {
        return Marker::Link(m_Callee.GetStart(), m_Callee.GetName());
    }

    int FunctionCall::GetReturnCount() const
    {
        return static_cast<int>(m_Callee.GetRetCount());
    }

    std::string IndirectCall::GetSuffix() const
    {
        if (m_Targets.empty())
            return {};

        std::string comment = " /* -> ";
        for (size_t i = 0; i < m_Targets.size(); i++)
        {
            if (i)
                comment += ", ";
            const auto& [name, address] = m_Targets[i];
            comment += address != AstToken::NO_ADDRESS ? Marker::Link(address, name) : name;
        }
        return comment + " */";
    }

    std::string StringLiteral::ToString() const
    {
        std::string escaped = "\"";
        escaped.reserve(m_Value.size() + 2);

        for (char c : m_Value)
        {
            switch (c)
            {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20 || c == 0x7F)
                {
                    char hex[8];
                    std::snprintf(hex, sizeof(hex), "\\x%02X", static_cast<unsigned char>(c));
                    escaped += hex;
                }
                else
                {
                    escaped += c;
                }
                break;
            }
        }

        escaped += "\"";

        if (!m_Comment.empty())
        {
            // Keep the comment on one line
            std::string comment;
            comment.reserve(m_Comment.size());
            for (size_t i = 0; i < m_Comment.size(); i++)
            {
                char c = m_Comment[i];
                if (static_cast<unsigned char>(c) < 0x20 || c == 0x7F)
                    c = ' ';
                if (c == '*' && i + 1 < m_Comment.size() && m_Comment[i + 1] == '/')
                {
                    comment += "* /";
                    i++;
                    continue;
                }
                comment += c;
            }

            escaped += " /* " + comment + " */";
        }

        return escaped;
    }

    std::string ConditionalJump::ToString() const
    {
        return "if (" + NegateCondition(m_Condition)->Render() + ") goto " + FormatLabel(m_Target) + ";";
    }

    std::string SwitchJump::ToString() const
    {
        std::string result = "switch (" + m_Value->Render() + ") {";
        for (auto& [value, target] : m_Cases)
            result += " case " + std::to_string(value) + ": goto " + FormatLabel(target) + ";";
        return result + " }";
    }
}