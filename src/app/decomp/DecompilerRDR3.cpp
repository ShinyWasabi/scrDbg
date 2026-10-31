#include "DecompilerRDR3.hpp"
#include "disasm/opcodes/OpcodesRDR3.hpp"
#include "ir/Function.hpp"

namespace scrDbgApp
{
    using namespace Decomp;

    static std::optional<IrOp> MapSimple(OpcodesRDR3 op) // ops whose RDR3 encoding maps 1:1 onto an operand less IR op
    {
        switch (op)
        {
        case OpcodesRDR3::NOP:
            return IrOp::NOP;
        case OpcodesRDR3::IADD:
            return IrOp::IADD;
        case OpcodesRDR3::ISUB:
            return IrOp::ISUB;
        case OpcodesRDR3::IMUL:
            return IrOp::IMUL;
        case OpcodesRDR3::IDIV:
            return IrOp::IDIV;
        case OpcodesRDR3::IMOD:
            return IrOp::IMOD;
        case OpcodesRDR3::INOT:
            return IrOp::INOT;
        case OpcodesRDR3::INEG:
            return IrOp::INEG;
        case OpcodesRDR3::IEQ:
            return IrOp::IEQ;
        case OpcodesRDR3::INE:
            return IrOp::INE;
        case OpcodesRDR3::IGT:
            return IrOp::IGT;
        case OpcodesRDR3::IGE:
            return IrOp::IGE;
        case OpcodesRDR3::ILT:
            return IrOp::ILT;
        case OpcodesRDR3::ILE:
            return IrOp::ILE;
        case OpcodesRDR3::FADD:
            return IrOp::FADD;
        case OpcodesRDR3::FSUB:
            return IrOp::FSUB;
        case OpcodesRDR3::FMUL:
            return IrOp::FMUL;
        case OpcodesRDR3::FDIV:
            return IrOp::FDIV;
        case OpcodesRDR3::FMOD:
            return IrOp::FMOD;
        case OpcodesRDR3::FNEG:
            return IrOp::FNEG;
        case OpcodesRDR3::FEQ:
            return IrOp::FEQ;
        case OpcodesRDR3::FNE:
            return IrOp::FNE;
        case OpcodesRDR3::FGT:
            return IrOp::FGT;
        case OpcodesRDR3::FGE:
            return IrOp::FGE;
        case OpcodesRDR3::FLT:
            return IrOp::FLT;
        case OpcodesRDR3::FLE:
            return IrOp::FLE;
        case OpcodesRDR3::VADD:
            return IrOp::VADD;
        case OpcodesRDR3::VSUB:
            return IrOp::VSUB;
        case OpcodesRDR3::VMUL:
            return IrOp::VMUL;
        case OpcodesRDR3::VDIV:
            return IrOp::VDIV;
        case OpcodesRDR3::VNEG:
            return IrOp::VNEG;
        case OpcodesRDR3::IAND:
            return IrOp::IAND;
        case OpcodesRDR3::IOR:
            return IrOp::IOR;
        case OpcodesRDR3::IXOR:
            return IrOp::IXOR;
        case OpcodesRDR3::I2F:
            return IrOp::I2F;
        case OpcodesRDR3::F2I:
            return IrOp::F2I;
        case OpcodesRDR3::F2V:
            return IrOp::F2V;
        case OpcodesRDR3::DUP:
            return IrOp::DUP;
        case OpcodesRDR3::DROP:
            return IrOp::DROP;
        case OpcodesRDR3::LOAD:
            return IrOp::LOAD;
        case OpcodesRDR3::STORE:
            return IrOp::STORE;
        case OpcodesRDR3::STORE_REV:
            return IrOp::STORE_REV;
        case OpcodesRDR3::LOAD_N:
            return IrOp::LOAD_N;
        case OpcodesRDR3::STORE_N:
            return IrOp::STORE_N;
        case OpcodesRDR3::IOFFSET:
            return IrOp::IOFFSET;
        case OpcodesRDR3::STRING:
            return IrOp::STRING;
        case OpcodesRDR3::STRINGHASH:
            return IrOp::STRINGHASH;
        case OpcodesRDR3::TEXT_LABEL_COPY:
            return IrOp::TEXT_LABEL_COPY;
        case OpcodesRDR3::CATCH:
            return IrOp::CATCH;
        case OpcodesRDR3::THROW:
            return IrOp::THROW;
        case OpcodesRDR3::CALLINDIRECT:
            return IrOp::CALLINDIRECT;
        case OpcodesRDR3::LOCAL_LOAD_S:
            return IrOp::LOCAL_LOAD_S;
        case OpcodesRDR3::LOCAL_STORE_S:
            return IrOp::LOCAL_STORE_S;
        case OpcodesRDR3::LOCAL_STORE_SR:
            return IrOp::LOCAL_STORE_SR;
        case OpcodesRDR3::STATIC_LOAD_S:
            return IrOp::STATIC_LOAD_S;
        case OpcodesRDR3::STATIC_STORE_S:
            return IrOp::STATIC_STORE_S;
        case OpcodesRDR3::STATIC_STORE_SR:
            return IrOp::STATIC_STORE_SR;
        case OpcodesRDR3::GLOBAL_LOAD_S:
            return IrOp::GLOBAL_LOAD_S;
        case OpcodesRDR3::GLOBAL_STORE_S:
            return IrOp::GLOBAL_STORE_S;
        case OpcodesRDR3::GLOBAL_STORE_SR:
            return IrOp::GLOBAL_STORE_SR;
        case OpcodesRDR3::LOAD_N_S:
            return IrOp::LOAD_N_S;
        case OpcodesRDR3::STORE_N_S:
            return IrOp::STORE_N_S;
        case OpcodesRDR3::STORE_N_SR:
            return IrOp::STORE_N_SR;
        }

        return std::nullopt;
    }

    std::vector<Instruction> DecompilerRDR3::Lift(const Function& func) const
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
            const auto op = static_cast<OpcodesRDR3>(raw);

            if ((op == OpcodesRDR3::ENTER && pc + 5 > end) || (op == OpcodesRDR3::SWITCH && pc + 3 > end))
            {
                result.emplace_back(IrOp::UNKNOWN, raw, pc, end - pc);
                break;
            }

            const uint32_t size = static_cast<uint32_t>(GetInsnSizeRDR3(code.data(), pc));
            if (pc + size > end)
            {
                result.emplace_back(IrOp::UNKNOWN, raw, pc, end - pc);
                break;
            }

            auto make = [&](IrOp irOp) -> Instruction& {
                return result.emplace_back(irOp, raw, pc, size);
            };

            auto pushInts = [&](std::initializer_list<int32_t> values) {
                make(IrOp::PUSH_INT).Ints.assign(values);
            };

            auto withImm = [&](IrOp irOp, int64_t imm) {
                make(irOp).Imm = imm;
            };

            if (auto simple = MapSimple(op))
            {
                make(*simple);
                pc += size;
                continue;
            }

            switch (op)
            {
            case OpcodesRDR3::PUSH_CONST_U8:
                pushInts({r.GetU8(pc + 1)});
                break;
            case OpcodesRDR3::PUSH_CONST_U8_U8:
                pushInts({r.GetU8(pc + 1), r.GetU8(pc + 2)});
                break;
            case OpcodesRDR3::PUSH_CONST_U8_U8_U8:
                pushInts({r.GetU8(pc + 1), r.GetU8(pc + 2), r.GetU8(pc + 3)});
                break;
            case OpcodesRDR3::PUSH_CONST_U32:
                pushInts({static_cast<int32_t>(r.GetU32(pc + 1))});
                break;
            case OpcodesRDR3::PUSH_CONST_S16:
                pushInts({r.GetS16(pc + 1)});
                break;
            case OpcodesRDR3::PUSH_CONST_U24:
                pushInts({static_cast<int32_t>(r.GetU24(pc + 1))});
                break;
            // opcode values aren't consecutive in RDR3, so no arithmetic on them
            case OpcodesRDR3::PUSH_CONST_M1:
                pushInts({-1});
                break;
            case OpcodesRDR3::PUSH_CONST_0:
                pushInts({0});
                break;
            case OpcodesRDR3::PUSH_CONST_1:
                pushInts({1});
                break;
            case OpcodesRDR3::PUSH_CONST_2:
                pushInts({2});
                break;
            case OpcodesRDR3::PUSH_CONST_3:
                pushInts({3});
                break;
            case OpcodesRDR3::PUSH_CONST_4:
                pushInts({4});
                break;
            case OpcodesRDR3::PUSH_CONST_5:
                pushInts({5});
                break;
            case OpcodesRDR3::PUSH_CONST_6:
                pushInts({6});
                break;
            case OpcodesRDR3::PUSH_CONST_7:
                pushInts({7});
                break;

            case OpcodesRDR3::PUSH_CONST_F:
                make(IrOp::PUSH_FLOAT).Float = r.GetF32(pc + 1);
                break;
            case OpcodesRDR3::PUSH_CONST_FM1:
                make(IrOp::PUSH_FLOAT).Float = -1.0f;
                break;
            case OpcodesRDR3::PUSH_CONST_F0:
                make(IrOp::PUSH_FLOAT).Float = 0.0f;
                break;
            case OpcodesRDR3::PUSH_CONST_F1:
                make(IrOp::PUSH_FLOAT).Float = 1.0f;
                break;
            case OpcodesRDR3::PUSH_CONST_F2:
                make(IrOp::PUSH_FLOAT).Float = 2.0f;
                break;
            case OpcodesRDR3::PUSH_CONST_F3:
                make(IrOp::PUSH_FLOAT).Float = 3.0f;
                break;
            case OpcodesRDR3::PUSH_CONST_F4:
                make(IrOp::PUSH_FLOAT).Float = 4.0f;
                break;
            case OpcodesRDR3::PUSH_CONST_F5:
                make(IrOp::PUSH_FLOAT).Float = 5.0f;
                break;
            case OpcodesRDR3::PUSH_CONST_F6:
                make(IrOp::PUSH_FLOAT).Float = 6.0f;
                break;
            case OpcodesRDR3::PUSH_CONST_F7:
                make(IrOp::PUSH_FLOAT).Float = 7.0f;
                break;

            case OpcodesRDR3::NATIVE:
            {
                auto& ins = make(IrOp::NATIVE);
                uint8_t packed = r.GetU8(pc + 1);
                ins.ArgCount = static_cast<uint8_t>(packed >> 2);
                ins.RetCount = static_cast<uint8_t>(packed & 3);
                ins.Imm = (r.GetU8(pc + 2) << 8) | r.GetU8(pc + 3);
                break;
            }

            case OpcodesRDR3::ENTER:
            {
                auto& ins = make(IrOp::ENTER);
                ins.ArgCount = r.GetU8(pc + 1);
                ins.Imm = r.GetU16(pc + 2);
                break;
            }
            case OpcodesRDR3::LEAVE:
            {
                auto& ins = make(IrOp::LEAVE);
                ins.ArgCount = r.GetU8(pc + 1);
                ins.RetCount = r.GetU8(pc + 2);
                break;
            }

            case OpcodesRDR3::ARRAY_U8:
                withImm(IrOp::ARRAY, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::ARRAY_U8_LOAD:
                withImm(IrOp::ARRAY_LOAD, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::ARRAY_U8_STORE:
                withImm(IrOp::ARRAY_STORE, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::ARRAY_U16:
                withImm(IrOp::ARRAY, r.GetU16(pc + 1));
                break;
            case OpcodesRDR3::ARRAY_U16_LOAD:
                withImm(IrOp::ARRAY_LOAD, r.GetU16(pc + 1));
                break;
            case OpcodesRDR3::ARRAY_U16_STORE:
                withImm(IrOp::ARRAY_STORE, r.GetU16(pc + 1));
                break;

            case OpcodesRDR3::LOCAL_U8:
                withImm(IrOp::LOCAL, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::LOCAL_U8_LOAD:
                withImm(IrOp::LOCAL_LOAD, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::LOCAL_U8_STORE:
                withImm(IrOp::LOCAL_STORE, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::LOCAL_U16:
                withImm(IrOp::LOCAL, r.GetU16(pc + 1));
                break;
            case OpcodesRDR3::LOCAL_U16_LOAD:
                withImm(IrOp::LOCAL_LOAD, r.GetU16(pc + 1));
                break;
            case OpcodesRDR3::LOCAL_U16_STORE:
                withImm(IrOp::LOCAL_STORE, r.GetU16(pc + 1));
                break;

            case OpcodesRDR3::STATIC_U8:
                withImm(IrOp::STATIC, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::STATIC_U8_LOAD:
                withImm(IrOp::STATIC_LOAD, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::STATIC_U8_STORE:
                withImm(IrOp::STATIC_STORE, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::STATIC_U16:
                withImm(IrOp::STATIC, r.GetU16(pc + 1));
                break;
            case OpcodesRDR3::STATIC_U16_LOAD:
                withImm(IrOp::STATIC_LOAD, r.GetU16(pc + 1));
                break;
            case OpcodesRDR3::STATIC_U16_STORE:
                withImm(IrOp::STATIC_STORE, r.GetU16(pc + 1));
                break;
            case OpcodesRDR3::STATIC_U24:
                withImm(IrOp::STATIC, r.GetU24(pc + 1));
                break;
            case OpcodesRDR3::STATIC_U24_LOAD:
                withImm(IrOp::STATIC_LOAD, r.GetU24(pc + 1));
                break;
            case OpcodesRDR3::STATIC_U24_STORE:
                withImm(IrOp::STATIC_STORE, r.GetU24(pc + 1));
                break;

            case OpcodesRDR3::GLOBAL_U16:
                withImm(IrOp::GLOBAL, r.GetU16(pc + 1));
                break;
            case OpcodesRDR3::GLOBAL_U16_LOAD:
                withImm(IrOp::GLOBAL_LOAD, r.GetU16(pc + 1));
                break;
            case OpcodesRDR3::GLOBAL_U16_STORE:
                withImm(IrOp::GLOBAL_STORE, r.GetU16(pc + 1));
                break;
            case OpcodesRDR3::GLOBAL_U24:
                withImm(IrOp::GLOBAL, r.GetU24(pc + 1));
                break;
            case OpcodesRDR3::GLOBAL_U24_LOAD:
                withImm(IrOp::GLOBAL_LOAD, r.GetU24(pc + 1));
                break;
            case OpcodesRDR3::GLOBAL_U24_STORE:
                withImm(IrOp::GLOBAL_STORE, r.GetU24(pc + 1));
                break;

            case OpcodesRDR3::IADD_U8:
                withImm(IrOp::IADD_IMM, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::IADD_S16:
                withImm(IrOp::IADD_IMM, r.GetS16(pc + 1));
                break;
            case OpcodesRDR3::IMUL_U8:
                withImm(IrOp::IMUL_IMM, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::IMUL_S16:
                withImm(IrOp::IMUL_IMM, r.GetS16(pc + 1));
                break;

            case OpcodesRDR3::IOFFSET_U8:
                withImm(IrOp::IOFFSET_IMM, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::IOFFSET_U8_LOAD:
                withImm(IrOp::IOFFSET_IMM_LOAD, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::IOFFSET_U8_STORE:
                withImm(IrOp::IOFFSET_IMM_STORE, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::IOFFSET_S16:
                withImm(IrOp::IOFFSET_IMM, r.GetS16(pc + 1));
                break;
            case OpcodesRDR3::IOFFSET_S16_LOAD:
                withImm(IrOp::IOFFSET_IMM_LOAD, r.GetS16(pc + 1));
                break;
            case OpcodesRDR3::IOFFSET_S16_STORE:
                withImm(IrOp::IOFFSET_IMM_STORE, r.GetS16(pc + 1));
                break;

            case OpcodesRDR3::J:
            case OpcodesRDR3::JZ:
            case OpcodesRDR3::IEQ_JZ:
            case OpcodesRDR3::INE_JZ:
            case OpcodesRDR3::IGT_JZ:
            case OpcodesRDR3::IGE_JZ:
            case OpcodesRDR3::ILT_JZ:
            case OpcodesRDR3::ILE_JZ:
            {
                IrOp jump = IrOp::J;
                switch (op)
                {
                case OpcodesRDR3::JZ:
                    jump = IrOp::JZ;
                    break;
                case OpcodesRDR3::IEQ_JZ:
                    jump = IrOp::IEQ_JZ;
                    break;
                case OpcodesRDR3::INE_JZ:
                    jump = IrOp::INE_JZ;
                    break;
                case OpcodesRDR3::IGT_JZ:
                    jump = IrOp::IGT_JZ;
                    break;
                case OpcodesRDR3::IGE_JZ:
                    jump = IrOp::IGE_JZ;
                    break;
                case OpcodesRDR3::ILT_JZ:
                    jump = IrOp::ILT_JZ;
                    break;
                case OpcodesRDR3::ILE_JZ:
                    jump = IrOp::ILE_JZ;
                    break;
                }

                auto& ins = make(jump);
                ins.Target = static_cast<uint32_t>(static_cast<int64_t>(pc) + 3 + r.GetS16(pc + 1));
                break;
            }

            case OpcodesRDR3::CALL:
                make(IrOp::CALL).Target = r.GetU24(pc + 1);
                break;

            case OpcodesRDR3::SWITCH:
            {
                auto& ins = make(IrOp::SWITCH);
                uint16_t count = r.GetU16(pc + 1); // 16 bit count, unlike GTA V
                ins.Cases.reserve(count);
                for (uint32_t i = 0, entry = pc + 3; i < count; i++, entry += 6)
                    ins.Cases.push_back({static_cast<int32_t>(r.GetU32(entry)), static_cast<uint32_t>(static_cast<int64_t>(entry) + 6 + r.GetS16(entry + 4))});
                break;
            }

            case OpcodesRDR3::TEXT_LABEL_ASSIGN_STRING:
                withImm(IrOp::TEXT_LABEL_ASSIGN_STRING, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::TEXT_LABEL_ASSIGN_INT:
                withImm(IrOp::TEXT_LABEL_ASSIGN_INT, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::TEXT_LABEL_APPEND_STRING:
                withImm(IrOp::TEXT_LABEL_APPEND_STRING, r.GetU8(pc + 1));
                break;
            case OpcodesRDR3::TEXT_LABEL_APPEND_INT:
                withImm(IrOp::TEXT_LABEL_APPEND_INT, r.GetU8(pc + 1));
                break;

            default:
                make(IrOp::UNKNOWN);
                break;
            }

            pc += size;
        }

        return result;
    }
}