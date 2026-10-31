#include "DecompilerGTA4.hpp"
#include "disasm/opcodes/OpcodesGTA4.hpp"
#include "ir/Function.hpp"

namespace scrDbgApp
{
    using namespace Decomp;

    static std::optional<IrOp> MapSimple(OpcodesGTA4 op) // ops whose GTA 4 encoding maps 1:1 onto an operand less IR op
    {
        switch (op)
        {
        case OpcodesGTA4::NOP:
            return IrOp::NOP;
        case OpcodesGTA4::IADD:
            return IrOp::IADD;
        case OpcodesGTA4::ISUB:
            return IrOp::ISUB;
        case OpcodesGTA4::IMUL:
            return IrOp::IMUL;
        case OpcodesGTA4::IDIV:
            return IrOp::IDIV;
        case OpcodesGTA4::IMOD:
            return IrOp::IMOD;
        case OpcodesGTA4::INOT:
            return IrOp::INOT;
        case OpcodesGTA4::INEG:
            return IrOp::INEG;
        case OpcodesGTA4::IEQ:
            return IrOp::IEQ;
        case OpcodesGTA4::INE:
            return IrOp::INE;
        case OpcodesGTA4::IGT:
            return IrOp::IGT;
        case OpcodesGTA4::IGE:
            return IrOp::IGE;
        case OpcodesGTA4::ILT:
            return IrOp::ILT;
        case OpcodesGTA4::ILE:
            return IrOp::ILE;
        case OpcodesGTA4::FADD:
            return IrOp::FADD;
        case OpcodesGTA4::FSUB:
            return IrOp::FSUB;
        case OpcodesGTA4::FMUL:
            return IrOp::FMUL;
        case OpcodesGTA4::FDIV:
            return IrOp::FDIV;
        case OpcodesGTA4::FMOD:
            return IrOp::FMOD;
        case OpcodesGTA4::FNEG:
            return IrOp::FNEG;
        case OpcodesGTA4::FEQ:
            return IrOp::FEQ;
        case OpcodesGTA4::FNE:
            return IrOp::FNE;
        case OpcodesGTA4::FGT:
            return IrOp::FGT;
        case OpcodesGTA4::FGE:
            return IrOp::FGE;
        case OpcodesGTA4::FLT:
            return IrOp::FLT;
        case OpcodesGTA4::FLE:
            return IrOp::FLE;
        case OpcodesGTA4::VADD:
            return IrOp::VADD;
        case OpcodesGTA4::VSUB:
            return IrOp::VSUB;
        case OpcodesGTA4::VMUL:
            return IrOp::VMUL;
        case OpcodesGTA4::VDIV:
            return IrOp::VDIV;
        case OpcodesGTA4::VNEG:
            return IrOp::VNEG;
        case OpcodesGTA4::IAND:
            return IrOp::IAND;
        case OpcodesGTA4::IOR:
            return IrOp::IOR;
        case OpcodesGTA4::IXOR:
            return IrOp::IXOR;
        case OpcodesGTA4::I2F:
            return IrOp::I2F;
        case OpcodesGTA4::F2I:
            return IrOp::F2I;
        case OpcodesGTA4::F2V:
            return IrOp::F2V;
        case OpcodesGTA4::DUP:
            return IrOp::DUP;
        case OpcodesGTA4::DROP:
            return IrOp::DROP;
        case OpcodesGTA4::LOAD:
            return IrOp::LOAD;
        case OpcodesGTA4::STORE:
            return IrOp::STORE;
        case OpcodesGTA4::STORE_REV:
            return IrOp::STORE_REV;
        case OpcodesGTA4::TEXT_LABEL_COPY:
            return IrOp::TEXT_LABEL_COPY;
        case OpcodesGTA4::CATCH:
            return IrOp::CATCH;
        case OpcodesGTA4::THROW:
            return IrOp::THROW;
        case OpcodesGTA4::LOAD_N:
            return IrOp::LOAD_N;
        case OpcodesGTA4::STORE_N:
            return IrOp::STORE_N;
        case OpcodesGTA4::GETXPROTECT:
            return IrOp::GETXPROTECT;
        case OpcodesGTA4::SETXPROTECT:
            return IrOp::SETXPROTECT;
        case OpcodesGTA4::REFXPROTECT:
            return IrOp::REFXPROTECT;
        // same stack effect as a different IR op
        case OpcodesGTA4::_NULL:
            return IrOp::PUSH_NULL;
        case OpcodesGTA4::LOCAL:
            return IrOp::LOCAL_STACK;
        case OpcodesGTA4::STATIC:
            return IrOp::STATIC_STACK;
        case OpcodesGTA4::GLOBAL:
            return IrOp::GLOBAL_STACK;
        case OpcodesGTA4::ARRAY:
            return IrOp::ARRAY_STACK;
        default:
            return std::nullopt;
        }
    }

    std::vector<Instruction> DecompilerGTA4::Lift(const Function& func) const
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
            const auto op = static_cast<OpcodesGTA4>(raw);

            if ((op == OpcodesGTA4::STRING || op == OpcodesGTA4::SWITCH) && pc + 2 > codeSize)
            {
                result.emplace_back(IrOp::UNKNOWN, raw, pc, codeSize - pc);
                break;
            }

            const uint32_t size = static_cast<uint32_t>(GetInsnSizeGTA4(code.data(), pc));
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

            if (raw >= static_cast<uint8_t>(OpcodesGTA4::PUSH_CONST_M16)) // PUSH_CONST_M16..PUSH_CONST_159, the value is op - 96
            {
                make(IrOp::PUSH_INT).Ints.assign({static_cast<int32_t>(raw) - static_cast<int32_t>(OpcodesGTA4::PUSH_CONST_0)});
                continue;
            }

            switch (op)
            {
            case OpcodesGTA4::PUSH_CONST_S16:
                make(IrOp::PUSH_INT).Ints.assign({r.GetS16(pc + 1)});
                break;
            case OpcodesGTA4::PUSH_CONST_U32:
                make(IrOp::PUSH_INT).Ints.assign({static_cast<int32_t>(r.GetU32(pc + 1))});
                break;
            case OpcodesGTA4::PUSH_CONST_F:
                make(IrOp::PUSH_FLOAT).Float = r.GetF32(pc + 1);
                break;

            case OpcodesGTA4::NATIVE:
            {
                auto& ins = make(IrOp::NATIVE);
                ins.ArgCount = r.GetU8(pc + 1);
                ins.RetCount = r.GetU8(pc + 2);
                ins.Imm = r.GetU32(pc + 3); // handler address, there's no native table
                break;
            }

            case OpcodesGTA4::ENTER:
            {
                auto& ins = make(IrOp::ENTER);
                ins.ArgCount = r.GetU8(pc + 1);
                ins.Imm = r.GetU16(pc + 2);
                break;
            }
            case OpcodesGTA4::LEAVE:
            {
                auto& ins = make(IrOp::LEAVE);
                ins.ArgCount = r.GetU8(pc + 1);
                ins.RetCount = r.GetU8(pc + 2);
                break;
            }

            case OpcodesGTA4::LOCAL_0:
            case OpcodesGTA4::LOCAL_1:
            case OpcodesGTA4::LOCAL_2:
            case OpcodesGTA4::LOCAL_3:
            case OpcodesGTA4::LOCAL_4:
            case OpcodesGTA4::LOCAL_5:
            case OpcodesGTA4::LOCAL_6:
            case OpcodesGTA4::LOCAL_7:
                make(IrOp::LOCAL).Imm = raw - static_cast<uint8_t>(OpcodesGTA4::LOCAL_0);
                break;

            // absolute targets
            case OpcodesGTA4::J:
                make(IrOp::J).Target = r.GetU32(pc + 1);
                break;
            case OpcodesGTA4::JZ:
                make(IrOp::JZ).Target = r.GetU32(pc + 1);
                break;
            case OpcodesGTA4::JNZ:
                make(IrOp::JNZ).Target = r.GetU32(pc + 1);
                break;
            case OpcodesGTA4::CALL:
                make(IrOp::CALL).Target = r.GetU32(pc + 1);
                break;

            case OpcodesGTA4::SWITCH:
            {
                // count, then { i32 value, u32 absolute target }
                auto& ins = make(IrOp::SWITCH);
                uint8_t count = r.GetU8(pc + 1);
                ins.Cases.reserve(count);
                for (uint32_t i = 0, entry = pc + 2; i < count; i++, entry += 8)
                    ins.Cases.push_back({static_cast<int32_t>(r.GetU32(entry)), r.GetU32(entry + 4)});
                break;
            }

            case OpcodesGTA4::STRING:
            {
                // length, then the characters inline
                auto& ins = make(IrOp::PUSH_STRING);
                uint8_t length = r.GetU8(pc + 1);
                for (uint32_t i = 0; i < length && r.GetU8(pc + 2 + i); i++)
                    ins.Text += static_cast<char>(r.GetU8(pc + 2 + i));
                break;
            }

            case OpcodesGTA4::TEXT_LABEL_ASSIGN_STRING:
                make(IrOp::TEXT_LABEL_ASSIGN_STRING).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesGTA4::TEXT_LABEL_ASSIGN_INT:
                make(IrOp::TEXT_LABEL_ASSIGN_INT).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesGTA4::TEXT_LABEL_APPEND_STRING:
                make(IrOp::TEXT_LABEL_APPEND_STRING).Imm = r.GetU8(pc + 1);
                break;
            case OpcodesGTA4::TEXT_LABEL_APPEND_INT:
                make(IrOp::TEXT_LABEL_APPEND_INT).Imm = r.GetU8(pc + 1);
                break;

            default: // EXIT, the only invalid opcode
                make(IrOp::UNKNOWN);
                break;
            }
        }

        return result;
    }

    uint64_t DecompilerGTA4::GetNativeHash(uint32_t index) const
    {
        return g_Game->GetNativeHashByHandler(index);
    }
}