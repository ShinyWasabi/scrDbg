#include "ScriptHelpers.hpp"
#include "game/rage/scrOpcode.hpp"

namespace scrDbg::ScriptHelpers
{
    uint8_t ReadByte(const std::vector<uint8_t>& code, uint32_t pc)
    {
        return (pc < code.size()) ? code[pc] : 0xFF;
    }

    uint16_t ReadU16(const std::vector<uint8_t>& code, uint32_t pc)
    {
        return static_cast<uint16_t>(ReadByte(code, pc) | (ReadByte(code, pc + 1) << 8));
    }

    int16_t ReadS16(const std::vector<uint8_t>& code, uint32_t pc)
    {
        return static_cast<int16_t>(ReadU16(code, pc));
    }

    uint32_t ReadU24(const std::vector<uint8_t>& code, uint32_t pc)
    {
        return static_cast<uint32_t>(ReadByte(code, pc) | (ReadByte(code, pc + 1) << 8) | (ReadByte(code, pc + 2) << 16));
    }

    uint32_t ReadU32(const std::vector<uint8_t>& code, uint32_t pc)
    {
        return static_cast<uint32_t>(ReadByte(code, pc) | (ReadByte(code, pc + 1) << 8) | (ReadByte(code, pc + 2) << 16) | (ReadByte(code, pc + 3) << 24));
    }

    float ReadF32(const std::vector<uint8_t>& code, uint32_t pc)
    {
        float f;
        uint32_t val = ReadU32(code, pc);
        std::memcpy(&f, &val, sizeof(float));
        return f;
    }

    bool IsJumpInstruction(uint8_t opcode)
    {
        switch (opcode)
        {
        case rage::scrOpcode::J:
        case rage::scrOpcode::JZ:
        case rage::scrOpcode::IEQ_JZ:
        case rage::scrOpcode::INE_JZ:
        case rage::scrOpcode::IGT_JZ:
        case rage::scrOpcode::IGE_JZ:
        case rage::scrOpcode::ILT_JZ:
        case rage::scrOpcode::ILE_JZ:
            return true;
        }

        return false;
    }

    bool IsWildcardInstruction(uint8_t opcode)
    {
        switch (opcode)
        {
        case rage::scrOpcode::CALL:
        case rage::scrOpcode::J:
        case rage::scrOpcode::JZ:
        case rage::scrOpcode::IEQ_JZ:
        case rage::scrOpcode::INE_JZ:
        case rage::scrOpcode::IGT_JZ:
        case rage::scrOpcode::IGE_JZ:
        case rage::scrOpcode::ILT_JZ:
        case rage::scrOpcode::ILE_JZ:
        case rage::scrOpcode::STATIC_U8:
        case rage::scrOpcode::STATIC_U8_LOAD:
        case rage::scrOpcode::STATIC_U8_STORE:
        case rage::scrOpcode::STATIC_U16:
        case rage::scrOpcode::STATIC_U16_LOAD:
        case rage::scrOpcode::STATIC_U16_STORE:
        case rage::scrOpcode::GLOBAL_U16:
        case rage::scrOpcode::GLOBAL_U16_LOAD:
        case rage::scrOpcode::GLOBAL_U16_STORE:
        case rage::scrOpcode::STATIC_U24:
        case rage::scrOpcode::STATIC_U24_LOAD:
        case rage::scrOpcode::STATIC_U24_STORE:
        case rage::scrOpcode::GLOBAL_U24:
        case rage::scrOpcode::GLOBAL_U24_LOAD:
        case rage::scrOpcode::GLOBAL_U24_STORE:
            return true;
        }

        return false;
    }

    bool IsXrefToPc(const std::vector<uint8_t>& code, uint32_t pc, uint32_t targetPc)
    {
        if (IsJumpInstruction(code[pc]))
        {
            if ((pc + 2 + ScriptHelpers::ReadS16(code, pc + 1) + 1) == targetPc)
                return true;
        }
        else if (code[pc] == rage::scrOpcode::CALL || code[pc] == rage::scrOpcode::PUSH_CONST_U24) // check for function pointers
        {
            if (ScriptHelpers::ReadU24(code, pc + 1) == targetPc)
                return true;
        }

        return false;
    }

    int GetInstructionSize(const std::vector<uint8_t>& code, uint32_t pc)
    {
        uint8_t opcode = code[pc];

        switch (opcode)
        {
        case rage::scrOpcode::NOP: return 1;
        case rage::scrOpcode::IADD: return 1;
        case rage::scrOpcode::ISUB: return 1;
        case rage::scrOpcode::IMUL: return 1;
        case rage::scrOpcode::IDIV: return 1;
        case rage::scrOpcode::IMOD: return 1;
        case rage::scrOpcode::INOT: return 1;
        case rage::scrOpcode::INEG: return 1;
        case rage::scrOpcode::IEQ: return 1;
        case rage::scrOpcode::INE: return 1;
        case rage::scrOpcode::IGT: return 1;
        case rage::scrOpcode::IGE: return 1;
        case rage::scrOpcode::ILT: return 1;
        case rage::scrOpcode::ILE: return 1;
        case rage::scrOpcode::FADD: return 1;
        case rage::scrOpcode::FSUB: return 1;
        case rage::scrOpcode::FMUL: return 1;
        case rage::scrOpcode::FDIV: return 1;
        case rage::scrOpcode::FMOD: return 1;
        case rage::scrOpcode::FNEG: return 1;
        case rage::scrOpcode::FEQ: return 1;
        case rage::scrOpcode::FNE: return 1;
        case rage::scrOpcode::FGT: return 1;
        case rage::scrOpcode::FGE: return 1;
        case rage::scrOpcode::FLT: return 1;
        case rage::scrOpcode::FLE: return 1;
        case rage::scrOpcode::VADD: return 1;
        case rage::scrOpcode::VSUB: return 1;
        case rage::scrOpcode::VMUL: return 1;
        case rage::scrOpcode::VDIV: return 1;
        case rage::scrOpcode::VNEG: return 1;
        case rage::scrOpcode::IAND: return 1;
        case rage::scrOpcode::IOR: return 1;
        case rage::scrOpcode::IXOR: return 1;
        case rage::scrOpcode::I2F: return 1;
        case rage::scrOpcode::F2I: return 1;
        case rage::scrOpcode::F2V: return 1;
        case rage::scrOpcode::PUSH_CONST_U8: return 2;
        case rage::scrOpcode::PUSH_CONST_U8_U8: return 2;
        case rage::scrOpcode::PUSH_CONST_U8_U8_U8: return 3;
        case rage::scrOpcode::PUSH_CONST_U32: return 5;
        case rage::scrOpcode::PUSH_CONST_F: return 5;
        case rage::scrOpcode::DUP: return 1;
        case rage::scrOpcode::DROP: return 1;
        case rage::scrOpcode::NATIVE: return 4;
        case rage::scrOpcode::ENTER: return 5 + code[pc + 4];
        case rage::scrOpcode::LEAVE: return 3;
        case rage::scrOpcode::LOAD: return 1;
        case rage::scrOpcode::STORE: return 1;
        case rage::scrOpcode::STORE_REV: return 1;
        case rage::scrOpcode::LOAD_N: return 1;
        case rage::scrOpcode::STORE_N: return 1;
        case rage::scrOpcode::ARRAY_U8: return 2;
        case rage::scrOpcode::ARRAY_U8_LOAD: return 2;
        case rage::scrOpcode::ARRAY_U8_STORE: return 2;
        case rage::scrOpcode::LOCAL_U8: return 2;
        case rage::scrOpcode::LOCAL_U8_LOAD: return 2;
        case rage::scrOpcode::LOCAL_U8_STORE: return 2;
        case rage::scrOpcode::STATIC_U8: return 2;
        case rage::scrOpcode::STATIC_U8_LOAD: return 2;
        case rage::scrOpcode::STATIC_U8_STORE: return 2;
        case rage::scrOpcode::IADD_U8: return 2;
        case rage::scrOpcode::IMUL_U8: return 2;
        case rage::scrOpcode::IOFFSET: return 1;
        case rage::scrOpcode::IOFFSET_U8: return 2;
        case rage::scrOpcode::IOFFSET_U8_LOAD: return 2;
        case rage::scrOpcode::IOFFSET_U8_STORE: return 2;
        case rage::scrOpcode::PUSH_CONST_S16: return 3;
        case rage::scrOpcode::IADD_S16: return 3;
        case rage::scrOpcode::IMUL_S16: return 3;
        case rage::scrOpcode::IOFFSET_S16: return 3;
        case rage::scrOpcode::IOFFSET_S16_LOAD: return 3;
        case rage::scrOpcode::IOFFSET_S16_STORE: return 3;
        case rage::scrOpcode::ARRAY_U16: return 3;
        case rage::scrOpcode::ARRAY_U16_LOAD: return 3;
        case rage::scrOpcode::ARRAY_U16_STORE: return 3;
        case rage::scrOpcode::LOCAL_U16: return 3;
        case rage::scrOpcode::LOCAL_U16_LOAD: return 3;
        case rage::scrOpcode::LOCAL_U16_STORE: return 3;
        case rage::scrOpcode::STATIC_U16: return 3;
        case rage::scrOpcode::STATIC_U16_LOAD: return 3;
        case rage::scrOpcode::STATIC_U16_STORE: return 3;
        case rage::scrOpcode::GLOBAL_U16: return 3;
        case rage::scrOpcode::GLOBAL_U16_LOAD: return 3;
        case rage::scrOpcode::GLOBAL_U16_STORE: return 3;
        case rage::scrOpcode::J: return 3;
        case rage::scrOpcode::JZ: return 3;
        case rage::scrOpcode::IEQ_JZ: return 3;
        case rage::scrOpcode::INE_JZ: return 3;
        case rage::scrOpcode::IGT_JZ: return 3;
        case rage::scrOpcode::IGE_JZ: return 3;
        case rage::scrOpcode::ILT_JZ: return 3;
        case rage::scrOpcode::ILE_JZ: return 3;
        case rage::scrOpcode::CALL: return 4;
        case rage::scrOpcode::STATIC_U24: return 4;
        case rage::scrOpcode::STATIC_U24_LOAD: return 4;
        case rage::scrOpcode::STATIC_U24_STORE: return 4;
        case rage::scrOpcode::GLOBAL_U24: return 4;
        case rage::scrOpcode::GLOBAL_U24_LOAD: return 4;
        case rage::scrOpcode::GLOBAL_U24_STORE: return 4;
        case rage::scrOpcode::PUSH_CONST_U24: return 4;
        case rage::scrOpcode::SWITCH: return 2 + code[pc + 1] * 6;
        case rage::scrOpcode::STRING: return 1;
        case rage::scrOpcode::STRINGHASH: return 1;
        case rage::scrOpcode::TEXT_LABEL_ASSIGN_STRING: return 2;
        case rage::scrOpcode::TEXT_LABEL_ASSIGN_INT: return 2;
        case rage::scrOpcode::TEXT_LABEL_APPEND_STRING: return 2;
        case rage::scrOpcode::TEXT_LABEL_APPEND_INT: return 2;
        case rage::scrOpcode::TEXT_LABEL_COPY: return 1;
        case rage::scrOpcode::CATCH: return 1;
        case rage::scrOpcode::THROW: return 1;
        case rage::scrOpcode::CALLINDIRECT: return 1;
        case rage::scrOpcode::PUSH_CONST_M1: return 1;
        case rage::scrOpcode::PUSH_CONST_0: return 1;
        case rage::scrOpcode::PUSH_CONST_1: return 1;
        case rage::scrOpcode::PUSH_CONST_2: return 1;
        case rage::scrOpcode::PUSH_CONST_3: return 1;
        case rage::scrOpcode::PUSH_CONST_4: return 1;
        case rage::scrOpcode::PUSH_CONST_5: return 1;
        case rage::scrOpcode::PUSH_CONST_6: return 1;
        case rage::scrOpcode::PUSH_CONST_7: return 1;
        case rage::scrOpcode::PUSH_CONST_FM1: return 1;
        case rage::scrOpcode::PUSH_CONST_F0: return 1;
        case rage::scrOpcode::PUSH_CONST_F1: return 1;
        case rage::scrOpcode::PUSH_CONST_F2: return 1;
        case rage::scrOpcode::PUSH_CONST_F3: return 1;
        case rage::scrOpcode::PUSH_CONST_F4: return 1;
        case rage::scrOpcode::PUSH_CONST_F5: return 1;
        case rage::scrOpcode::PUSH_CONST_F6: return 1;
        case rage::scrOpcode::PUSH_CONST_F7: return 1;
        case rage::scrOpcode::IS_BIT_SET: return 1;
        }

        return 1;
    }

    std::string MakePattern(const std::vector<uint8_t>& code, uint32_t start, int len)
    {
        std::ostringstream ss;
        ss << std::uppercase << std::hex << std::setfill('0');

        int i = 0;
        bool first = true;

        while (i < len && (start + i) < static_cast<int>(code.size()))
        {
            if (!first)
                ss << ' ';
            first = false;

            uint8_t opcode = code[start + i];
            ss << std::setw(2) << static_cast<int>(opcode);

            int instrSize = GetInstructionSize(code, start + i);
            int operandSize = instrSize - 1;

            if (IsWildcardInstruction(opcode))
            {
                for (int j = 0; j < operandSize && i + 1 + j < len && (start + i + 1 + j) < code.size(); ++j)
                    ss << " ?";
            }
            else
            {
                for (int j = 0; j < operandSize && i + 1 + j < len && (start + i + 1 + j) < code.size(); ++j)
                    ss << ' ' << std::setw(2) << static_cast<int>(code[start + i + 1 + j]);
            }

            i += instrSize;
        }

        return ss.str();
    }

    bool IsPatternUnique(const std::vector<uint8_t>& code, uint32_t pc, int patternLength)
    {
        int count = 0;

        for (int i = 0; i + patternLength <= static_cast<int>(code.size()); ++i)
        {
            bool match = true;
            int j = 0;

            while (j < patternLength)
            {
                uint8_t a = code[pc + j];
                uint8_t b = code[i + j];

                int instrSize = GetInstructionSize(code, pc + j);
                int operandSize = instrSize - 1;

                for (int k = 0; k < instrSize && j + k < patternLength && (pc + j + k) < code.size() && (i + j + k) < code.size(); ++k)
                {
                    if (IsWildcardInstruction(a) && k > 0)
                        continue;

                    if (code[pc + j + k] != code[i + j + k])
                    {
                        match = false;
                        break;
                    }
                }

                if (!match)
                    break;

                j += instrSize;
            }

            if (match && ++count > 1)
                return false;
        }

        return count == 1;
    }

    std::vector<uint32_t> ScanPattern(const std::vector<uint8_t>& code, const std::vector<std::optional<uint8_t>>& pattern)
    {
        std::vector<uint32_t> results;

        int codeSize = static_cast<int>(code.size());
        int patSize = static_cast<int>(pattern.size());
        if (patSize == 0 || codeSize < patSize)
            return results;

        for (int i = 0; i + patSize <= codeSize; ++i)
        {
            bool match = true;
            for (int j = 0; j < patSize; ++j)
            {
                if (pattern[j] && *pattern[j] != code[i + j])
                {
                    match = false;
                    break;
                }
            }

            if (match)
                results.push_back(i);
        }

        return results;
    }
}