#include "ExpressionEvaluator.hpp"
#include "decomp/ir/Function.hpp"
#include "decomp/ir/ScriptContext.hpp"

namespace scrDbgApp::Decomp
{
    // For "a OP b" VM pushes a then b, so the first pop is the right-hand side.
    void ExpressionEvaluator::PushBinary(const char* symbol)
    {
        AstPtr rhs = m_Stack.Pop();
        AstPtr lhs = m_Stack.Pop();
        m_Stack.Push(MakeAst<BinaryArithmetic>(std::move(lhs), std::move(rhs), symbol));
    }

    void ExpressionEvaluator::PushCompare(const char* op)
    {
        AstPtr rhs = m_Stack.Pop();
        AstPtr lhs = m_Stack.Pop();
        m_Stack.Push(MakeAst<Compare>(std::move(lhs), std::move(rhs), op));
    }

    void ExpressionEvaluator::PushVectorBinary(const char* symbol)
    {
        AstPtr rhs = m_Stack.PopVector();
        AstPtr lhs = m_Stack.PopVector();
        m_Stack.Push(MakeAst<VectorBinary>(std::move(lhs), std::move(rhs), symbol));
    }

    void ExpressionEvaluator::PushCall(std::shared_ptr<FunctionCallBase> call, std::vector<AstPtr>& statements)
    {
        if (call->IsStatement())
            statements.push_back(std::move(call));
        else
            m_Stack.Push(std::move(call));
    }

    int ExpressionEvaluator::PopConstantCount(const char* opName)
    {
        AstPtr count = m_Stack.Pop();
        auto constant = dynamic_cast<ConstantInt*>(count.get());
        if (!constant)
            throw std::runtime_error(std::string(opName) + " with a non-constant count is not supported");
        if (constant->GetValue() < 0 || constant->GetValue() > 0xFFFF)
            throw std::runtime_error(std::string(opName) + " count " + std::to_string(constant->GetValue()) + " is out of range");
        return constant->GetValue();
    }

    AstPtr ExpressionEvaluator::PopCondition(const Instruction& ins)
    {
        m_Stack.SetCurrentAddress(ins.Offset);

        switch (ins.Op)
        {
        case IrOp::IEQ_JZ:
            PushCompare("==");
            break;
        case IrOp::INE_JZ:
            PushCompare("!=");
            break;
        case IrOp::IGT_JZ:
            PushCompare(">");
            break;
        case IrOp::IGE_JZ:
            PushCompare(">=");
            break;
        case IrOp::ILT_JZ:
            PushCompare("<");
            break;
        case IrOp::ILE_JZ:
            PushCompare("<=");
            break;
        default: // JZ tests whatever is on top of the stack
            break;
        }

        return m_Stack.Pop();
    }

    bool ExpressionEvaluator::Evaluate(const Instruction& ins, std::vector<AstPtr>& statements)
    {
        m_Stack.SetCurrentAddress(ins.Offset);

        const size_t firstNew = statements.size();
        bool handled = EvaluateInternal(ins, statements);

        for (size_t i = firstNew; i < statements.size(); i++)
        {
            if (statements[i]->Address == AstToken::NO_ADDRESS)
                statements[i]->Address = ins.Offset;
        }

        return handled;
    }

    bool ExpressionEvaluator::EvaluateInternal(const Instruction& ins, std::vector<AstPtr>& statements)
    {
        switch (ins.Op)
        {
        case IrOp::NOP:
            break;
        case IrOp::IADD:
        case IrOp::FADD:
            PushBinary("+");
            break;
        case IrOp::ISUB:
        case IrOp::FSUB:
            PushBinary("-");
            break;
        case IrOp::IMUL:
        case IrOp::FMUL:
            PushBinary("*");
            break;
        case IrOp::IDIV:
        case IrOp::FDIV:
            PushBinary("/");
            break;
        case IrOp::IMOD:
        case IrOp::FMOD:
            PushBinary("%");
            break;
        case IrOp::INOT:
            m_Stack.Push(MakeAst<LogicalNot>(m_Stack.Pop()));
            break;
        case IrOp::INEG:
        case IrOp::FNEG:
            m_Stack.Push(MakeAst<Negate>(m_Stack.Pop()));
            break;
        case IrOp::IEQ:
        case IrOp::FEQ:
            PushCompare("==");
            break;
        case IrOp::INE:
        case IrOp::FNE:
            PushCompare("!=");
            break;
        case IrOp::IGT:
        case IrOp::FGT:
            PushCompare(">");
            break;
        case IrOp::IGE:
        case IrOp::FGE:
            PushCompare(">=");
            break;
        case IrOp::ILT:
        case IrOp::FLT:
            PushCompare("<");
            break;
        case IrOp::ILE:
        case IrOp::FLE:
            PushCompare("<=");
            break;

        case IrOp::VADD:
            PushVectorBinary("+");
            break;
        case IrOp::VSUB:
            PushVectorBinary("-");
            break;
        case IrOp::VMUL:
            PushVectorBinary("*");
            break;
        case IrOp::VDIV:
            PushVectorBinary("/");
            break;
        case IrOp::VNEG:
            m_Stack.Push(MakeAst<VectorNegate>(m_Stack.PopVector()));
            break;

        case IrOp::IAND:
        case IrOp::IOR:
        {
            AstPtr rhs = m_Stack.Pop();
            AstPtr lhs = m_Stack.Pop();
            m_Stack.Push(MakeAst<BitwiseAndOr>(std::move(lhs), std::move(rhs), ins.Op == IrOp::IAND));
            break;
        }
        case IrOp::IXOR:
        {
            AstPtr rhs = m_Stack.Pop();
            AstPtr lhs = m_Stack.Pop();
            m_Stack.Push(MakeAst<BitwiseXor>(std::move(lhs), std::move(rhs)));
            break;
        }

        case IrOp::I2F:
            m_Stack.Push(MakeAst<Cast>(m_Stack.Pop(), "float"));
            break;
        case IrOp::F2I:
            m_Stack.Push(MakeAst<Cast>(m_Stack.Pop(), "int"));
            break;
        case IrOp::F2V:
        {
            AstPtr value = m_Stack.Pop();
            m_Stack.Push(value->HasSideEffects() ? MakeAst<FloatToVec>(value) : MakeAst<Vector>(value, value, value));
            break;
        }

        case IrOp::PUSH_INT:
            for (int32_t value : ins.Ints)
                m_Stack.Push(MakeAst<ConstantInt>(value));
            break;
        case IrOp::PUSH_FLOAT:
            m_Stack.Push(MakeAst<ConstantFloat>(ins.Float));
            break;

        case IrOp::DUP:
        {
            AstPtr value = m_Stack.Pop();
            if (value->HasSideEffects())
                throw std::runtime_error("Cannot DUP a value with side effects");
            m_Stack.Push(value);
            m_Stack.Push(value);
            break;
        }

        case IrOp::DROP:
        {
            // dropping one slot of a multi-return call just narrows the call.
            if (m_Stack.Peek()->GetStackCount() > 1)
            {
                if (auto call = std::dynamic_pointer_cast<FunctionCallBase>(m_Stack.Peek()))
                {
                    call->DropReturn();
                    break;
                }
            }

            AstPtr dropped = m_Stack.Pop(true);
            if (dropped->HasSideEffects())
                statements.push_back(MakeAst<Drop>(std::move(dropped)));
            break;
        }

        case IrOp::NATIVE:
        {
            auto args = m_Stack.PopCount(ins.ArgCount);
            PushCall(std::make_shared<NativeCall>(std::move(args), m_Context.GetNativeName(static_cast<uint32_t>(ins.Imm)), ins.RetCount), statements);
            break;
        }

        case IrOp::CALL:
        {
            const Function* callee = m_Context.FindCallTarget(ins.Target);
            if (!callee)
            {
                char buf[96];
                std::snprintf(buf, sizeof(buf), "CALL target 0x%X does not match any known function", ins.Target);
                throw std::runtime_error(buf);
            }

            auto args = m_Stack.PopCount(static_cast<int>(callee->GetArgCount()));
            PushCall(std::make_shared<FunctionCall>(std::move(args), *callee), statements);
            break;
        }

        case IrOp::CALLINDIRECT:
        {
            AstPtr location = m_Stack.Pop();

            std::vector<uint32_t> addresses;
            if (auto constant = dynamic_cast<ConstantInt*>(location.get()))
                addresses.push_back(static_cast<uint32_t>(constant->GetValue()));
            else if (auto observed = m_Context.GetIndirectCallTargets(ins.Offset))
                addresses = *observed;

            const Function* signature = nullptr;
            bool consistent = !addresses.empty();
            std::vector<std::pair<std::string, uint32_t>> names;

            for (uint32_t address : addresses)
            {
                const Function* callee = m_Context.FindCallTarget(address);
                if (!callee)
                {
                    char buf[32];
                    std::snprintf(buf, sizeof(buf), "<0x%X>", address);
                    names.emplace_back(buf, AstToken::NO_ADDRESS);
                    consistent = false;
                    continue;
                }

                names.emplace_back(callee->GetName(), callee->GetStart());

                if (!signature)
                    signature = callee;
                else if (callee->GetArgCount() != signature->GetArgCount() || callee->GetRetCount() != signature->GetRetCount())
                    consistent = false;
            }

            // Targets that dont match on the signature (or don't resolve) are still listed, but the call falls back to the unknown-signature form
            std::vector<AstPtr> args;
            int retCount = 0;
            if (consistent && signature)
            {
                args = m_Stack.PopCount(static_cast<int>(signature->GetArgCount()));
                retCount = static_cast<int>(signature->GetRetCount());
            }
            else
            {
                args = m_Stack.PopCount(m_Stack.GetSlotCount());
            }

            if (dynamic_cast<ConstantInt*>(location.get()) && consistent && signature)
            {
                location = MakeAst<Symbol>(signature->GetName(), signature->GetStart());
                names.clear();
            }

            PushCall(std::make_shared<IndirectCall>(std::move(args), std::move(location), retCount, std::move(names)), statements);
            break;
        }

        case IrOp::ENTER:
            throw std::runtime_error("Unexpected ENTER inside a function body");

        case IrOp::LEAVE:
            statements.push_back(MakeAst<Return>(m_Stack.PopCount(ins.RetCount)));
            break;

        case IrOp::LOAD:
            m_Stack.Push(MakeAst<Load>(m_Stack.Pop()));
            break;
        case IrOp::STORE:
        {
            AstPtr pointer = m_Stack.Pop();
            AstPtr value = m_Stack.Pop();
            statements.push_back(MakeAst<Store>(std::move(pointer), std::move(value)));
            break;
        }
        case IrOp::STORE_REV:
        {
            AstPtr value = m_Stack.Pop(); // Stores through the pointer below the value without popping it
            statements.push_back(MakeAst<Store>(m_Stack.Peek(), std::move(value)));
            break;
        }
        case IrOp::LOAD_N:
        {
            AstPtr pointer = m_Stack.Pop();
            int count = PopConstantCount("LOAD_N");
            m_Stack.Push(MakeAst<LoadN>(std::move(pointer), count));
            break;
        }
        case IrOp::STORE_N:
        {
            AstPtr pointer = m_Stack.Pop();
            int count = PopConstantCount("STORE_N");
            statements.push_back(MakeAst<StoreN>(std::move(pointer), m_Stack.PopCount(count)));
            break;
        }

        case IrOp::ARRAY:
        {
            AstPtr pointer = m_Stack.Pop();
            AstPtr index = m_Stack.Pop();
            m_Stack.Push(MakeAst<Array>(static_cast<uint32_t>(ins.Imm), std::move(pointer), std::move(index)));
            break;
        }
        case IrOp::ARRAY_LOAD:
        {
            AstPtr pointer = m_Stack.Pop();
            AstPtr index = m_Stack.Pop();
            m_Stack.Push(MakeAst<ArrayLoad>(static_cast<uint32_t>(ins.Imm), std::move(pointer), std::move(index)));
            break;
        }
        case IrOp::ARRAY_STORE:
        {
            AstPtr pointer = m_Stack.Pop();
            AstPtr index = m_Stack.Pop();
            AstPtr value = m_Stack.Pop();
            statements.push_back(MakeAst<ArrayStore>(static_cast<uint32_t>(ins.Imm), std::move(pointer), std::move(index), std::move(value)));
            break;
        }

        case IrOp::LOCAL:
            m_Stack.Push(MakeAst<Local>(m_Func, static_cast<uint32_t>(ins.Imm)));
            break;
        case IrOp::LOCAL_LOAD:
            m_Stack.Push(MakeAst<LocalLoad>(m_Func, static_cast<uint32_t>(ins.Imm)));
            break;
        case IrOp::LOCAL_STORE:
            statements.push_back(MakeAst<LocalStore>(m_Func, static_cast<uint32_t>(ins.Imm), m_Stack.Pop()));
            break;

        case IrOp::STATIC:
            m_Stack.Push(MakeAst<Variable>("Static_", static_cast<uint32_t>(ins.Imm)));
            break;
        case IrOp::STATIC_LOAD:
            m_Stack.Push(MakeAst<VariableLoad>("Static_", static_cast<uint32_t>(ins.Imm)));
            break;
        case IrOp::STATIC_STORE:
            statements.push_back(MakeAst<VariableStore>("Static_", static_cast<uint32_t>(ins.Imm), m_Stack.Pop()));
            break;

        case IrOp::GLOBAL:
            m_Stack.Push(MakeAst<Variable>("Global_", static_cast<uint32_t>(ins.Imm)));
            break;
        case IrOp::GLOBAL_LOAD:
            m_Stack.Push(MakeAst<VariableLoad>("Global_", static_cast<uint32_t>(ins.Imm)));
            break;
        case IrOp::GLOBAL_STORE:
            statements.push_back(MakeAst<VariableStore>("Global_", static_cast<uint32_t>(ins.Imm), m_Stack.Pop()));
            break;

        case IrOp::IADD_IMM:
            m_Stack.Push(MakeAst<BinaryArithmetic>(m_Stack.Pop(), MakeAst<ConstantInt>(static_cast<int32_t>(ins.Imm)), "+"));
            break;
        case IrOp::IMUL_IMM:
            m_Stack.Push(MakeAst<BinaryArithmetic>(m_Stack.Pop(), MakeAst<ConstantInt>(static_cast<int32_t>(ins.Imm)), "*"));
            break;

        case IrOp::IOFFSET:
        {
            AstPtr offset = m_Stack.Pop();
            AstPtr base = m_Stack.Pop();
            m_Stack.Push(MakeAst<Offset>(std::move(base), std::move(offset)));
            break;
        }
        case IrOp::IOFFSET_IMM:
            m_Stack.Push(MakeAst<Offset>(m_Stack.Pop(), MakeAst<ConstantInt>(static_cast<int32_t>(ins.Imm))));
            break;
        case IrOp::IOFFSET_IMM_LOAD:
            m_Stack.Push(MakeAst<OffsetLoad>(m_Stack.Pop(), static_cast<int32_t>(ins.Imm)));
            break;
        case IrOp::IOFFSET_IMM_STORE:
        {
            AstPtr base = m_Stack.Pop();
            AstPtr value = m_Stack.Pop();
            statements.push_back(MakeAst<OffsetStore>(std::move(base), static_cast<int32_t>(ins.Imm), std::move(value)));
            break;
        }

        case IrOp::STRING:
        {
            AstPtr index = m_Stack.Pop();
            if (auto constant = dynamic_cast<ConstantInt*>(index.get()))
            {
                std::string value = m_Context.GetString(static_cast<uint32_t>(constant->GetValue()));
                std::string comment = m_Context.GetStringComment(value);
                m_Stack.Push(MakeAst<StringLiteral>(std::move(value), std::move(comment)));
            }
            else
            {
                m_Stack.Push(MakeAst<StringTableIndex>(std::move(index)));
            }
            break;
        }
        case IrOp::STRINGHASH:
            m_Stack.Push(MakeAst<StringHash>(m_Stack.Pop()));
            break;

        case IrOp::TEXT_LABEL_ASSIGN_STRING:
        case IrOp::TEXT_LABEL_ASSIGN_INT:
        case IrOp::TEXT_LABEL_APPEND_STRING:
        case IrOp::TEXT_LABEL_APPEND_INT:
        {
            AstPtr dst = m_Stack.Pop();
            AstPtr value = m_Stack.Pop();
            statements.push_back(MakeAst<TextLabelOp>(GetIrOpName(ins.Op), std::move(dst), std::move(value), static_cast<int>(ins.Imm)));
            break;
        }
        case IrOp::TEXT_LABEL_COPY:
        {
            AstPtr dst = m_Stack.Pop();
            AstPtr dstSize = m_Stack.Pop();
            int count = PopConstantCount("TEXT_LABEL_COPY");
            statements.push_back(MakeAst<TextLabelCopy>(std::move(dst), m_Stack.PopCount(count), std::move(dstSize)));
            break;
        }

        case IrOp::IS_BIT_SET:
        {
            AstPtr bit = m_Stack.Pop();
            AstPtr value = m_Stack.Pop();
            m_Stack.Push(MakeAst<IsBitSet>(std::move(value), std::move(bit)));
            break;
        }

        case IrOp::CATCH:
        case IrOp::THROW:
            throw std::runtime_error("CATCH/THROW are not supported"); // these aren't actually for exception handling

        case IrOp::J:
        case IrOp::JZ:
        case IrOp::IEQ_JZ:
        case IrOp::INE_JZ:
        case IrOp::IGT_JZ:
        case IrOp::IGE_JZ:
        case IrOp::ILT_JZ:
        case IrOp::ILE_JZ:
        case IrOp::SWITCH:
            return false;

        case IrOp::UNKNOWN:
        default:
        {
            char buf[96];
            std::snprintf(buf, sizeof(buf), "Unknown opcode 0x%02X at 0x%X", ins.RawOpcode, ins.Offset);
            throw std::runtime_error(buf);
        }
        }

        return true;
    }
}