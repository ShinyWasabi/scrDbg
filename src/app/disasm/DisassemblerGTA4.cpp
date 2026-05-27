#include "DisassemblerGTA4.hpp"

namespace scrDbgApp
{
    int DisassemblerGTA4::GetInstructionSize(uint32_t pc) const
    {
        Opcodes op = static_cast<Opcodes>(GetU8(pc));

        switch (op)
        {
        case Opcodes::J:
        case Opcodes::JZ:
        case Opcodes::JNZ:
        case Opcodes::PUSH_CONST_U32:
        case Opcodes::PUSH_CONST_F:
        case Opcodes::CALL:
            return 5;
        case Opcodes::PUSH_CONST_S16:
            return 3;
        case Opcodes::NATIVE:
            return 7;
        case Opcodes::ENTER:
            return 4;
        case Opcodes::LEAVE:
            return 3;
        case Opcodes::TEXT_LABEL_ASSIGN_STRING:
        case Opcodes::TEXT_LABEL_ASSIGN_INT:
        case Opcodes::TEXT_LABEL_APPEND_STRING:
        case Opcodes::TEXT_LABEL_APPEND_INT:
            return 2;
        case Opcodes::STRING:
            return 2 + GetU8(pc + 1);
        case Opcodes::SWITCH:
            return 2 + GetU8(pc + 1) * 8;
        }

        return 1;
    }

    bool DisassemblerGTA4::IsJumpOrCall(uint8_t op) const
    {
        switch (static_cast<Opcodes>(op))
        {
        case Opcodes::J:
        case Opcodes::JZ:
        case Opcodes::JNZ:
        case Opcodes::CALL:
            return true;
        default:
            return false;
        }
    }

    uint32_t DisassemblerGTA4::GetJumpTarget(uint32_t pc) const
    {
        return GetU32(pc + 1);
    }

    // Might as well just return false since GTA IV scripts aren't likely to be updated anymore
    bool DisassemblerGTA4::IsWildcard(uint8_t op) const
    {
        switch (static_cast<Opcodes>(op))
        {
        case Opcodes::J:
        case Opcodes::JZ:
        case Opcodes::JNZ:
        case Opcodes::CALL:
        case Opcodes::NATIVE:
            return true;
        }

        return false;
    }

    bool DisassemblerGTA4::IsXrefToPc(uint32_t pc, uint32_t targetPc) const
    {
        uint8_t op = GetU8(pc);

        if (IsJumpOrCall(op))
        {
            if (GetU32(pc + 1) == targetPc)
                return true;
        }

        return false;
    }

    std::string DisassemblerGTA4::MakePattern(uint32_t start, int len) const
    {
        std::ostringstream ss;
        ss << std::uppercase << std::hex << std::setfill('0');

        int i = 0;
        bool first = true;

        while (i < len && (start + i) < static_cast<int>(m_Code.size()))
        {
            if (!first)
                ss << ' ';
            first = false;

            uint8_t opcode = m_Code[start + i];
            ss << std::setw(2) << static_cast<int>(opcode);

            int instrSize = GetInstructionSize(start + i);
            int operandSize = instrSize - 1;

            if (static_cast<Opcodes>(opcode) == Opcodes::NATIVE)
            {
                for (int j = 0; j < operandSize; j++)
                {
                    if (j >= operandSize - 4)
                        ss << " ?";
                    else
                        ss << ' ' << std::setw(2) << static_cast<int>(m_Code[start + i + 1 + j]);
                }
            }
            else if (IsWildcard(opcode))
            {
                for (int j = 0; j < operandSize && (i + 1 + j) < len && (start + i + 1 + j) < static_cast<int>(m_Code.size()); j++)
                    ss << " ?";
            }
            else
            {
                for (int j = 0; j < operandSize && (i + 1 + j) < len && (start + i + 1 + j) < static_cast<int>(m_Code.size()); j++)
                    ss << ' ' << std::setw(2) << static_cast<int>(m_Code[start + i + 1 + j]);
            }

            i += instrSize;
        }

        return ss.str();
    }

    bool DisassemblerGTA4::IsPatternUnique(uint32_t pc, int patternLength) const
    {
        int count = 0;

        for (int i = 0; i + patternLength <= static_cast<int>(m_Code.size()); ++i)
        {
            bool match = true;
            int j = 0;

            while (j < patternLength)
            {
                uint8_t a = m_Code[pc + j];

                int instrSize = GetInstructionSize(pc + j);

                for (int k = 0; k < instrSize && (j + k) < patternLength && (pc + j + k) < m_Code.size() && (i + j + k) < m_Code.size(); ++k)
                {
                    if (static_cast<Opcodes>(a) == Opcodes::NATIVE && k > 2)
                        continue;

                    else if (IsWildcard(a) && k > 0)
                        continue;

                    if (m_Code[pc + j + k] != m_Code[i + j + k])
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

    const char* DisassemblerGTA4::GetInstructionDescription(uint8_t opcode) const
    {
        if (opcode >= m_InstructionTable.size())
            return "???";

        return m_InstructionTable[opcode].Description;
    }

    std::string DisassemblerGTA4::DecodeInstructionInternal(const InstructionInfo& insnInfo) const
    {
        std::string result = "???";

        uint8_t op = GetU8(insnInfo.Pc);
        if (op >= m_InstructionTable.size())
            return "???";

        const auto& insn = m_InstructionTable[op];

        std::ostringstream instr;
        instr << insn.Name << " ";

        uint32_t offset = insnInfo.Pc + 1;

        const char* fmt = insn.OperandFmt;
        while (*fmt)
        {
            switch (*fmt++)
            {
            case 'a': // U8
                instr << std::dec << static_cast<int>(GetU8(offset++));
                break;
            case 'b': // U16
                instr << std::dec << GetU16(offset);
                offset += 2;
                break;
            case 'c': // S16
                instr << std::dec << GetS16(offset);
                offset += 2;
                break;
            case 'e': // U32
            {
                uint32_t val = GetU32(offset);
                if (IsJumpOrCall(op) || op == Opcodes::NATIVE)
                    instr << "0x" << std::uppercase << std::hex << val;
                else
                    instr << std::dec << val;
                offset += 4;
                break;
            }
            case 'f': // FLOAT
                instr << GetF32(offset);
                offset += 4;
                break;
            case 'h': // NATIVE
            {
                uint32_t argCount = GetU8(offset++);
                uint32_t retCount = GetU8(offset++);
                uint32_t handler = GetU32(offset);
                offset += 4;

                uint32_t hash = static_cast<uint32_t>(g_Game->GetNativeHashByHandler(handler));

                instr << argCount << ", " << retCount;
                if (handler && hash != 0)
                {
                    std::ostringstream nativeStr;

                    auto name = g_Game->GetNativeNameByHash(hash);
                    nativeStr << " // " << (name.empty() ? "UNKNOWN_NATIVE" : name);

                    nativeStr << ", 0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << hash;
                    nativeStr << ", " << Process::GetName() << "+0x" << handler - Process::GetBaseAddress();

                    instr << nativeStr.str();
                }

                break;
            }
            case 'i': // SWITCH
            {
                uint8_t count = GetU8(offset++);
                instr << "[" << std::dec << (int)count << "]";
                for (int i = 0; i < count; i++)
                {
                    int32_t key = GetU32(offset);
                    uint32_t target = GetU32(offset + 4);
                    instr << " " << key << "=0x" << std::hex << target;
                    offset += 8;
                }
                break;
            }
            case 's': // STRING
            {
                uint8_t len = GetU8(offset++);
                std::string str(reinterpret_cast<const char*>(&m_Code[offset]), len);

                if (!str.empty() && str.back() == '\0')
                    str.pop_back();

                instr << "\"" << str << "\"";
                offset += len;
                break;
            }
            }

            if (*fmt)
                instr << ", ";
        }

        result = instr.str();
        return result;
    }
}