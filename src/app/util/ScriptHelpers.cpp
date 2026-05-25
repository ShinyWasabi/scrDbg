#include "ScriptHelpers.hpp"
#include "script/Opcodes.hpp"

namespace scrDbgApp::ScriptHelpers
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
        case OpcodesGTA5::J:
        case OpcodesGTA5::JZ:
        case OpcodesGTA5::IEQ_JZ:
        case OpcodesGTA5::INE_JZ:
        case OpcodesGTA5::IGT_JZ:
        case OpcodesGTA5::IGE_JZ:
        case OpcodesGTA5::ILT_JZ:
        case OpcodesGTA5::ILE_JZ:
            return true;
        }

        return false;
    }

    bool IsWildcardInstruction(uint8_t opcode)
    {
        switch (opcode)
        {
        case OpcodesGTA5::CALL:
        case OpcodesGTA5::J:
        case OpcodesGTA5::JZ:
        case OpcodesGTA5::IEQ_JZ:
        case OpcodesGTA5::INE_JZ:
        case OpcodesGTA5::IGT_JZ:
        case OpcodesGTA5::IGE_JZ:
        case OpcodesGTA5::ILT_JZ:
        case OpcodesGTA5::ILE_JZ:
        case OpcodesGTA5::STATIC_U8:
        case OpcodesGTA5::STATIC_U8_LOAD:
        case OpcodesGTA5::STATIC_U8_STORE:
        case OpcodesGTA5::STATIC_U16:
        case OpcodesGTA5::STATIC_U16_LOAD:
        case OpcodesGTA5::STATIC_U16_STORE:
        case OpcodesGTA5::GLOBAL_U16:
        case OpcodesGTA5::GLOBAL_U16_LOAD:
        case OpcodesGTA5::GLOBAL_U16_STORE:
        case OpcodesGTA5::STATIC_U24:
        case OpcodesGTA5::STATIC_U24_LOAD:
        case OpcodesGTA5::STATIC_U24_STORE:
        case OpcodesGTA5::GLOBAL_U24:
        case OpcodesGTA5::GLOBAL_U24_LOAD:
        case OpcodesGTA5::GLOBAL_U24_STORE:
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
        else if (code[pc] == OpcodesGTA5::CALL || code[pc] == OpcodesGTA5::PUSH_CONST_U24) // check for function pointers
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
        case OpcodesGTA5::NOP:
            return 1;
        case OpcodesGTA5::IADD:
            return 1;
        case OpcodesGTA5::ISUB:
            return 1;
        case OpcodesGTA5::IMUL:
            return 1;
        case OpcodesGTA5::IDIV:
            return 1;
        case OpcodesGTA5::IMOD:
            return 1;
        case OpcodesGTA5::INOT:
            return 1;
        case OpcodesGTA5::INEG:
            return 1;
        case OpcodesGTA5::IEQ:
            return 1;
        case OpcodesGTA5::INE:
            return 1;
        case OpcodesGTA5::IGT:
            return 1;
        case OpcodesGTA5::IGE:
            return 1;
        case OpcodesGTA5::ILT:
            return 1;
        case OpcodesGTA5::ILE:
            return 1;
        case OpcodesGTA5::FADD:
            return 1;
        case OpcodesGTA5::FSUB:
            return 1;
        case OpcodesGTA5::FMUL:
            return 1;
        case OpcodesGTA5::FDIV:
            return 1;
        case OpcodesGTA5::FMOD:
            return 1;
        case OpcodesGTA5::FNEG:
            return 1;
        case OpcodesGTA5::FEQ:
            return 1;
        case OpcodesGTA5::FNE:
            return 1;
        case OpcodesGTA5::FGT:
            return 1;
        case OpcodesGTA5::FGE:
            return 1;
        case OpcodesGTA5::FLT:
            return 1;
        case OpcodesGTA5::FLE:
            return 1;
        case OpcodesGTA5::VADD:
            return 1;
        case OpcodesGTA5::VSUB:
            return 1;
        case OpcodesGTA5::VMUL:
            return 1;
        case OpcodesGTA5::VDIV:
            return 1;
        case OpcodesGTA5::VNEG:
            return 1;
        case OpcodesGTA5::IAND:
            return 1;
        case OpcodesGTA5::IOR:
            return 1;
        case OpcodesGTA5::IXOR:
            return 1;
        case OpcodesGTA5::I2F:
            return 1;
        case OpcodesGTA5::F2I:
            return 1;
        case OpcodesGTA5::F2V:
            return 1;
        case OpcodesGTA5::PUSH_CONST_U8:
            return 2;
        case OpcodesGTA5::PUSH_CONST_U8_U8:
            return 3;
        case OpcodesGTA5::PUSH_CONST_U8_U8_U8:
            return 4;
        case OpcodesGTA5::PUSH_CONST_U32:
            return 5;
        case OpcodesGTA5::PUSH_CONST_F:
            return 5;
        case OpcodesGTA5::DUP:
            return 1;
        case OpcodesGTA5::DROP:
            return 1;
        case OpcodesGTA5::NATIVE:
            return 4;
        case OpcodesGTA5::ENTER:
            return 5 + code[pc + 4];
        case OpcodesGTA5::LEAVE:
            return 3;
        case OpcodesGTA5::LOAD:
            return 1;
        case OpcodesGTA5::STORE:
            return 1;
        case OpcodesGTA5::STORE_REV:
            return 1;
        case OpcodesGTA5::LOAD_N:
            return 1;
        case OpcodesGTA5::STORE_N:
            return 1;
        case OpcodesGTA5::ARRAY_U8:
            return 2;
        case OpcodesGTA5::ARRAY_U8_LOAD:
            return 2;
        case OpcodesGTA5::ARRAY_U8_STORE:
            return 2;
        case OpcodesGTA5::LOCAL_U8:
            return 2;
        case OpcodesGTA5::LOCAL_U8_LOAD:
            return 2;
        case OpcodesGTA5::LOCAL_U8_STORE:
            return 2;
        case OpcodesGTA5::STATIC_U8:
            return 2;
        case OpcodesGTA5::STATIC_U8_LOAD:
            return 2;
        case OpcodesGTA5::STATIC_U8_STORE:
            return 2;
        case OpcodesGTA5::IADD_U8:
            return 2;
        case OpcodesGTA5::IMUL_U8:
            return 2;
        case OpcodesGTA5::IOFFSET:
            return 1;
        case OpcodesGTA5::IOFFSET_U8:
            return 2;
        case OpcodesGTA5::IOFFSET_U8_LOAD:
            return 2;
        case OpcodesGTA5::IOFFSET_U8_STORE:
            return 2;
        case OpcodesGTA5::PUSH_CONST_S16:
            return 3;
        case OpcodesGTA5::IADD_S16:
            return 3;
        case OpcodesGTA5::IMUL_S16:
            return 3;
        case OpcodesGTA5::IOFFSET_S16:
            return 3;
        case OpcodesGTA5::IOFFSET_S16_LOAD:
            return 3;
        case OpcodesGTA5::IOFFSET_S16_STORE:
            return 3;
        case OpcodesGTA5::ARRAY_U16:
            return 3;
        case OpcodesGTA5::ARRAY_U16_LOAD:
            return 3;
        case OpcodesGTA5::ARRAY_U16_STORE:
            return 3;
        case OpcodesGTA5::LOCAL_U16:
            return 3;
        case OpcodesGTA5::LOCAL_U16_LOAD:
            return 3;
        case OpcodesGTA5::LOCAL_U16_STORE:
            return 3;
        case OpcodesGTA5::STATIC_U16:
            return 3;
        case OpcodesGTA5::STATIC_U16_LOAD:
            return 3;
        case OpcodesGTA5::STATIC_U16_STORE:
            return 3;
        case OpcodesGTA5::GLOBAL_U16:
            return 3;
        case OpcodesGTA5::GLOBAL_U16_LOAD:
            return 3;
        case OpcodesGTA5::GLOBAL_U16_STORE:
            return 3;
        case OpcodesGTA5::J:
            return 3;
        case OpcodesGTA5::JZ:
            return 3;
        case OpcodesGTA5::IEQ_JZ:
            return 3;
        case OpcodesGTA5::INE_JZ:
            return 3;
        case OpcodesGTA5::IGT_JZ:
            return 3;
        case OpcodesGTA5::IGE_JZ:
            return 3;
        case OpcodesGTA5::ILT_JZ:
            return 3;
        case OpcodesGTA5::ILE_JZ:
            return 3;
        case OpcodesGTA5::CALL:
            return 4;
        case OpcodesGTA5::STATIC_U24:
            return 4;
        case OpcodesGTA5::STATIC_U24_LOAD:
            return 4;
        case OpcodesGTA5::STATIC_U24_STORE:
            return 4;
        case OpcodesGTA5::GLOBAL_U24:
            return 4;
        case OpcodesGTA5::GLOBAL_U24_LOAD:
            return 4;
        case OpcodesGTA5::GLOBAL_U24_STORE:
            return 4;
        case OpcodesGTA5::PUSH_CONST_U24:
            return 4;
        case OpcodesGTA5::SWITCH:
            return 2 + code[pc + 1] * 6;
        case OpcodesGTA5::STRING:
            return 1;
        case OpcodesGTA5::STRINGHASH:
            return 1;
        case OpcodesGTA5::TEXT_LABEL_ASSIGN_STRING:
            return 2;
        case OpcodesGTA5::TEXT_LABEL_ASSIGN_INT:
            return 2;
        case OpcodesGTA5::TEXT_LABEL_APPEND_STRING:
            return 2;
        case OpcodesGTA5::TEXT_LABEL_APPEND_INT:
            return 2;
        case OpcodesGTA5::TEXT_LABEL_COPY:
            return 1;
        case OpcodesGTA5::CATCH:
            return 1;
        case OpcodesGTA5::THROW:
            return 1;
        case OpcodesGTA5::CALLINDIRECT:
            return 1;
        case OpcodesGTA5::PUSH_CONST_M1:
            return 1;
        case OpcodesGTA5::PUSH_CONST_0:
            return 1;
        case OpcodesGTA5::PUSH_CONST_1:
            return 1;
        case OpcodesGTA5::PUSH_CONST_2:
            return 1;
        case OpcodesGTA5::PUSH_CONST_3:
            return 1;
        case OpcodesGTA5::PUSH_CONST_4:
            return 1;
        case OpcodesGTA5::PUSH_CONST_5:
            return 1;
        case OpcodesGTA5::PUSH_CONST_6:
            return 1;
        case OpcodesGTA5::PUSH_CONST_7:
            return 1;
        case OpcodesGTA5::PUSH_CONST_FM1:
            return 1;
        case OpcodesGTA5::PUSH_CONST_F0:
            return 1;
        case OpcodesGTA5::PUSH_CONST_F1:
            return 1;
        case OpcodesGTA5::PUSH_CONST_F2:
            return 1;
        case OpcodesGTA5::PUSH_CONST_F3:
            return 1;
        case OpcodesGTA5::PUSH_CONST_F4:
            return 1;
        case OpcodesGTA5::PUSH_CONST_F5:
            return 1;
        case OpcodesGTA5::PUSH_CONST_F6:
            return 1;
        case OpcodesGTA5::PUSH_CONST_F7:
            return 1;
        case OpcodesGTA5::IS_BIT_SET:
            return 1;
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

            if (opcode == OpcodesGTA5::NATIVE)
            {
                // Wildcard native index
                for (int j = 0; j < operandSize; j++)
                {
                    if (j >= operandSize - 2)
                        ss << " ?";
                    else
                        ss << ' ' << std::setw(2) << static_cast<int>(code[start + i + 1 + j]);
                }
            }
            else if (IsWildcardInstruction(opcode))
            {
                for (int j = 0; j < operandSize && i + 1 + j < len && (start + i + 1 + j) < code.size(); j++)
                    ss << " ?";
            }
            else
            {
                for (int j = 0; j < operandSize && i + 1 + j < len && (start + i + 1 + j) < code.size(); j++)
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
                    if (a == OpcodesGTA5::NATIVE && k > 1)
                        continue;
                    else if (IsWildcardInstruction(a) && k > 0)
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