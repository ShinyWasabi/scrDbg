#include "DecompilerGTA5.hpp"
#include "disasm/opcodes/OpcodesGTA5.hpp"
#include "ir/Function.hpp"

namespace scrDbgApp
{
    using namespace Decomp;

    struct Reader
    {
        const std::vector<uint8_t>& Code;

        // code duplication!!!

        uint8_t U8(uint32_t pc) const
        {
            return pc < Code.size() ? Code[pc] : 0;
        }

        uint16_t U16(uint32_t pc) const
        {
            return static_cast<uint16_t>(U8(pc) | (U8(pc + 1) << 8));
        }

        int16_t S16(uint32_t pc) const
        {
            return static_cast<int16_t>(U16(pc));
        }

        uint32_t U24(uint32_t pc) const
        {
            return static_cast<uint32_t>(U8(pc) | (U8(pc + 1) << 8) | (U8(pc + 2) << 16));
        }

        uint32_t U32(uint32_t pc) const
        {
            return static_cast<uint32_t>(U8(pc) | (U8(pc + 1) << 8) | (U8(pc + 2) << 16) | (static_cast<uint32_t>(U8(pc + 3)) << 24));
        }

        float F32(uint32_t pc) const
        {
            uint32_t bits = U32(pc);
            float value;
            std::memcpy(&value, &bits, sizeof(value));
            return value;
        }
    };

    static std::optional<IrOp> MapSimple(OpcodesGTA5 op) // ops whose GTA V encoding maps 1:1 onto an operand less IR op
    {
        switch (op)
        {
        case OpcodesGTA5::NOP:
            return IrOp::NOP;
        case OpcodesGTA5::IADD:
            return IrOp::IADD;
        case OpcodesGTA5::ISUB:
            return IrOp::ISUB;
        case OpcodesGTA5::IMUL:
            return IrOp::IMUL;
        case OpcodesGTA5::IDIV:
            return IrOp::IDIV;
        case OpcodesGTA5::IMOD:
            return IrOp::IMOD;
        case OpcodesGTA5::INOT:
            return IrOp::INOT;
        case OpcodesGTA5::INEG:
            return IrOp::INEG;
        case OpcodesGTA5::IEQ:
            return IrOp::IEQ;
        case OpcodesGTA5::INE:
            return IrOp::INE;
        case OpcodesGTA5::IGT:
            return IrOp::IGT;
        case OpcodesGTA5::IGE:
            return IrOp::IGE;
        case OpcodesGTA5::ILT:
            return IrOp::ILT;
        case OpcodesGTA5::ILE:
            return IrOp::ILE;
        case OpcodesGTA5::FADD:
            return IrOp::FADD;
        case OpcodesGTA5::FSUB:
            return IrOp::FSUB;
        case OpcodesGTA5::FMUL:
            return IrOp::FMUL;
        case OpcodesGTA5::FDIV:
            return IrOp::FDIV;
        case OpcodesGTA5::FMOD:
            return IrOp::FMOD;
        case OpcodesGTA5::FNEG:
            return IrOp::FNEG;
        case OpcodesGTA5::FEQ:
            return IrOp::FEQ;
        case OpcodesGTA5::FNE:
            return IrOp::FNE;
        case OpcodesGTA5::FGT:
            return IrOp::FGT;
        case OpcodesGTA5::FGE:
            return IrOp::FGE;
        case OpcodesGTA5::FLT:
            return IrOp::FLT;
        case OpcodesGTA5::FLE:
            return IrOp::FLE;
        case OpcodesGTA5::VADD:
            return IrOp::VADD;
        case OpcodesGTA5::VSUB:
            return IrOp::VSUB;
        case OpcodesGTA5::VMUL:
            return IrOp::VMUL;
        case OpcodesGTA5::VDIV:
            return IrOp::VDIV;
        case OpcodesGTA5::VNEG:
            return IrOp::VNEG;
        case OpcodesGTA5::IAND:
            return IrOp::IAND;
        case OpcodesGTA5::IOR:
            return IrOp::IOR;
        case OpcodesGTA5::IXOR:
            return IrOp::IXOR;
        case OpcodesGTA5::I2F:
            return IrOp::I2F;
        case OpcodesGTA5::F2I:
            return IrOp::F2I;
        case OpcodesGTA5::F2V:
            return IrOp::F2V;
        case OpcodesGTA5::DUP:
            return IrOp::DUP;
        case OpcodesGTA5::DROP:
            return IrOp::DROP;
        case OpcodesGTA5::LOAD:
            return IrOp::LOAD;
        case OpcodesGTA5::STORE:
            return IrOp::STORE;
        case OpcodesGTA5::STORE_REV:
            return IrOp::STORE_REV;
        case OpcodesGTA5::LOAD_N:
            return IrOp::LOAD_N;
        case OpcodesGTA5::STORE_N:
            return IrOp::STORE_N;
        case OpcodesGTA5::IOFFSET:
            return IrOp::IOFFSET;
        case OpcodesGTA5::STRING:
            return IrOp::STRING;
        case OpcodesGTA5::STRINGHASH:
            return IrOp::STRINGHASH;
        case OpcodesGTA5::TEXT_LABEL_COPY:
            return IrOp::TEXT_LABEL_COPY;
        case OpcodesGTA5::CATCH:
            return IrOp::CATCH;
        case OpcodesGTA5::THROW:
            return IrOp::THROW;
        case OpcodesGTA5::CALLINDIRECT:
            return IrOp::CALLINDIRECT;
        case OpcodesGTA5::IS_BIT_SET:
            return IrOp::IS_BIT_SET;
        }

        return std::nullopt;
    }

    std::vector<Instruction> DecompilerGTA5::Lift(const Function& func) const
    {
        const auto& code = m_Disassembler.GetCode();
        const Reader r{code};
        const uint32_t end = std::min<uint32_t>(func.GetEnd(), static_cast<uint32_t>(code.size()));

        std::vector<Instruction> result;
        result.reserve((end - func.GetStart()) / 2);

        uint32_t pc = func.GetStart();
        while (pc < end)
        {
            const uint8_t raw = code[pc];
            const auto op = static_cast<OpcodesGTA5>(raw);

            if ((op == OpcodesGTA5::ENTER && pc + 5 > end) || (op == OpcodesGTA5::SWITCH && pc + 2 > end))
            {
                result.emplace_back(IrOp::UNKNOWN, raw, pc, end - pc);
                break;
            }

            const uint32_t size = static_cast<uint32_t>(GetInsnSizeGTA5(code.data(), pc));
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
            case OpcodesGTA5::PUSH_CONST_U8:
                pushInts({r.U8(pc + 1)});
                break;
            case OpcodesGTA5::PUSH_CONST_U8_U8:
                pushInts({r.U8(pc + 1), r.U8(pc + 2)});
                break;
            case OpcodesGTA5::PUSH_CONST_U8_U8_U8:
                pushInts({r.U8(pc + 1), r.U8(pc + 2), r.U8(pc + 3)});
                break;
            case OpcodesGTA5::PUSH_CONST_U32:
                pushInts({static_cast<int32_t>(r.U32(pc + 1))});
                break;
            case OpcodesGTA5::PUSH_CONST_S16:
                pushInts({r.S16(pc + 1)});
                break;
            case OpcodesGTA5::PUSH_CONST_U24:
                pushInts({static_cast<int32_t>(r.U24(pc + 1))});
                break;
            case OpcodesGTA5::PUSH_CONST_M1:
            case OpcodesGTA5::PUSH_CONST_0:
            case OpcodesGTA5::PUSH_CONST_1:
            case OpcodesGTA5::PUSH_CONST_2:
            case OpcodesGTA5::PUSH_CONST_3:
            case OpcodesGTA5::PUSH_CONST_4:
            case OpcodesGTA5::PUSH_CONST_5:
            case OpcodesGTA5::PUSH_CONST_6:
            case OpcodesGTA5::PUSH_CONST_7:
                pushInts({static_cast<int32_t>(raw) - static_cast<int32_t>(OpcodesGTA5::PUSH_CONST_0)});
                break;

            case OpcodesGTA5::PUSH_CONST_F:
                make(IrOp::PUSH_FLOAT).Float = r.F32(pc + 1);
                break;
            case OpcodesGTA5::PUSH_CONST_FM1:
            case OpcodesGTA5::PUSH_CONST_F0:
            case OpcodesGTA5::PUSH_CONST_F1:
            case OpcodesGTA5::PUSH_CONST_F2:
            case OpcodesGTA5::PUSH_CONST_F3:
            case OpcodesGTA5::PUSH_CONST_F4:
            case OpcodesGTA5::PUSH_CONST_F5:
            case OpcodesGTA5::PUSH_CONST_F6:
            case OpcodesGTA5::PUSH_CONST_F7:
                make(IrOp::PUSH_FLOAT).Float = static_cast<float>(static_cast<int32_t>(raw) - static_cast<int32_t>(OpcodesGTA5::PUSH_CONST_F0));
                break;

            case OpcodesGTA5::NATIVE:
            {
                auto& ins = make(IrOp::NATIVE);
                uint8_t packed = r.U8(pc + 1);
                ins.ArgCount = static_cast<uint8_t>(packed >> 2);
                ins.RetCount = static_cast<uint8_t>(packed & 3);
                ins.Imm = (r.U8(pc + 2) << 8) | r.U8(pc + 3);
                break;
            }

            case OpcodesGTA5::ENTER:
            {
                auto& ins = make(IrOp::ENTER);
                ins.ArgCount = r.U8(pc + 1);
                ins.Imm = r.U16(pc + 2);
                break;
            }
            case OpcodesGTA5::LEAVE:
            {
                auto& ins = make(IrOp::LEAVE);
                ins.ArgCount = r.U8(pc + 1);
                ins.RetCount = r.U8(pc + 2);
                break;
            }

            case OpcodesGTA5::ARRAY_U8:
                withImm(IrOp::ARRAY, r.U8(pc + 1));
                break;
            case OpcodesGTA5::ARRAY_U8_LOAD:
                withImm(IrOp::ARRAY_LOAD, r.U8(pc + 1));
                break;
            case OpcodesGTA5::ARRAY_U8_STORE:
                withImm(IrOp::ARRAY_STORE, r.U8(pc + 1));
                break;
            case OpcodesGTA5::ARRAY_U16:
                withImm(IrOp::ARRAY, r.U16(pc + 1));
                break;
            case OpcodesGTA5::ARRAY_U16_LOAD:
                withImm(IrOp::ARRAY_LOAD, r.U16(pc + 1));
                break;
            case OpcodesGTA5::ARRAY_U16_STORE:
                withImm(IrOp::ARRAY_STORE, r.U16(pc + 1));
                break;

            case OpcodesGTA5::LOCAL_U8:
                withImm(IrOp::LOCAL, r.U8(pc + 1));
                break;
            case OpcodesGTA5::LOCAL_U8_LOAD:
                withImm(IrOp::LOCAL_LOAD, r.U8(pc + 1));
                break;
            case OpcodesGTA5::LOCAL_U8_STORE:
                withImm(IrOp::LOCAL_STORE, r.U8(pc + 1));
                break;
            case OpcodesGTA5::LOCAL_U16:
                withImm(IrOp::LOCAL, r.U16(pc + 1));
                break;
            case OpcodesGTA5::LOCAL_U16_LOAD:
                withImm(IrOp::LOCAL_LOAD, r.U16(pc + 1));
                break;
            case OpcodesGTA5::LOCAL_U16_STORE:
                withImm(IrOp::LOCAL_STORE, r.U16(pc + 1));
                break;

            case OpcodesGTA5::STATIC_U8:
                withImm(IrOp::STATIC, r.U8(pc + 1));
                break;
            case OpcodesGTA5::STATIC_U8_LOAD:
                withImm(IrOp::STATIC_LOAD, r.U8(pc + 1));
                break;
            case OpcodesGTA5::STATIC_U8_STORE:
                withImm(IrOp::STATIC_STORE, r.U8(pc + 1));
                break;
            case OpcodesGTA5::STATIC_U16:
                withImm(IrOp::STATIC, r.U16(pc + 1));
                break;
            case OpcodesGTA5::STATIC_U16_LOAD:
                withImm(IrOp::STATIC_LOAD, r.U16(pc + 1));
                break;
            case OpcodesGTA5::STATIC_U16_STORE:
                withImm(IrOp::STATIC_STORE, r.U16(pc + 1));
                break;
            case OpcodesGTA5::STATIC_U24:
                withImm(IrOp::STATIC, r.U24(pc + 1));
                break;
            case OpcodesGTA5::STATIC_U24_LOAD:
                withImm(IrOp::STATIC_LOAD, r.U24(pc + 1));
                break;
            case OpcodesGTA5::STATIC_U24_STORE:
                withImm(IrOp::STATIC_STORE, r.U24(pc + 1));
                break;

            case OpcodesGTA5::GLOBAL_U16:
                withImm(IrOp::GLOBAL, r.U16(pc + 1));
                break;
            case OpcodesGTA5::GLOBAL_U16_LOAD:
                withImm(IrOp::GLOBAL_LOAD, r.U16(pc + 1));
                break;
            case OpcodesGTA5::GLOBAL_U16_STORE:
                withImm(IrOp::GLOBAL_STORE, r.U16(pc + 1));
                break;
            case OpcodesGTA5::GLOBAL_U24:
                withImm(IrOp::GLOBAL, r.U24(pc + 1));
                break;
            case OpcodesGTA5::GLOBAL_U24_LOAD:
                withImm(IrOp::GLOBAL_LOAD, r.U24(pc + 1));
                break;
            case OpcodesGTA5::GLOBAL_U24_STORE:
                withImm(IrOp::GLOBAL_STORE, r.U24(pc + 1));
                break;

            case OpcodesGTA5::IADD_U8:
                withImm(IrOp::IADD_IMM, r.U8(pc + 1));
                break;
            case OpcodesGTA5::IADD_S16:
                withImm(IrOp::IADD_IMM, r.S16(pc + 1));
                break;
            case OpcodesGTA5::IMUL_U8:
                withImm(IrOp::IMUL_IMM, r.U8(pc + 1));
                break;
            case OpcodesGTA5::IMUL_S16:
                withImm(IrOp::IMUL_IMM, r.S16(pc + 1));
                break;

            case OpcodesGTA5::IOFFSET_U8:
                withImm(IrOp::IOFFSET_IMM, r.U8(pc + 1));
                break;
            case OpcodesGTA5::IOFFSET_U8_LOAD:
                withImm(IrOp::IOFFSET_IMM_LOAD, r.U8(pc + 1));
                break;
            case OpcodesGTA5::IOFFSET_U8_STORE:
                withImm(IrOp::IOFFSET_IMM_STORE, r.U8(pc + 1));
                break;
            case OpcodesGTA5::IOFFSET_S16:
                withImm(IrOp::IOFFSET_IMM, r.S16(pc + 1));
                break;
            case OpcodesGTA5::IOFFSET_S16_LOAD:
                withImm(IrOp::IOFFSET_IMM_LOAD, r.S16(pc + 1));
                break;
            case OpcodesGTA5::IOFFSET_S16_STORE:
                withImm(IrOp::IOFFSET_IMM_STORE, r.S16(pc + 1));
                break;

            case OpcodesGTA5::J:
            case OpcodesGTA5::JZ:
            case OpcodesGTA5::IEQ_JZ:
            case OpcodesGTA5::INE_JZ:
            case OpcodesGTA5::IGT_JZ:
            case OpcodesGTA5::IGE_JZ:
            case OpcodesGTA5::ILT_JZ:
            case OpcodesGTA5::ILE_JZ:
            {
                static constexpr IrOp jumps[] = {IrOp::J, IrOp::JZ, IrOp::IEQ_JZ, IrOp::INE_JZ, IrOp::IGT_JZ, IrOp::IGE_JZ, IrOp::ILT_JZ, IrOp::ILE_JZ};
                auto& ins = make(jumps[raw - static_cast<uint8_t>(OpcodesGTA5::J)]);
                ins.Target = static_cast<uint32_t>(static_cast<int64_t>(pc) + 3 + r.S16(pc + 1));
                break;
            }

            case OpcodesGTA5::CALL:
                make(IrOp::CALL).Target = r.U24(pc + 1);
                break;

            case OpcodesGTA5::SWITCH:
            {
                auto& ins = make(IrOp::SWITCH);
                uint8_t count = r.U8(pc + 1);
                ins.Cases.reserve(count);
                for (uint32_t i = 0, entry = pc + 2; i < count; i++, entry += 6)
                    ins.Cases.push_back({static_cast<int32_t>(r.U32(entry)), static_cast<uint32_t>(static_cast<int64_t>(entry) + 6 + r.S16(entry + 4))});
                break;
            }

            case OpcodesGTA5::TEXT_LABEL_ASSIGN_STRING:
                withImm(IrOp::TEXT_LABEL_ASSIGN_STRING, r.U8(pc + 1));
                break;
            case OpcodesGTA5::TEXT_LABEL_ASSIGN_INT:
                withImm(IrOp::TEXT_LABEL_ASSIGN_INT, r.U8(pc + 1));
                break;
            case OpcodesGTA5::TEXT_LABEL_APPEND_STRING:
                withImm(IrOp::TEXT_LABEL_APPEND_STRING, r.U8(pc + 1));
                break;
            case OpcodesGTA5::TEXT_LABEL_APPEND_INT:
                withImm(IrOp::TEXT_LABEL_APPEND_INT, r.U8(pc + 1));
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