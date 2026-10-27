#pragma once

namespace scrDbgApp::Decomp
{
    class Function;

    namespace Marker
    {
        constexpr char NODE_BEGIN = '\x01';
        constexpr char SEPARATOR = '\x02';
        constexpr char END = '\x03';
        constexpr char LINK_BEGIN = '\x04';

        std::string Link(uint32_t address, const std::string& text); // Text naming a function at address that the UI can navigate to
    }

    class AstToken
    {
    public:
        static constexpr uint32_t NO_ADDRESS = 0xFFFFFFFF;

        virtual ~AstToken() = default;

        std::string Render() const;
        std::string RenderPointer() const;

        virtual bool IsStatement() const
        {
            return false;
        }

        virtual bool HasSideEffects() const
        {
            return false;
        }

        // number of stack slots this value occupies (3 for vectors, N for multi return calls and LOAD_N)
        virtual int GetStackCount() const
        {
            return 1;
        }

        virtual bool IsPointer() const
        {
            return false;
        }

        virtual std::string ToString() const = 0;

        // rendering of an address node when it's being dereferenced/indexed (like Static_5 instead of &Static_5)
        virtual std::string ToPointerString() const
        {
            return ToString();
        }

        uint32_t Address = NO_ADDRESS; // Address of the instruction this node was built from
    };

    using AstPtr = std::shared_ptr<AstToken>;

    template <typename T, typename... Args>
    AstPtr MakeAst(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    std::string JoinTokens(const std::vector<AstPtr>& tokens, const char* separator = ", ");

    AstPtr NegateCondition(const AstPtr& condition);

    class Unknown : public AstToken
    {
    public:
        std::string ToString() const override
        {
            return "<stack underflow>";
        }
    };

    class ConstantInt : public AstToken
    {
    public:
        explicit ConstantInt(int32_t value)
            : m_Value(value)
        {
        }

        int32_t GetValue() const
        {
            return m_Value;
        }

        std::string ToString() const override
        {
            return std::to_string(m_Value);
        }

    private:
        int32_t m_Value;
    };

    class ConstantFloat : public AstToken
    {
    public:
        explicit ConstantFloat(float value)
            : m_Value(value)
        {
        }

        std::string ToString() const override;

    private:
        float m_Value;
    };

    // IADD/ISUB/IMUL/IDIV/IMOD and the float equivalents. A sub-operand is
    // parenthesized only if it's another arithmetic node with a different operator.
    class BinaryArithmetic : public AstToken
    {
    public:
        BinaryArithmetic(AstPtr lhs, AstPtr rhs, std::string symbol)
            : m_Lhs(std::move(lhs)),
              m_Rhs(std::move(rhs)),
              m_Symbol(std::move(symbol))
        {
        }

        bool HasSideEffects() const override
        {
            return m_Lhs->HasSideEffects() || m_Rhs->HasSideEffects();
        }

        std::string ToString() const override
        {
            return Wrap(m_Lhs) + " " + m_Symbol + " " + Wrap(m_Rhs);
        }

    private:
        std::string Wrap(const AstPtr& operand) const
        {
            auto bin = dynamic_cast<BinaryArithmetic*>(operand.get());
            return (bin && bin->m_Symbol != m_Symbol) ? "(" + operand->Render() + ")" : operand->Render();
        }

        AstPtr m_Lhs;
        AstPtr m_Rhs;
        std::string m_Symbol;
    };

    // VADD/VSUB/VMUL/VDIV
    class VectorBinary : public AstToken
    {
    public:
        VectorBinary(AstPtr lhs, AstPtr rhs, std::string symbol)
            : m_Lhs(std::move(lhs)),
              m_Rhs(std::move(rhs)),
              m_Symbol(std::move(symbol))
        {
        }

        int GetStackCount() const override
        {
            return 3;
        }

        bool HasSideEffects() const override
        {
            return m_Lhs->HasSideEffects() || m_Rhs->HasSideEffects();
        }

        std::string ToString() const override
        {
            return Wrap(m_Lhs) + " " + m_Symbol + " " + Wrap(m_Rhs);
        }

    private:
        std::string Wrap(const AstPtr& operand) const
        {
            auto bin = dynamic_cast<VectorBinary*>(operand.get());
            return (bin && bin->m_Symbol != m_Symbol) ? "(" + operand->Render() + ")" : operand->Render();
        }

        AstPtr m_Lhs;
        AstPtr m_Rhs;
        std::string m_Symbol;
    };

    // INEG/FNEG
    class Negate : public AstToken
    {
    public:
        explicit Negate(AstPtr value)
            : m_Value(std::move(value))
        {
        }

        bool HasSideEffects() const override
        {
            return m_Value->HasSideEffects();
        }

        std::string ToString() const override
        {
            bool wrap = dynamic_cast<BinaryArithmetic*>(m_Value.get()) != nullptr;
            return wrap ? "-(" + m_Value->Render() + ")" : "-" + m_Value->Render();
        }

    private:
        AstPtr m_Value;
    };

    class VectorNegate : public AstToken
    {
    public:
        explicit VectorNegate(AstPtr value)
            : m_Value(std::move(value))
        {
        }

        int GetStackCount() const override
        {
            return 3;
        }

        bool HasSideEffects() const override
        {
            return m_Value->HasSideEffects();
        }

        std::string ToString() const override
        {
            bool wrap = dynamic_cast<VectorBinary*>(m_Value.get()) != nullptr;
            return wrap ? "-(" + m_Value->Render() + ")" : "-" + m_Value->Render();
        }

    private:
        AstPtr m_Value;
    };

    // INOT
    class LogicalNot : public AstToken
    {
    public:
        explicit LogicalNot(AstPtr value)
            : m_Value(std::move(value))
        {
        }

        bool HasSideEffects() const override
        {
            return m_Value->HasSideEffects();
        }

        std::string ToString() const override;

        const AstPtr& GetValue() const
        {
            return m_Value;
        }

    private:
        AstPtr m_Value;
    };

    // IEQ/INE/IGT/IGE/ILT/ILE and the float equivalents
    class Compare : public AstToken
    {
    public:
        Compare(AstPtr lhs, AstPtr rhs, std::string op)
            : m_Lhs(std::move(lhs)),
              m_Rhs(std::move(rhs)),
              m_Op(std::move(op))
        {
        }

        bool HasSideEffects() const override
        {
            return m_Lhs->HasSideEffects() || m_Rhs->HasSideEffects();
        }

        std::string ToString() const override
        {
            return m_Lhs->Render() + " " + m_Op + " " + m_Rhs->Render();
        }

        // Same operands with the opposite operator (== <-> !=, < <-> >=, etc)
        AstPtr Invert() const;

    private:
        AstPtr m_Lhs;
        AstPtr m_Rhs;
        std::string m_Op;
    };

    // IAND/IOR - rendered as && or || when neither operand is a constant, otherwise as bitwise & or |.
    class BitwiseAndOr : public AstToken
    {
    public:
        BitwiseAndOr(AstPtr lhs, AstPtr rhs, bool isAnd)
            : m_Lhs(std::move(lhs)),
              m_Rhs(std::move(rhs)),
              m_IsAnd(isAnd)
        {
        }

        bool HasSideEffects() const override
        {
            return m_Lhs->HasSideEffects() || m_Rhs->HasSideEffects();
        }

        bool IsLogical() const
        {
            return dynamic_cast<ConstantInt*>(m_Lhs.get()) == nullptr && dynamic_cast<ConstantInt*>(m_Rhs.get()) == nullptr;
        }

        bool IsAnd() const
        {
            return m_IsAnd;
        }

        std::string ToString() const override;

    private:
        AstPtr m_Lhs;
        AstPtr m_Rhs;
        bool m_IsAnd;
    };

    class BitwiseXor : public AstToken
    {
    public:
        BitwiseXor(AstPtr lhs, AstPtr rhs)
            : m_Lhs(std::move(lhs)),
              m_Rhs(std::move(rhs))
        {
        }

        bool HasSideEffects() const override
        {
            return m_Lhs->HasSideEffects() || m_Rhs->HasSideEffects();
        }

        std::string ToString() const override
        {
            return m_Lhs->Render() + " ^ " + m_Rhs->Render();
        }

    private:
        AstPtr m_Lhs;
        AstPtr m_Rhs;
    };

    // I2F / F2I
    class Cast : public AstToken
    {
    public:
        Cast(AstPtr value, const char* type)
            : m_Value(std::move(value)),
              m_Type(type)
        {
        }

        bool HasSideEffects() const override
        {
            return m_Value->HasSideEffects();
        }

        std::string ToString() const override;

    private:
        AstPtr m_Value;
        const char* m_Type;
    };

    // F2V on a value with side effects (otherwise it becomes { x, x, x }).
    class FloatToVec : public AstToken
    {
    public:
        explicit FloatToVec(AstPtr value)
            : m_Value(std::move(value))
        {
        }

        int GetStackCount() const override
        {
            return 3;
        }

        bool HasSideEffects() const override
        {
            return m_Value->HasSideEffects();
        }

        std::string ToString() const override
        {
            return "F2V(" + m_Value->Render() + ")";
        }

    private:
        AstPtr m_Value;
    };

    class Vector : public AstToken
    {
    public:
        Vector(AstPtr x, AstPtr y, AstPtr z)
            : m_X(std::move(x)),
              m_Y(std::move(y)),
              m_Z(std::move(z))
        {
        }

        int GetStackCount() const override
        {
            return 3;
        }

        bool HasSideEffects() const override
        {
            return m_X->HasSideEffects() || m_Y->HasSideEffects() || m_Z->HasSideEffects();
        }

        std::string ToString() const override
        {
            return "{ " + m_X->Render() + ", " + m_Y->Render() + ", " + m_Z->Render() + " }";
        }

    private:
        AstPtr m_X;
        AstPtr m_Y;
        AstPtr m_Z;
    };

    // A plain name, for example a function referenced by a constant pointer
    class Symbol : public AstToken
    {
    public:
        explicit Symbol(std::string name, uint32_t linkTarget = NO_ADDRESS)
            : m_Name(std::move(name)),
              m_LinkTarget(linkTarget)
        {
        }

        std::string ToString() const override
        {
            return m_LinkTarget != NO_ADDRESS ? Marker::Link(m_LinkTarget, m_Name) : m_Name;
        }

    private:
        std::string m_Name;
        uint32_t m_LinkTarget;
    };

    class Local : public AstToken
    {
    public:
        Local(const Function& func, uint32_t index)
            : m_Func(func),
              m_Index(index)
        {
        }

        bool IsPointer() const override
        {
            return true;
        }

        std::string ToString() const override;
        std::string ToPointerString() const override;

    private:
        const Function& m_Func;
        uint32_t m_Index;
    };

    class LocalLoad : public AstToken
    {
    public:
        LocalLoad(const Function& func, uint32_t index)
            : m_Func(func),
              m_Index(index)
        {
        }

        std::string ToString() const override;

    private:
        const Function& m_Func;
        uint32_t m_Index;
    };

    class LocalStore : public AstToken
    {
    public:
        LocalStore(const Function& func, uint32_t index, AstPtr value)
            : m_Func(func),
              m_Index(index),
              m_Value(std::move(value))
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override;

    private:
        const Function& m_Func;
        uint32_t m_Index;
        AstPtr m_Value;
    };

    class Variable : public AstToken
    {
    public:
        Variable(const char* prefix, uint32_t index)
            : m_Prefix(prefix),
              m_Index(index)
        {
        }

        bool IsPointer() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return "&" + ToPointerString();
        }

        std::string ToPointerString() const override
        {
            return m_Prefix + std::to_string(m_Index);
        }

    private:
        std::string m_Prefix;
        uint32_t m_Index;
    };

    class VariableLoad : public AstToken
    {
    public:
        VariableLoad(const char* prefix, uint32_t index)
            : m_Prefix(prefix),
              m_Index(index)
        {
        }

        std::string ToString() const override
        {
            return m_Prefix + std::to_string(m_Index);
        }

    private:
        std::string m_Prefix;
        uint32_t m_Index;
    };

    class VariableStore : public AstToken
    {
    public:
        VariableStore(const char* prefix, uint32_t index, AstPtr value)
            : m_Prefix(prefix),
              m_Index(index),
              m_Value(std::move(value))
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return m_Prefix + std::to_string(m_Index) + " = " + m_Value->Render() + ";";
        }

    private:
        std::string m_Prefix;
        uint32_t m_Index;
        AstPtr m_Value;
    };

    class Load : public AstToken
    {
    public:
        explicit Load(AstPtr pointer)
            : m_Pointer(std::move(pointer))
        {
        }

        bool HasSideEffects() const override
        {
            return m_Pointer->HasSideEffects();
        }

        std::string ToString() const override
        {
            return m_Pointer->IsPointer() ? m_Pointer->RenderPointer() : "*" + m_Pointer->Render();
        }

    private:
        AstPtr m_Pointer;
    };

    class Store : public AstToken
    {
    public:
        Store(AstPtr pointer, AstPtr value)
            : m_Pointer(std::move(pointer)),
              m_Value(std::move(value))
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            std::string lhs = m_Pointer->IsPointer() ? m_Pointer->RenderPointer() : "*" + m_Pointer->Render();
            return lhs + " = " + m_Value->Render() + ";";
        }

    private:
        AstPtr m_Pointer;
        AstPtr m_Value;
    };

    // The slot count must be a constant.
    class LoadN : public AstToken
    {
    public:
        LoadN(AstPtr pointer, int count)
            : m_Pointer(std::move(pointer)),
              m_Count(count)
        {
        }

        int GetStackCount() const override
        {
            return m_Count;
        }

        bool HasSideEffects() const override
        {
            return m_Pointer->HasSideEffects();
        }

        std::string ToString() const override
        {
            return m_Pointer->IsPointer() ? m_Pointer->RenderPointer() : "*" + m_Pointer->Render();
        }

    private:
        AstPtr m_Pointer;
        int m_Count;
    };

    class StoreN : public AstToken
    {
    public:
        StoreN(AstPtr pointer, std::vector<AstPtr> values)
            : m_Pointer(std::move(pointer)),
              m_Values(std::move(values))
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            std::string lhs = m_Pointer->IsPointer() ? m_Pointer->RenderPointer() : "*" + m_Pointer->Render();

            // One node covering every slot (a multi-return call, a vector, LOAD_N) is assigned as-is
            if (m_Values.size() == 1)
                return lhs + " = " + m_Values[0]->Render() + ";";

            return lhs + " = { " + JoinTokens(m_Values) + " };";
        }

    private:
        AstPtr m_Pointer;
        std::vector<AstPtr> m_Values;
    };

    class Array : public AstToken
    {
    public:
        Array(uint32_t size, AstPtr pointer, AstPtr index)
            : m_Size(size),
              m_Pointer(std::move(pointer)),
              m_Index(std::move(index))
        {
        }

        bool IsPointer() const override
        {
            return true;
        }

        bool HasSideEffects() const override
        {
            return m_Pointer->HasSideEffects() || m_Index->HasSideEffects();
        }

        std::string ToString() const override
        {
            return "&" + ToPointerString();
        }

        std::string ToPointerString() const override;

    private:
        uint32_t m_Size;
        AstPtr m_Pointer;
        AstPtr m_Index;
    };

    class ArrayLoad : public AstToken
    {
    public:
        ArrayLoad(uint32_t size, AstPtr pointer, AstPtr index)
            : m_Element(size, std::move(pointer), std::move(index))
        {
        }

        bool HasSideEffects() const override
        {
            return m_Element.HasSideEffects();
        }

        std::string ToString() const override
        {
            return m_Element.ToPointerString();
        }

    private:
        Array m_Element;
    };

    class ArrayStore : public AstToken
    {
    public:
        ArrayStore(uint32_t size, AstPtr pointer, AstPtr index, AstPtr value)
            : m_Element(size, std::move(pointer), std::move(index)),
              m_Value(std::move(value))
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return m_Element.ToPointerString() + " = " + m_Value->Render() + ";";
        }

    private:
        Array m_Element;
        AstPtr m_Value;
    };

    class Offset : public AstToken
    {
    public:
        Offset(AstPtr base, AstPtr offset)
            : m_Base(std::move(base)),
              m_Offset(std::move(offset))
        {
        }

        bool IsPointer() const override
        {
            return true;
        }

        bool HasSideEffects() const override
        {
            return m_Base->HasSideEffects() || m_Offset->HasSideEffects();
        }

        std::string ToString() const override
        {
            return "&(" + ToPointerString() + ")";
        }

        std::string ToPointerString() const override;

    private:
        AstPtr m_Base;
        AstPtr m_Offset;
    };

    class OffsetLoad : public AstToken
    {
    public:
        OffsetLoad(AstPtr base, int32_t offset)
            : m_Field(std::move(base), MakeAst<ConstantInt>(offset))
        {
        }

        bool HasSideEffects() const override
        {
            return m_Field.HasSideEffects();
        }

        std::string ToString() const override
        {
            return m_Field.ToPointerString();
        }

    private:
        Offset m_Field;
    };

    class OffsetStore : public AstToken
    {
    public:
        OffsetStore(AstPtr base, int32_t offset, AstPtr value)
            : m_Field(std::move(base), MakeAst<ConstantInt>(offset)),
              m_Value(std::move(value))
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return m_Field.ToPointerString() + " = " + m_Value->Render() + ";";
        }

    private:
        Offset m_Field;
        AstPtr m_Value;
    };

    class Drop : public AstToken
    {
    public:
        explicit Drop(AstPtr value)
            : m_Value(std::move(value))
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return m_Value->Render() + ";";
        }

    private:
        AstPtr m_Value;
    };

    class Break : public AstToken
    {
    public:
        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return "break;";
        }
    };

    class Default : public AstToken
    {
    public:
        std::string ToString() const override
        {
            return "default";
        }
    };

    // annnotation that isnt a real statement, such as [[fallthrough]]
    class Attribute : public AstToken
    {
    public:
        explicit Attribute(std::string value)
            : m_Value(std::move(value))
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return "[[" + m_Value + "]];";
        }

    private:
        std::string m_Value;
    };

    class Return : public AstToken
    {
    public:
        explicit Return(std::vector<AstPtr> values)
            : m_Values(std::move(values))
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return m_Values.empty() ? "return;" : "return " + JoinTokens(m_Values) + ";";
        }

    private:
        std::vector<AstPtr> m_Values;
    };

    class FunctionCallBase : public AstToken
    {
    public:
        explicit FunctionCallBase(std::vector<AstPtr> arguments)
            : m_Arguments(std::move(arguments))
        {
        }

        virtual std::string GetName() const = 0;
        virtual int GetReturnCount() const = 0;

        bool IsStatement() const override
        {
            return GetStackCount() == 0;
        }

        int GetStackCount() const override
        {
            return GetReturnCount() - m_DroppedReturns;
        }

        bool HasSideEffects() const override
        {
            return true;
        }

        // DROP of one slot of a multi-return call
        void DropReturn()
        {
            m_DroppedReturns++;
        }

        std::string ToString() const override
        {
            return GetName() + "(" + JoinTokens(m_Arguments) + ")" + GetSuffix() + (IsStatement() ? ";" : "");
        }

    protected:
        // rendered between the closing parenthesis and the statement's ';'.
        virtual std::string GetSuffix() const
        {
            return {};
        }

    private:
        std::vector<AstPtr> m_Arguments;
        int m_DroppedReturns = 0;
    };

    class NativeCall : public FunctionCallBase
    {
    public:
        NativeCall(std::vector<AstPtr> arguments, std::string name, int returnCount)
            : FunctionCallBase(std::move(arguments)),
              m_Name(std::move(name)),
              m_ReturnCount(returnCount)
        {
        }

        std::string GetName() const override
        {
            return m_Name;
        }

        int GetReturnCount() const override
        {
            return m_ReturnCount;
        }

    private:
        std::string m_Name;
        int m_ReturnCount;
    };

    class FunctionCall : public FunctionCallBase
    {
    public:
        FunctionCall(std::vector<AstPtr> arguments, const Function& callee)
            : FunctionCallBase(std::move(arguments)),
              m_Callee(callee)
        {
        }

        std::string GetName() const override;
        int GetReturnCount() const override;

    private:
        const Function& m_Callee;
    };

    class IndirectCall : public FunctionCallBase
    {
    public:
        // targets: display name and when it resolved to a function, its start address
        IndirectCall(std::vector<AstPtr> arguments, AstPtr location, int returnCount, std::vector<std::pair<std::string, uint32_t>> targets)
            : FunctionCallBase(std::move(arguments)),
              m_Location(std::move(location)),
              m_ReturnCount(returnCount),
              m_Targets(std::move(targets))
        {
        }

        std::string GetName() const override
        {
            return "(*" + m_Location->Render() + ")";
        }

        int GetReturnCount() const override
        {
            return m_ReturnCount;
        }

    protected:
        std::string GetSuffix() const override;

    private:
        AstPtr m_Location;
        int m_ReturnCount;
        std::vector<std::pair<std::string, uint32_t>> m_Targets;
    };

    class StringLiteral : public AstToken
    {
    public:
        StringLiteral(std::string value, std::string comment)
            : m_Value(std::move(value)),
              m_Comment(std::move(comment))
        {
        }

        std::string ToString() const override;

    private:
        std::string m_Value;
        std::string m_Comment;
    };

    // STRING with a non-constant index (shouldn't happen normally)
    class StringTableIndex : public AstToken
    {
    public:
        explicit StringTableIndex(AstPtr index)
            : m_Index(std::move(index))
        {
        }

        std::string ToString() const override
        {
            return "STRING(" + m_Index->Render() + ")";
        }

    private:
        AstPtr m_Index;
    };

    // is this even used by any script?
    class StringHash : public AstToken
    {
    public:
        explicit StringHash(AstPtr str)
            : m_String(std::move(str))
        {
        }

        bool HasSideEffects() const override
        {
            return m_String->HasSideEffects();
        }

        std::string ToString() const override
        {
            return "joaat(" + m_String->Render() + ")";
        }

    private:
        AstPtr m_String;
    };

    class IsBitSet : public AstToken
    {
    public:
        IsBitSet(AstPtr value, AstPtr bit)
            : m_Value(std::move(value)),
              m_Bit(std::move(bit))
        {
        }

        bool HasSideEffects() const override
        {
            return m_Value->HasSideEffects() || m_Bit->HasSideEffects();
        }

        std::string ToString() const override
        {
            return "IS_BIT_SET(" + m_Value->Render() + ", " + m_Bit->Render() + ")";
        }

    private:
        AstPtr m_Value;
        AstPtr m_Bit;
    };

    // TEXT_LABEL_ASSIGN/APPEND_STRING/INT
    class TextLabelOp : public AstToken
    {
    public:
        TextLabelOp(const char* name, AstPtr dst, AstPtr value, int size)
            : m_Name(name),
              m_Dst(std::move(dst)),
              m_Value(std::move(value)),
              m_Size(size)
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return std::string(m_Name) + "(" + m_Dst->Render() + ", " + m_Value->Render() + ", " + std::to_string(m_Size) + ");";
        }

    private:
        const char* m_Name;
        AstPtr m_Dst;
        AstPtr m_Value;
        int m_Size;
    };

    class TextLabelCopy : public AstToken
    {
    public:
        TextLabelCopy(AstPtr dst, std::vector<AstPtr> values, AstPtr count)
            : m_Dst(std::move(dst)),
              m_Values(std::move(values)),
              m_Count(std::move(count))
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return "TEXT_LABEL_COPY(" + m_Dst->Render() + ", { " + JoinTokens(m_Values) + " }, " + m_Count->Render() + ");";
        }

    private:
        AstPtr m_Dst;
        std::vector<AstPtr> m_Values;
        AstPtr m_Count;
    };

    std::string FormatLabel(uint32_t address);

    class Jump : public AstToken
    {
    public:
        explicit Jump(uint32_t target)
            : m_Target(target)
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return "goto " + FormatLabel(m_Target) + ";";
        }

    private:
        uint32_t m_Target;
    };

    class ConditionalJump : public AstToken
    {
    public:
        ConditionalJump(AstPtr condition, uint32_t target)
            : m_Condition(std::move(condition)),
              m_Target(target)
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        // JZ branches when the condition is false.
        std::string ToString() const override;

    private:
        AstPtr m_Condition;
        uint32_t m_Target;
    };

    class SwitchJump : public AstToken
    {
    public:
        SwitchJump(AstPtr value, std::vector<std::pair<int32_t, uint32_t>> cases)
            : m_Value(std::move(value)),
              m_Cases(std::move(cases))
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override;

    private:
        AstPtr m_Value;
        std::vector<std::pair<int32_t, uint32_t>> m_Cases;
    };

    // "loc_*:" line emited by the flat builder before jump targets.
    class Label : public AstToken
    {
    public:
        explicit Label(uint32_t address)
            : m_Address(address)
        {
        }

        bool IsStatement() const override
        {
            return true;
        }

        std::string ToString() const override
        {
            return FormatLabel(m_Address) + ":";
        }

    private:
        uint32_t m_Address;
    };
}