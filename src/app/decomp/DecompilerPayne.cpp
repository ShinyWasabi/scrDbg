#include "DecompilerPayne.hpp"
#include "disasm/opcodes/OpcodesPayne.hpp"
#include "ir/Function.hpp"

namespace scrDbgApp
{
    using namespace Decomp;

    static std::optional<IrOp> MapSimple(OpcodesPayne op) // ops whose MP3 encoding maps 1:1 onto an operand less IR op
    {
        switch (op)
        {
        case OpcodesPayne::NOP:
            return IrOp::NOP;
        case OpcodesPayne::IADD:
            return IrOp::IADD;
        case OpcodesPayne::ISUB:
            return IrOp::ISUB;
        case OpcodesPayne::IMUL:
            return IrOp::IMUL;
        case OpcodesPayne::IDIV:
            return IrOp::IDIV;
        case OpcodesPayne::IMOD:
            return IrOp::IMOD;
        case OpcodesPayne::INOT:
            return IrOp::INOT;
        case OpcodesPayne::INEG:
            return IrOp::INEG;
        case OpcodesPayne::IEQ:
            return IrOp::IEQ;
        case OpcodesPayne::INE:
            return IrOp::INE;
        case OpcodesPayne::IGT:
            return IrOp::IGT;
        case OpcodesPayne::IGE:
            return IrOp::IGE;
        case OpcodesPayne::ILT:
            return IrOp::ILT;
        case OpcodesPayne::ILE:
            return IrOp::ILE;
        case OpcodesPayne::FADD:
            return IrOp::FADD;
        case OpcodesPayne::FSUB:
            return IrOp::FSUB;
        case OpcodesPayne::FMUL:
            return IrOp::FMUL;
        case OpcodesPayne::FDIV:
            return IrOp::FDIV;
        case OpcodesPayne::FMOD:
            return IrOp::FMOD;
        case OpcodesPayne::FNEG:
            return IrOp::FNEG;
        case OpcodesPayne::FEQ:
            return IrOp::FEQ;
        case OpcodesPayne::FNE:
            return IrOp::FNE;
        case OpcodesPayne::FGT:
            return IrOp::FGT;
        case OpcodesPayne::FGE:
            return IrOp::FGE;
        case OpcodesPayne::FLT:
            return IrOp::FLT;
        case OpcodesPayne::FLE:
            return IrOp::FLE;
        case OpcodesPayne::VADD:
            return IrOp::VADD;
        case OpcodesPayne::VSUB:
            return IrOp::VSUB;
        case OpcodesPayne::VMUL:
            return IrOp::VMUL;
        case OpcodesPayne::VDIV:
            return IrOp::VDIV;
        case OpcodesPayne::VNEG:
            return IrOp::VNEG;
        case OpcodesPayne::IAND:
            return IrOp::IAND;
        case OpcodesPayne::IOR:
            return IrOp::IOR;
        case OpcodesPayne::IXOR:
            return IrOp::IXOR;
        case OpcodesPayne::I2F:
            return IrOp::I2F;
        case OpcodesPayne::F2I:
            return IrOp::F2I;
        case OpcodesPayne::F2V:
            return IrOp::F2V;
        case OpcodesPayne::DUP:
            return IrOp::DUP;
        case OpcodesPayne::DROP:
            return IrOp::DROP;
        case OpcodesPayne::LOAD:
            return IrOp::LOAD;
        case OpcodesPayne::STORE:
            return IrOp::STORE;
        case OpcodesPayne::STORE_REV:
            return IrOp::STORE_REV;
        case OpcodesPayne::TEXT_LABEL_COPY:
            return IrOp::TEXT_LABEL_COPY;
        case OpcodesPayne::CATCH:
            return IrOp::CATCH;
        case OpcodesPayne::THROW:
            return IrOp::THROW;
        case OpcodesPayne::LOAD_N:
            return IrOp::LOAD_N;
        case OpcodesPayne::STORE_N:
            return IrOp::STORE_N;
        case OpcodesPayne::CALLINDIRECT:
            return IrOp::CALLINDIRECT;
        // same stack effect as a different IR op
        case OpcodesPayne::_NULL:
            return IrOp::PUSH_NULL;
        case OpcodesPayne::LOCAL:
            return IrOp::LOCAL_STACK;
        case OpcodesPayne::STATIC:
            return IrOp::STATIC_STACK;
        case OpcodesPayne::GLOBAL:
            return IrOp::GLOBAL_STACK;
        case OpcodesPayne::ARRAY:
            return IrOp::ARRAY_STACK;
        default:
            return std::nullopt;
        }
    }

    std::vector<Instruction> DecompilerPayne::Lift(const Function& func) const
    {
        const auto& code = m_Disassembler.GetCode();
        const auto& r = m_Disassembler;
        const uint32_t codeSize = static_cast<uint32_t>(code.size());

        // only the function's own instructions, the range also has the bodies of functions it calls first
        const auto pcs = m_Disassembler.GetFunctionInstructions(func.GetInfo());

        std::vector<Instruction> result;
        result.reserve(pcs.size());

        for (uint32_t pc : pcs)
        {
            const uint8_t raw = code[pc];
            const auto op = static_cast<OpcodesPayne>(raw);

            // the operand bytes GetInsnSizePayne reads have to exist
            bool sizeReadable = true;
            if (op == OpcodesPayne::ENTER)
                sizeReadable = pc + 5 <= codeSize;
            else if (op == OpcodesPayne::SWITCH)
                sizeReadable = pc + 2 <= codeSize;
            else if (op == OpcodesPayne::STRING)
                sizeReadable = pc + 2 <= codeSize && (code[pc + 1] != 0 || pc + 4 <= codeSize);

            if (!sizeReadable)
            {
                result.emplace_back(IrOp::UNKNOWN, raw, pc, codeSize - pc);
                break;
            }

            const uint32_t size = static_cast<uint32_t>(GetInsnSizePayne(code.data(), pc));
            if (pc + size > codeSize)
            {
                result.emplace_back(IrOp::UNKNOWN, raw, pc, codeSize - pc);
                break;
            }

            auto make = [&](IrOp irOp) -> Instruction& {
                return result.emplace_back(irOp, raw, pc, size);
            };

            if (auto simple = MapSimple(op))
            {
                make(*simple);
                continue;
            }

            if (raw >= static_cast<uint8_t>(OpcodesPayne::PUSH_CONST_M16)) // PUSH_CONST_M16..PUSH_CONST_159, the value is op - 96
            {
                make(IrOp::PUSH_INT).Ints.assign({static_cast<int32_t>(raw) - static_cast<int32_t>(OpcodesPayne::PUSH_CONST_0)});
                continue;
            }

            switch (op)
            {
            case OpcodesPayne::PUSH_CONST_S16:
                make(IrOp::PUSH_INT).Ints.assign({r.GetS16(pc + 1)});
                break;
            case OpcodesPayne::PUSH_CONST_U32:
                make(IrOp::PUSH_INT).Ints.assign({static_cast<int32_t>(r.GetU32(pc + 1))});
                break;
            case OpcodesPayne::PUSH_CONST_F:
                make(IrOp::PUSH_FLOAT).Float = r.GetF32(pc + 1);
                break;

            case OpcodesPayne::NATIVE:
            {
                auto& ins = make(IrOp::NATIVE);
                ins.ArgCount = r.GetU8(pc + 1);
                ins.RetCount = r.GetU8(pc + 2);
                ins.Imm = r.GetU32(pc + 3); // handler address, there's no native table
                break;
            }

            case OpcodesPayne::ENTER:
            {
                auto& ins = make(IrOp::ENTER);
                ins.ArgCount = r.GetU8(pc + 1);
                ins.Imm = r.GetU16(pc + 2);
                break;
            }
            case OpcodesPayne::LEAVE:
            {
                auto& ins = make(IrOp::LEAVE);
                ins.ArgCount = r.GetU8(pc + 1);
                ins.RetCount = r.GetU8(pc + 2);
                break;
            }

            case OpcodesPayne::LOCAL_0:
            case OpcodesPayne::LOCAL_1:
            case OpcodesPayne::LOCAL_2:
            case OpcodesPayne::LOCAL_3:
            case OpcodesPayne::LOCAL_4:
            case OpcodesPayne::LOCAL_5:
            case OpcodesPayne::LOCAL_6:
            case OpcodesPayne::LOCAL_7:
                make(IrOp::LOCAL).Imm = raw - static_cast<uint8_t>(OpcodesPayne::LOCAL_0);
                break;

            // absolute targets
            case OpcodesPayne::J:
                make(IrOp::J).Target = r.GetU32(pc + 1);
                break;
            case OpcodesPayne::JZ:
                make(IrOp::JZ).Target = r.GetU32(pc + 1);
                break;
            case OpcodesPayne::JNZ:
                make(IrOp::JNZ).Target = r.GetU32(pc + 1);
                break;
            case OpcodesPayne::CALL:
                make(IrOp::CALL).Target = r.GetU32(pc + 1);
                break;

            case OpcodesPayne::SWITCH:
            {
                // count, then { i32 value, u32 absolute target }
                auto& ins = make(IrOp::SWITCH);
                uint8_t count = r.GetU8(pc + 1);
                ins.Cases.reserve(count);
                for (uint32_t i = 0, entry = pc + 2; i < count; i++, entry += 8)
                    ins.Cases.push_back({static_cast<int32_t>(r.GetU32(entry)), r.GetU32(entry + 4)});
                break;
            }

            case OpcodesPayne::STRING:
            {
                // length, then the characters inline. A length of 0 means a u16 length follows
                auto& ins = make(IrOp::PUSH_STRING);
                uint32_t length = r.GetU8(pc + 1);
                uint32_t start = pc + 2;
                if (length == 0)
                {
                    length = r.GetU16(pc + 2);
                    start = pc + 4;
                }

                for (uint32_t i = 0; i < length && r.GetU8(start + i); i++)
                    ins.Text += static_cast<char>(r.GetU8(start + i));
                break;
            }

            case OpcodesPayne::TEXT_LABEL_ASSIGN_STRING:
                make(IrOp::TEXT_LABEL_ASSIGN_STRING).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesPayne::TEXT_LABEL_ASSIGN_INT:
                make(IrOp::TEXT_LABEL_ASSIGN_INT).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesPayne::TEXT_LABEL_APPEND_STRING:
                make(IrOp::TEXT_LABEL_APPEND_STRING).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesPayne::TEXT_LABEL_APPEND_INT:
                make(IrOp::TEXT_LABEL_APPEND_INT).Imm = r.GetU8(pc + 1);
                break;

            default: // INVALID_OP_77..79
                make(IrOp::UNKNOWN);
                break;
            }
        }

        return result;
    }

    uint64_t DecompilerPayne::GetNativeHash(uint32_t index) const
    {
        return g_Game->GetNativeHashByHandler(index);
    }
}