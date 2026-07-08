#include "DisassemblerRDR2.hpp"
#include "opcodes/OpcodesRDR2.hpp"

namespace scrDbgApp
{
    std::string DisassemblerRDR2::GetFunctionName(uint32_t pc, uint32_t size) const
    {
        if (size > 0)
        {
            std::string name(reinterpret_cast<const char*>(&m_Code[pc + 7])); // 7 instead of 5 to skip the profiler placeholders
            if (!name.empty())
                return name;
        }

        std::ostringstream nameStr;
        nameStr << "sub_" << std::uppercase << std::hex << pc;
        return nameStr.str();
    }

    int DisassemblerRDR2::GetInstructionSize(uint32_t pc) const
    {
        OpcodesRDR2 op = static_cast<OpcodesRDR2>(GetU8(pc));

        switch (op)
        {
        case OpcodesRDR2::PUSH_CONST_U8:
        case OpcodesRDR2::ARRAY_U8:
        case OpcodesRDR2::ARRAY_U8_LOAD:
        case OpcodesRDR2::ARRAY_U8_STORE:
        case OpcodesRDR2::LOCAL_U8:
        case OpcodesRDR2::LOCAL_U8_LOAD:
        case OpcodesRDR2::LOCAL_U8_STORE:
        case OpcodesRDR2::STATIC_U8:
        case OpcodesRDR2::STATIC_U8_LOAD:
        case OpcodesRDR2::STATIC_U8_STORE:
        case OpcodesRDR2::IADD_U8:
        case OpcodesRDR2::IOFFSET_U8_LOAD:
        case OpcodesRDR2::IOFFSET_U8_STORE:
        case OpcodesRDR2::IMUL_U8:
        case OpcodesRDR2::TEXT_LABEL_ASSIGN_STRING:
        case OpcodesRDR2::TEXT_LABEL_ASSIGN_INT:
        case OpcodesRDR2::TEXT_LABEL_APPEND_STRING:
        case OpcodesRDR2::TEXT_LABEL_APPEND_INT:
            return 2;
        case OpcodesRDR2::PUSH_CONST_U8_U8:
        case OpcodesRDR2::NATIVE:
        case OpcodesRDR2::LEAVE:
        case OpcodesRDR2::PUSH_CONST_S16:
        case OpcodesRDR2::IADD_S16:
        case OpcodesRDR2::IOFFSET_S16_LOAD:
        case OpcodesRDR2::IOFFSET_S16_STORE:
        case OpcodesRDR2::IMUL_S16:
        case OpcodesRDR2::ARRAY_U16:
        case OpcodesRDR2::ARRAY_U16_LOAD:
        case OpcodesRDR2::ARRAY_U16_STORE:
        case OpcodesRDR2::LOCAL_U16:
        case OpcodesRDR2::LOCAL_U16_LOAD:
        case OpcodesRDR2::LOCAL_U16_STORE:
        case OpcodesRDR2::STATIC_U16:
        case OpcodesRDR2::STATIC_U16_LOAD:
        case OpcodesRDR2::STATIC_U16_STORE:
        case OpcodesRDR2::GLOBAL_U16:
        case OpcodesRDR2::GLOBAL_U16_LOAD:
        case OpcodesRDR2::GLOBAL_U16_STORE:
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
        case OpcodesRDR2::J:
        case OpcodesRDR2::JZ:
        case OpcodesRDR2::INE_J:
        case OpcodesRDR2::IEQ_J:
        case OpcodesRDR2::ILE_J:
        case OpcodesRDR2::ILT_J:
        case OpcodesRDR2::IGE_J:
        case OpcodesRDR2::IGT_J:
            return 3;
        case OpcodesRDR2::PUSH_CONST_U8_U8_U8:
        case OpcodesRDR2::GLOBAL_U24:
        case OpcodesRDR2::GLOBAL_U24_LOAD:
        case OpcodesRDR2::GLOBAL_U24_STORE:
        case OpcodesRDR2::PUSH_CONST_U24:
        case OpcodesRDR2::CALL_PATCH:
        case OpcodesRDR2::CALL_OUT_OF_PATCH:
            return 4;
        case OpcodesRDR2::PUSH_CONST_U32:
        case OpcodesRDR2::PUSH_CONST_F:
            return 5;
        case OpcodesRDR2::ENTER:
            return 5 + m_Code[pc + 4];
        case OpcodesRDR2::SWITCH:
            return 2 + m_Code[pc + 1] * 6;
        case OpcodesRDR2::STRING:
            return 2 + m_Code[pc + 1];
        case OpcodesRDR2::ARRAY:
            return 5 + *(uint32_t*)&m_Code[pc + 1];
        }

        return 1;
    }

    bool DisassemblerRDR2::IsJumpOrCall(uint8_t op) const
    {
        switch (static_cast<OpcodesRDR2>(op))
        {
        case OpcodesRDR2::J:
        case OpcodesRDR2::JZ:
        case OpcodesRDR2::IEQ_J:
        case OpcodesRDR2::INE_J:
        case OpcodesRDR2::IGT_J:
        case OpcodesRDR2::IGE_J:
        case OpcodesRDR2::ILT_J:
        case OpcodesRDR2::ILE_J:
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
            return true;
        }

        return false;
    }

    uint32_t DisassemblerRDR2::GetJumpTarget(uint32_t pc) const
    {
        uint8_t op = GetU8(pc);

        if (op == static_cast<uint8_t>(OpcodesRDR2::CALL))
            return GetU24(pc + 1);

        // CALL_U8H_1..F encode the high byte of the 24-bit target in the opcode
        // itself, with the low 16 bits stored as the U16 operand.
        if (op >= static_cast<uint8_t>(OpcodesRDR2::CALL_U8H_1) && op <= static_cast<uint8_t>(OpcodesRDR2::CALL_U8H_F))
        {
            uint32_t highByte = op - static_cast<uint8_t>(OpcodesRDR2::CALL_U8H_1) + 1;
            return (highByte << 16) | GetU16(pc + 1);
        }

        return pc + 2 + GetS16(pc + 1) + 1;
    }

    bool DisassemblerRDR2::IsWildcard(uint8_t op) const
    {
        switch (static_cast<OpcodesRDR2>(op))
        {
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
        case OpcodesRDR2::J:
        case OpcodesRDR2::JZ:
        case OpcodesRDR2::IEQ_J:
        case OpcodesRDR2::INE_J:
        case OpcodesRDR2::IGT_J:
        case OpcodesRDR2::IGE_J:
        case OpcodesRDR2::ILT_J:
        case OpcodesRDR2::ILE_J:
        case OpcodesRDR2::STATIC_U8:
        case OpcodesRDR2::STATIC_U8_LOAD:
        case OpcodesRDR2::STATIC_U8_STORE:
        case OpcodesRDR2::STATIC_U16:
        case OpcodesRDR2::STATIC_U16_LOAD:
        case OpcodesRDR2::STATIC_U16_STORE:
        case OpcodesRDR2::GLOBAL_U16:
        case OpcodesRDR2::GLOBAL_U16_LOAD:
        case OpcodesRDR2::GLOBAL_U16_STORE:
        case OpcodesRDR2::GLOBAL_U24:
        case OpcodesRDR2::GLOBAL_U24_LOAD:
        case OpcodesRDR2::GLOBAL_U24_STORE:
            return true;
        }

        return false;
    }

    bool DisassemblerRDR2::IsXrefToPc(uint32_t pc, uint32_t targetPc) const
    {
        uint8_t op = m_Code[pc];

        if (op == static_cast<uint8_t>(OpcodesRDR2::CALL) || op == static_cast<uint8_t>(OpcodesRDR2::PUSH_CONST_U24)) // check for function pointers
        {
            if (GetU24(pc + 1) == targetPc)
                return true;
        }
        else if (op >= static_cast<uint8_t>(OpcodesRDR2::CALL_U8H_1) && op <= static_cast<uint8_t>(OpcodesRDR2::CALL_U8H_F))
        {
            if (GetJumpTarget(pc) == targetPc)
                return true;
        }
        else if (IsJumpOrCall(op))
        {
            if ((pc + 2 + GetS16(pc + 1) + 1) == targetPc)
                return true;
        }

        return false;
    }

    std::string DisassemblerRDR2::MakePattern(uint32_t start, int len) const
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

            /*
            if (opcode == OpcodesRDR2::NATIVE)
            {
                // Wildcard native index
                for (int j = 0; j < operandSize; j++)
                {
                    if (j >= operandSize - 2)
                        ss << " ?";
                    else
                        ss << ' ' << std::setw(2) << static_cast<int>(m_Code[start + i + 1 + j]);
                }
            }
            */
            if (IsWildcard(opcode))
            {
                for (int j = 0; j < operandSize && i + 1 + j < len && (start + i + 1 + j) < m_Code.size(); j++)
                    ss << " ?";
            }
            else
            {
                for (int j = 0; j < operandSize && i + 1 + j < len && (start + i + 1 + j) < m_Code.size(); j++)
                    ss << ' ' << std::setw(2) << static_cast<int>(m_Code[start + i + 1 + j]);
            }

            i += instrSize;
        }

        return ss.str();
    }

    bool DisassemblerRDR2::IsPatternUnique(uint32_t pc, int patternLength) const
    {
        int count = 0;

        for (int i = 0; i + patternLength <= static_cast<int>(m_Code.size()); ++i)
        {
            bool match = true;
            int j = 0;

            while (j < patternLength)
            {
                uint8_t a = m_Code[pc + j];
                uint8_t b = m_Code[i + j];

                int instrSize = GetInstructionSize(pc + j);
                int operandSize = instrSize - 1;

                for (int k = 0; k < instrSize && j + k < patternLength && (pc + j + k) < m_Code.size() && (i + j + k) < m_Code.size(); ++k)
                {
                    /*
                    if (a == OpcodesRDR2::NATIVE && k > 1)
                        continue;
                    */
                    if (IsWildcard(a) && k > 0)
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

    Disassembler::BinarySearchPattern DisassemblerRDR2::MakeStringSearchPatterns(const std::string& value) const
    {
        BinarySearchPattern result;

        uint32_t pc = 0;
        while (pc < m_Code.size())
        {
            OpcodesRDR2 op = static_cast<OpcodesRDR2>(GetU8(pc));
            int insnSize = GetInstructionSize(pc);

            if (op == OpcodesRDR2::STRING)
            {
                uint8_t len = GetU8(pc + 1);
                if (len > 0 && pc + 2 + len <= m_Code.size())
                {
                    std::string str(reinterpret_cast<const char*>(&m_Code[pc + 2]), len);
                    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

                    if (str.find(value) != std::string::npos)
                    {
                        std::vector<std::optional<uint8_t>> pattern;
                        pattern.push_back(static_cast<uint8_t>(OpcodesRDR2::STRING));
                        pattern.push_back(len);
                        for (int i = 0; i < len; i++)
                            pattern.push_back(m_Code[pc + 2 + i]);
                        result.push_back(std::move(pattern));
                    }
                }
            }

            pc += insnSize;
        }

        return result;
    }

    const char* DisassemblerRDR2::GetInstructionDescription(uint8_t opcode) const
    {
        if (opcode >= m_InstructionTable.size())
            return "???";

        return m_InstructionTable[opcode].Description;
    }

    void DisassemblerRDR2::BuildFunction(uint32_t pc)
    {
        if (pc >= m_Code.size() || GetU8(pc) != static_cast<uint8_t>(OpcodesRDR2::ENTER))
            return;

        uint32_t start = pc;
        uint8_t argCount = GetU8(pc + 1);
        uint16_t frameSize = GetU16(pc + 2);
        uint8_t nameLen = GetU8(pc + 4);

        std::string name = GetFunctionName(pc, nameLen);

        uint32_t pos = pc + GetInstructionSize(pc);

        uint32_t lastLeave = 0;
        uint8_t retCount = 0;
        while (pos < m_Code.size())
        {
            uint8_t op = GetU8(pos);
            int size = GetInstructionSize(pos);

            bool isLeaveImm = op >= static_cast<uint8_t>(OpcodesRDR2::LEAVE_0_0) && op <= static_cast<uint8_t>(OpcodesRDR2::LEAVE_3_3);

            if (op == static_cast<uint8_t>(OpcodesRDR2::LEAVE) || isLeaveImm)
            {
                uint32_t next = pos + size;
                uint8_t nextOp = (next < m_Code.size()) ? GetU8(next) : 0xFF;

                // If next op is ENTER, this is the last LEAVE of the function
                if (nextOp == static_cast<uint8_t>(OpcodesRDR2::ENTER) || next >= m_Code.size())
                {
                    lastLeave = pos;

                    // LEAVE_X_Y bakes the arg/return counts into the opcode
                    // itself (no operand bytes); generic LEAVE stores them
                    // as operands (argCount, retCount).
                    retCount = isLeaveImm ? ((op - static_cast<uint8_t>(OpcodesRDR2::LEAVE_0_0)) % 4) : GetU8(pos + 2);
                    break;
                }
            }

            pos += size;
        }

        FunctionInfo info{};
        info.Start = start;
        info.End = lastLeave;
        info.Length = lastLeave + GetInstructionSize(lastLeave) - start;
        info.ArgCount = argCount;
        info.FrameSize = frameSize;
        info.RetCount = retCount;
        info.Name = name;
        m_Functions.push_back(info);
    }

    std::string DisassemblerRDR2::DecodeInstructionInternal(int index) const
    {
        std::string result = "???";

        auto& insnPc = m_Instructions[index];

        uint8_t op = GetU8(insnPc);
        if (op >= m_InstructionTable.size())
            return result;

        auto& insnTable = m_InstructionTable[op];

        std::ostringstream instr;
        instr << insnTable.Name << " ";

        uint32_t offset = insnPc + 1;

        auto fmt = insnTable.OperandFmt;
        while (*fmt)
        {
            switch (*fmt++)
            {
            case 'a': // U8
                instr << std::dec << static_cast<int>(GetU8(offset++));
                break;
            case 'b': // U16
            {
                uint16_t val = GetU16(offset);

                // CALL / CALL_U8H_1..F: the low 16 bits of the call target are
                // stored here; the high byte is implied by the opcode itself
                // (plain CALL == high byte 0).
                if (op == static_cast<uint8_t>(OpcodesRDR2::CALL) || (op >= static_cast<uint8_t>(OpcodesRDR2::CALL_U8H_1) && op <= static_cast<uint8_t>(OpcodesRDR2::CALL_U8H_F)))
                {
                    uint32_t highByte = (op == static_cast<uint8_t>(OpcodesRDR2::CALL)) ? 0 : (op - static_cast<uint8_t>(OpcodesRDR2::CALL_U8H_1) + 1);
                    uint32_t target = (highByte << 16) | val;

                    instr << "0x" << std::uppercase << std::hex << target;

                    if (auto func = GetFunctionForPc(target))
                    {
                        if (!func->Name.empty())
                            instr << " // " << func->Name;
                    }
                }
                else
                {
                    instr << std::dec << val;
                }

                offset += 2;
                break;
            }
            case 'c': // S16
                instr << std::dec << GetS16(offset);
                offset += 2;
                break;
            case 'd': // U24
            {
                uint32_t val = GetU24(offset);

                // CALL_PATCH / CALL_OUT_OF_PATCH address the PatchData code
                // space, not m_Code, so jump to xref won't work for these.
                // But since these OpcodesRDR2 are unused (?), this is fine.
                if (op == static_cast<uint8_t>(OpcodesRDR2::CALL_PATCH) || op == static_cast<uint8_t>(OpcodesRDR2::CALL_OUT_OF_PATCH))
                    instr << "0x" << std::uppercase << std::hex << val;
                else
                    instr << std::dec << val;

                offset += 3;
                break;
            }
            case 'e': // U32
                instr << std::dec << GetU32(offset);
                offset += 4;
                break;
            case 'f': // FLOAT
                instr << GetF32(offset);
                offset += 4;
                break;
            case 'g': // REL
            {
                int16_t rel = GetS16(offset);
                uint32_t target = (offset + 2) + rel;
                instr << "0x" << std::uppercase << std::hex << target << " (";

                if (rel >= 0)
                    instr << "+" << std::dec << rel;
                else
                    instr << std::dec << rel;

                instr << ")";
                offset += 2;
                break;
            }
            case 'h': // NATIVE
            {
                uint8_t byte1 = GetU8(offset++);
                uint8_t byte2 = GetU8(offset++);

                uint32_t argCount = (byte1 >> 1) & 0x1FU;
                uint32_t retCount = byte1 & 1U;
                uint32_t index = ((static_cast<uint32_t>(byte1) * 4U) & 0xFFFFFF00U) | byte2;

                uint64_t handler = m_Program->GetNative(index);
                uint64_t hash = g_Game->GetNativeHashByHandler(handler);

                instr << argCount << ", " << retCount << ", " << index;
                if (handler && hash)
                {
                    std::ostringstream nativeStr;

                    auto name = g_Game->GetNativeNameByHash(hash);
                    nativeStr << " // " << (name.empty() ? "UNKNOWN_NATIVE" : name);

                    nativeStr << ", 0x" << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << hash;
                    nativeStr << ", " << Process::GetName() << "+0x" << handler - Process::GetBaseAddress();

                    instr << nativeStr.str();
                }

                break;
            }
            case 'i': // SWITCH
            {
                uint8_t cases = GetU8(offset++);
                instr << " [" << std::dec << static_cast<int>(cases) << "]";
                for (int j = 0; j < cases; j++)
                {
                    uint32_t key = GetU32(offset);
                    int16_t rel = GetS16(offset + 4);
                    instr << " " << std::uppercase << std::hex << key << "=0x" << (offset + 6 + rel);
                    offset += 6;
                    if (j != cases - 1)
                        instr << ",";
                }
                break;
            }
            case 'm': // STRING
            {
                uint8_t len = GetU8(offset++);
                std::string str(reinterpret_cast<const char*>(&m_Code[offset]), len);

                if (!str.empty() && str.back() == '\0')
                    str.pop_back();

                auto label = g_Game->GetTextLabel(JOAAT(str));
                if (!label.empty())
                    instr << "\"" << str << "\"" << " // GXT: " << label;
                else
                    instr << "\"" << str << "\"";
                offset += len;
                break;
            }
            case 'n': // ENTER
            {
                uint8_t argCount = GetU8(offset++);
                uint16_t frameSize = GetU16(offset);
                offset += 2;
                uint8_t nameLen = GetU8(offset++);

                instr << std::dec << static_cast<int>(argCount) << ", " << frameSize;
                if (auto func = GetFunctionForPc(offset - 5))
                {
                    if (!func->Name.empty())
                        instr << ", " << func->Name;
                }
                offset += nameLen;
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