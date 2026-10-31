#include "DecompilerRDR2.hpp"
#include "disasm/opcodes/OpcodesRDR2.hpp"
#include "ir/Function.hpp"

namespace scrDbgApp
{
    using namespace Decomp;

    static std::optional<IrOp> MapSimple(OpcodesRDR2 op) // ops whose RDR1 encoding maps 1:1 onto an operand less IR op
    {
        switch (op)
        {
        case OpcodesRDR2::NOP:
            return IrOp::NOP;
        case OpcodesRDR2::IADD:
            return IrOp::IADD;
        case OpcodesRDR2::ISUB:
            return IrOp::ISUB;
        case OpcodesRDR2::IMUL:
            return IrOp::IMUL;
        case OpcodesRDR2::IDIV:
            return IrOp::IDIV;
        case OpcodesRDR2::IMOD:
            return IrOp::IMOD;
        case OpcodesRDR2::INOT:
            return IrOp::INOT;
        case OpcodesRDR2::INEG:
            return IrOp::INEG;
        case OpcodesRDR2::IEQ:
            return IrOp::IEQ;
        case OpcodesRDR2::INE:
            return IrOp::INE;
        case OpcodesRDR2::IGT:
            return IrOp::IGT;
        case OpcodesRDR2::IGE:
            return IrOp::IGE;
        case OpcodesRDR2::ILT:
            return IrOp::ILT;
        case OpcodesRDR2::ILE:
            return IrOp::ILE;
        case OpcodesRDR2::FADD:
            return IrOp::FADD;
        case OpcodesRDR2::FSUB:
            return IrOp::FSUB;
        case OpcodesRDR2::FMUL:
            return IrOp::FMUL;
        case OpcodesRDR2::FDIV:
            return IrOp::FDIV;
        case OpcodesRDR2::FMOD:
            return IrOp::FMOD;
        case OpcodesRDR2::FNEG:
            return IrOp::FNEG;
        case OpcodesRDR2::FEQ:
            return IrOp::FEQ;
        case OpcodesRDR2::FNE:
            return IrOp::FNE;
        case OpcodesRDR2::FGT:
            return IrOp::FGT;
        case OpcodesRDR2::FGE:
            return IrOp::FGE;
        case OpcodesRDR2::FLT:
            return IrOp::FLT;
        case OpcodesRDR2::FLE:
            return IrOp::FLE;
        case OpcodesRDR2::VADD:
            return IrOp::VADD;
        case OpcodesRDR2::VSUB:
            return IrOp::VSUB;
        case OpcodesRDR2::VMUL:
            return IrOp::VMUL;
        case OpcodesRDR2::VDIV:
            return IrOp::VDIV;
        case OpcodesRDR2::VNEG:
            return IrOp::VNEG;
        case OpcodesRDR2::IAND:
            return IrOp::IAND;
        case OpcodesRDR2::IOR:
            return IrOp::IOR;
        case OpcodesRDR2::IXOR:
            return IrOp::IXOR;
        case OpcodesRDR2::I2F:
            return IrOp::I2F;
        case OpcodesRDR2::F2I:
            return IrOp::F2I;
        case OpcodesRDR2::F2V:
            return IrOp::F2V;
        case OpcodesRDR2::DUP:
            return IrOp::DUP;
        case OpcodesRDR2::DROP:
            return IrOp::DROP;
        case OpcodesRDR2::LOAD:
            return IrOp::LOAD;
        case OpcodesRDR2::STORE:
            return IrOp::STORE;
        case OpcodesRDR2::STORE_REV:
            return IrOp::STORE_REV;
        case OpcodesRDR2::LOAD_N:
            return IrOp::LOAD_N;
        case OpcodesRDR2::STORE_N:
            return IrOp::STORE_N;
        case OpcodesRDR2::TEXT_LABEL_COPY:
            return IrOp::TEXT_LABEL_COPY;
        case OpcodesRDR2::CATCH:
            return IrOp::CATCH;
        case OpcodesRDR2::THROW:
            return IrOp::THROW;
        case OpcodesRDR2::CALLINDIRECT:
            return IrOp::CALLINDIRECT;
        case OpcodesRDR2::VECTOR_MAKE:
            return IrOp::VECTOR_MAKE;
        case OpcodesRDR2::VECTOR_STORE:
            return IrOp::VECTOR_STORE;
        // same stack effect as a different IR op, REF_* copy the whole 8 byte slot
        case OpcodesRDR2::REF_LOAD:
            return IrOp::LOAD;
        case OpcodesRDR2::REF_STORE:
            return IrOp::STORE;
        case OpcodesRDR2::_NULL:
            return IrOp::PUSH_NULL;
        case OpcodesRDR2::PATCH_RET:
            return IrOp::NOP;
        default:
            return std::nullopt;
        }
    }

    std::vector<Instruction> DecompilerRDR2::Lift(const Function& func) const
    {
        const auto& code = m_Disassembler.GetCode();
        const auto& r = m_Disassembler;
        const uint32_t end = std::min<uint32_t>(func.GetEnd(), static_cast<uint32_t>(code.size()));

        std::vector<Instruction> result;
        result.reserve((end - func.GetStart()) / 2);

        uint32_t pc = func.GetStart();
        while (pc < end)
        {
            const uint8_t raw = code[pc];
            const auto op = static_cast<OpcodesRDR2>(raw);

            // the operand bytes GetInsnSizeRDR2 reads have to exist
            bool sizeReadable = true;
            if (op == OpcodesRDR2::ENTER || op == OpcodesRDR2::ARRAY)
                sizeReadable = pc + 5 <= end;
            else if (op == OpcodesRDR2::SWITCH || op == OpcodesRDR2::STRING)
                sizeReadable = pc + 2 <= end;

            if (!sizeReadable)
            {
                result.emplace_back(IrOp::UNKNOWN, raw, pc, end - pc);
                break;
            }

            const uint32_t size = static_cast<uint32_t>(GetInsnSizeRDR2(code.data(), pc));
            if (pc + size > end)
            {
                result.emplace_back(IrOp::UNKNOWN, raw, pc, end - pc);
                break;
            }

            auto make = [&](IrOp irOp) -> Instruction& {
                return result.emplace_back(irOp, raw, pc, size);
            };

            if (auto simple = MapSimple(op))
            {
                make(*simple);
                pc += size;
                continue;
            }

            const uint32_t relative = static_cast<uint32_t>(static_cast<int64_t>(pc) + 3 + r.GetS16(pc + 1));

            switch (op)
            {
            case OpcodesRDR2::PUSH_CONST_U8:
                make(IrOp::PUSH_INT).Ints.assign({r.GetU8(pc + 1)});
                break;
            case OpcodesRDR2::PUSH_CONST_U8_U8:
                make(IrOp::PUSH_INT).Ints.assign({r.GetU8(pc + 1), r.GetU8(pc + 2)});
                break;
            case OpcodesRDR2::PUSH_CONST_U8_U8_U8:
                make(IrOp::PUSH_INT).Ints.assign({r.GetU8(pc + 1), r.GetU8(pc + 2), r.GetU8(pc + 3)});
                break;
            case OpcodesRDR2::PUSH_CONST_U32:
                make(IrOp::PUSH_INT).Ints.assign({static_cast<int32_t>(r.GetU32(pc + 1))});
                break;
            case OpcodesRDR2::PUSH_CONST_S16:
                make(IrOp::PUSH_INT).Ints.assign({r.GetS16(pc + 1)});
                break;
            case OpcodesRDR2::PUSH_CONST_U24:
                make(IrOp::PUSH_INT).Ints.assign({static_cast<int32_t>(r.GetU24(pc + 1))});
                break;
            case OpcodesRDR2::PUSH_CONST_M1:
            case OpcodesRDR2::PUSH_CONST_0:
            case OpcodesRDR2::PUSH_CONST_1:
            case OpcodesRDR2::PUSH_CONST_2:
            case OpcodesRDR2::PUSH_CONST_3:
            case OpcodesRDR2::PUSH_CONST_4:
            case OpcodesRDR2::PUSH_CONST_5:
            case OpcodesRDR2::PUSH_CONST_6:
            case OpcodesRDR2::PUSH_CONST_7:
                make(IrOp::PUSH_INT).Ints.assign({static_cast<int32_t>(raw) - static_cast<int32_t>(OpcodesRDR2::PUSH_CONST_0)});
                break;
            case OpcodesRDR2::PUSH_CONST_F:
                make(IrOp::PUSH_FLOAT).Float = r.GetF32(pc + 1);
                break;
            case OpcodesRDR2::PUSH_CONST_FM1:
            case OpcodesRDR2::PUSH_CONST_F0:
            case OpcodesRDR2::PUSH_CONST_F1:
            case OpcodesRDR2::PUSH_CONST_F2:
            case OpcodesRDR2::PUSH_CONST_F3:
            case OpcodesRDR2::PUSH_CONST_F4:
            case OpcodesRDR2::PUSH_CONST_F5:
            case OpcodesRDR2::PUSH_CONST_F6:
            case OpcodesRDR2::PUSH_CONST_F7:
                make(IrOp::PUSH_FLOAT).Float = static_cast<float>(static_cast<int32_t>(raw) - static_cast<int32_t>(OpcodesRDR2::PUSH_CONST_F0));
                break;

            case OpcodesRDR2::NATIVE:
            {
                // args in bits 1-5, one return at most, the high bits of the index ride along in the first byte
                auto& ins = make(IrOp::NATIVE);
                uint8_t first = r.GetU8(pc + 1);
                ins.ArgCount = static_cast<uint8_t>((first >> 1) & 0x1F);
                ins.RetCount = static_cast<uint8_t>(first & 1);
                ins.Imm = ((static_cast<uint32_t>(first) * 4) & 0xFF00) | r.GetU8(pc + 2);
                break;
            }

            case OpcodesRDR2::ENTER:
            {
                auto& ins = make(IrOp::ENTER);
                ins.ArgCount = r.GetU8(pc + 1);
                ins.Imm = r.GetU16(pc + 2);
                break;
            }
            case OpcodesRDR2::LEAVE:
            {
                auto& ins = make(IrOp::LEAVE);
                ins.ArgCount = r.GetU8(pc + 1);
                ins.RetCount = r.GetU8(pc + 2);
                break;
            }
            case OpcodesRDR2::LEAVE_0_0:
            case OpcodesRDR2::LEAVE_0_1:
            case OpcodesRDR2::LEAVE_0_2:
            case OpcodesRDR2::LEAVE_0_3:
            case OpcodesRDR2::LEAVE_1_0:
            case OpcodesRDR2::LEAVE_1_1:
            case OpcodesRDR2::LEAVE_1_2:
            case OpcodesRDR2::LEAVE_1_3:
            case OpcodesRDR2::LEAVE_2_0:
            case OpcodesRDR2::LEAVE_2_1:
            case OpcodesRDR2::LEAVE_2_2:
            case OpcodesRDR2::LEAVE_2_3:
            case OpcodesRDR2::LEAVE_3_0:
            case OpcodesRDR2::LEAVE_3_1:
            case OpcodesRDR2::LEAVE_3_2:
            case OpcodesRDR2::LEAVE_3_3:
            {
                // args and returns baked into the opcode
                auto& ins = make(IrOp::LEAVE);
                uint8_t encoded = raw - static_cast<uint8_t>(OpcodesRDR2::LEAVE_0_0);
                ins.ArgCount = encoded >> 2;
                ins.RetCount = encoded & 3;
                break;
            }

            case OpcodesRDR2::ARRAY_U8:
                make(IrOp::ARRAY).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::ARRAY_U8_LOAD:
                make(IrOp::ARRAY_LOAD).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::ARRAY_U8_STORE:
                make(IrOp::ARRAY_STORE).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::LOCAL_U8:
                make(IrOp::LOCAL).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::LOCAL_U8_LOAD:
                make(IrOp::LOCAL_LOAD).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::LOCAL_U8_STORE:
                make(IrOp::LOCAL_STORE).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::STATIC_U8:
                make(IrOp::STATIC).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::STATIC_U8_LOAD:
                make(IrOp::STATIC_LOAD).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::STATIC_U8_STORE:
                make(IrOp::STATIC_STORE).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::ARRAY_U16:
                make(IrOp::ARRAY).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::ARRAY_U16_LOAD:
                make(IrOp::ARRAY_LOAD).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::ARRAY_U16_STORE:
                make(IrOp::ARRAY_STORE).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::LOCAL_U16:
                make(IrOp::LOCAL).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::LOCAL_U16_LOAD:
                make(IrOp::LOCAL_LOAD).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::LOCAL_U16_STORE:
                make(IrOp::LOCAL_STORE).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::STATIC_U16:
                make(IrOp::STATIC).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::STATIC_U16_LOAD:
                make(IrOp::STATIC_LOAD).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::STATIC_U16_STORE:
                make(IrOp::STATIC_STORE).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::GLOBAL_U16:
                make(IrOp::GLOBAL).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::GLOBAL_U16_LOAD:
                make(IrOp::GLOBAL_LOAD).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::GLOBAL_U16_STORE:
                make(IrOp::GLOBAL_STORE).Imm = r.GetU16(pc + 1);
                break;
            case OpcodesRDR2::GLOBAL_U24:
                make(IrOp::GLOBAL).Imm = r.GetU24(pc + 1);
                break;
            case OpcodesRDR2::GLOBAL_U24_LOAD:
                make(IrOp::GLOBAL_LOAD).Imm = r.GetU24(pc + 1);
                break;
            case OpcodesRDR2::GLOBAL_U24_STORE:
                make(IrOp::GLOBAL_STORE).Imm = r.GetU24(pc + 1);
                break;
            case OpcodesRDR2::IADD_U8:
                make(IrOp::IADD_IMM).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::IADD_S16:
                make(IrOp::IADD_IMM).Imm = r.GetS16(pc + 1);
                break;
            case OpcodesRDR2::IMUL_U8:
                make(IrOp::IMUL_IMM).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::IMUL_S16:
                make(IrOp::IMUL_IMM).Imm = r.GetS16(pc + 1);
                break;
            case OpcodesRDR2::TEXT_LABEL_ASSIGN_STRING:
                make(IrOp::TEXT_LABEL_ASSIGN_STRING).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::TEXT_LABEL_ASSIGN_INT:
                make(IrOp::TEXT_LABEL_ASSIGN_INT).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::TEXT_LABEL_APPEND_STRING:
                make(IrOp::TEXT_LABEL_APPEND_STRING).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesRDR2::TEXT_LABEL_APPEND_INT:
                make(IrOp::TEXT_LABEL_APPEND_INT).Imm = r.GetU8(pc + 1);
                break;

            // field offsets are in bytes, fields are named in 4 byte units like the statics/globals
            case OpcodesRDR2::IOFFSET_U8_LOAD:
            case OpcodesRDR2::IOFFSET_U8_STORE:
            case OpcodesRDR2::IOFFSET_S16_LOAD:
            case OpcodesRDR2::IOFFSET_S16_STORE:
            {
                bool isU8 = op == OpcodesRDR2::IOFFSET_U8_LOAD || op == OpcodesRDR2::IOFFSET_U8_STORE;
                bool isLoad = op == OpcodesRDR2::IOFFSET_U8_LOAD || op == OpcodesRDR2::IOFFSET_S16_LOAD;
                int32_t bytes = isU8 ? r.GetU8(pc + 1) : r.GetS16(pc + 1);
                if (bytes % 4 != 0)
                {
                    make(IrOp::UNKNOWN);
                    break;
                }

                make(isLoad ? IrOp::IOFFSET_IMM_LOAD : IrOp::IOFFSET_IMM_STORE).Imm = bytes / 4;
                break;
            }

            case OpcodesRDR2::J:
                make(IrOp::J).Target = relative;
                break;
            case OpcodesRDR2::JZ:
                make(IrOp::JZ).Target = relative;
                break;
            // these jump when the comparison is true, so they're the opposite *_JZ
            case OpcodesRDR2::IEQ_J:
                make(IrOp::INE_JZ).Target = relative;
                break;
            case OpcodesRDR2::INE_J:
                make(IrOp::IEQ_JZ).Target = relative;
                break;
            case OpcodesRDR2::ILT_J:
                make(IrOp::IGE_JZ).Target = relative;
                break;
            case OpcodesRDR2::ILE_J:
                make(IrOp::IGT_JZ).Target = relative;
                break;
            case OpcodesRDR2::IGT_J:
                make(IrOp::ILE_JZ).Target = relative;
                break;
            case OpcodesRDR2::IGE_J:
                make(IrOp::ILT_JZ).Target = relative;
                break;

            case OpcodesRDR2::CALL:
            case OpcodesRDR2::CALL_U8H_1:
            case OpcodesRDR2::CALL_U8H_2:
            case OpcodesRDR2::CALL_U8H_3:
            case OpcodesRDR2::CALL_U8H_4:
            case OpcodesRDR2::CALL_U8H_5:
            case OpcodesRDR2::CALL_U8H_6:
            case OpcodesRDR2::CALL_U8H_7:
            case OpcodesRDR2::CALL_U8H_8:
            case OpcodesRDR2::CALL_U8H_9:
            case OpcodesRDR2::CALL_U8H_A:
            case OpcodesRDR2::CALL_U8H_B:
            case OpcodesRDR2::CALL_U8H_C:
            case OpcodesRDR2::CALL_U8H_D:
            case OpcodesRDR2::CALL_U8H_E:
            case OpcodesRDR2::CALL_U8H_F:
                // the opcode carries bits 16-19 of the target
                make(IrOp::CALL).Target = (static_cast<uint32_t>(raw - static_cast<uint8_t>(OpcodesRDR2::CALL)) << 16) | r.GetU16(pc + 1);
                break;

            case OpcodesRDR2::SWITCH:
            {
                // count, then { u32 value, u16 offset relative to the end of the entry }
                auto& ins = make(IrOp::SWITCH);
                uint8_t count = r.GetU8(pc + 1);
                ins.Cases.reserve(count);
                for (uint32_t i = 0, entry = pc + 2; i < count; i++, entry += 6)
                    ins.Cases.push_back({static_cast<int32_t>(r.GetU32(entry)), entry + 6 + r.GetU16(entry + 4)});
                break;
            }

            case OpcodesRDR2::STRING:
            {
                // length, then the characters inline
                auto& ins = make(IrOp::PUSH_STRING);
                uint8_t length = r.GetU8(pc + 1);
                for (uint32_t i = 0; i < length && r.GetU8(pc + 2 + i); i++)
                    ins.Text += static_cast<char>(r.GetU8(pc + 2 + i));
                break;
            }

            case OpcodesRDR2::ARRAY:
            {
                // size in bytes, then the data inline
                auto& ins = make(IrOp::PUSH_ARRAY);
                uint32_t bytes = r.GetU32(pc + 1);
                for (uint32_t i = 0; i + 4 <= bytes; i += 4)
                    ins.Ints.push_back(static_cast<int32_t>(r.GetU32(pc + 5 + i)));
                break;
            }

            default: // PATCH_TRAP_*, CALL_PATCH, CALL_OUT_OF_PATCH, no script allocates patch data
                make(IrOp::UNKNOWN);
                break;
            }

            pc += size;
        }

        return result;
    }
}