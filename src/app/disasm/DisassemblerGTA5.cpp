#include "DisassemblerGTA5.hpp"

namespace scrDbgApp
{
    std::string DisassemblerGTA5::GetFunctionName(uint32_t pc, uint32_t size) const
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

    std::optional<uint32_t> DisassemblerGTA5::GetStringIndex(uint32_t pc) const
    {
        Opcodes op = static_cast<Opcodes>(GetU8(pc));

        switch (op)
        {
        case Opcodes::PUSH_CONST_0:
            return 0;
        case Opcodes::PUSH_CONST_1:
            return 1;
        case Opcodes::PUSH_CONST_2:
            return 2;
        case Opcodes::PUSH_CONST_3:
            return 3;
        case Opcodes::PUSH_CONST_4:
            return 4;
        case Opcodes::PUSH_CONST_5:
            return 5;
        case Opcodes::PUSH_CONST_6:
            return 6;
        case Opcodes::PUSH_CONST_7:
            return 7;
        case Opcodes::PUSH_CONST_U8:
            return GetU8(pc + 1);

        // Handle peephole optimizations
        case Opcodes::PUSH_CONST_U8_U8:
            return GetU8(pc + 2);
        case Opcodes::PUSH_CONST_U8_U8_U8:
            return GetU8(pc + 3);

        case Opcodes::PUSH_CONST_S16:
            return GetU8(pc + 1);
        case Opcodes::PUSH_CONST_U24:
            return GetU8(pc + 1);
        case Opcodes::PUSH_CONST_U32:
            return GetU8(pc + 1);
        }

        return std::nullopt;
    }

    int DisassemblerGTA5::GetInstructionSize(uint32_t pc) const
    {
        Opcodes op = static_cast<Opcodes>(GetU8(pc));

        switch (op)
        {
        case Opcodes::PUSH_CONST_U8:
        case Opcodes::ARRAY_U8:
        case Opcodes::ARRAY_U8_LOAD:
        case Opcodes::ARRAY_U8_STORE:
        case Opcodes::LOCAL_U8:
        case Opcodes::LOCAL_U8_LOAD:
        case Opcodes::LOCAL_U8_STORE:
        case Opcodes::STATIC_U8:
        case Opcodes::STATIC_U8_LOAD:
        case Opcodes::STATIC_U8_STORE:
        case Opcodes::IADD_U8:
        case Opcodes::IMUL_U8:
        case Opcodes::IOFFSET_U8:
        case Opcodes::IOFFSET_U8_LOAD:
        case Opcodes::IOFFSET_U8_STORE:
        case Opcodes::TEXT_LABEL_ASSIGN_STRING:
        case Opcodes::TEXT_LABEL_ASSIGN_INT:
        case Opcodes::TEXT_LABEL_APPEND_STRING:
        case Opcodes::TEXT_LABEL_APPEND_INT:
            return 2;
        case Opcodes::PUSH_CONST_U8_U8:
        case Opcodes::LEAVE:
        case Opcodes::PUSH_CONST_S16:
        case Opcodes::IADD_S16:
        case Opcodes::IMUL_S16:
        case Opcodes::IOFFSET_S16:
        case Opcodes::IOFFSET_S16_LOAD:
        case Opcodes::IOFFSET_S16_STORE:
        case Opcodes::ARRAY_U16:
        case Opcodes::ARRAY_U16_LOAD:
        case Opcodes::ARRAY_U16_STORE:
        case Opcodes::LOCAL_U16:
        case Opcodes::LOCAL_U16_LOAD:
        case Opcodes::LOCAL_U16_STORE:
        case Opcodes::STATIC_U16:
        case Opcodes::STATIC_U16_LOAD:
        case Opcodes::STATIC_U16_STORE:
        case Opcodes::GLOBAL_U16:
        case Opcodes::GLOBAL_U16_LOAD:
        case Opcodes::GLOBAL_U16_STORE:
        case Opcodes::J:
        case Opcodes::JZ:
        case Opcodes::IEQ_JZ:
        case Opcodes::INE_JZ:
        case Opcodes::IGT_JZ:
        case Opcodes::IGE_JZ:
        case Opcodes::ILT_JZ:
        case Opcodes::ILE_JZ:
            return 3;
        case Opcodes::PUSH_CONST_U8_U8_U8:
        case Opcodes::NATIVE:
        case Opcodes::CALL:
        case Opcodes::STATIC_U24:
        case Opcodes::STATIC_U24_LOAD:
        case Opcodes::STATIC_U24_STORE:
        case Opcodes::GLOBAL_U24:
        case Opcodes::GLOBAL_U24_LOAD:
        case Opcodes::GLOBAL_U24_STORE:
        case Opcodes::PUSH_CONST_U24:
            return 4;
        case Opcodes::PUSH_CONST_U32:
        case Opcodes::PUSH_CONST_F:
            return 5;
        case Opcodes::ENTER:
            return 5 + m_Code[pc + 4];
        case Opcodes::SWITCH:
            return 2 + m_Code[pc + 1] * 6;
        }

        return 1;
    }

    bool DisassemblerGTA5::IsJumpOrCall(uint8_t op) const
    {
        switch (op)
        {
        case Opcodes::J:
        case Opcodes::JZ:
        case Opcodes::IEQ_JZ:
        case Opcodes::INE_JZ:
        case Opcodes::IGT_JZ:
        case Opcodes::IGE_JZ:
        case Opcodes::ILT_JZ:
        case Opcodes::ILE_JZ:
        case Opcodes::CALL:
            return true;
        }

        return false;
    }

    uint32_t DisassemblerGTA5::GetJumpTarget(uint32_t pc) const
    {
        uint8_t op = GetU8(pc);

        if (op == Opcodes::CALL)
            return GetU24(pc + 1);

        return pc + 2 + GetS16(pc + 1) + 1;
    }

    bool DisassemblerGTA5::IsWildcard(uint8_t op) const
    {
        switch (op)
        {
        case Opcodes::CALL:
        case Opcodes::J:
        case Opcodes::JZ:
        case Opcodes::IEQ_JZ:
        case Opcodes::INE_JZ:
        case Opcodes::IGT_JZ:
        case Opcodes::IGE_JZ:
        case Opcodes::ILT_JZ:
        case Opcodes::ILE_JZ:
        case Opcodes::STATIC_U8:
        case Opcodes::STATIC_U8_LOAD:
        case Opcodes::STATIC_U8_STORE:
        case Opcodes::STATIC_U16:
        case Opcodes::STATIC_U16_LOAD:
        case Opcodes::STATIC_U16_STORE:
        case Opcodes::GLOBAL_U16:
        case Opcodes::GLOBAL_U16_LOAD:
        case Opcodes::GLOBAL_U16_STORE:
        case Opcodes::STATIC_U24:
        case Opcodes::STATIC_U24_LOAD:
        case Opcodes::STATIC_U24_STORE:
        case Opcodes::GLOBAL_U24:
        case Opcodes::GLOBAL_U24_LOAD:
        case Opcodes::GLOBAL_U24_STORE:
            return true;
        }

        return false;
    }

    bool DisassemblerGTA5::IsXrefToPc(uint32_t pc, uint32_t targetPc) const
    {
        if (m_Code[pc] == Opcodes::CALL || m_Code[pc] == Opcodes::PUSH_CONST_U24) // check for function pointers
        {
            if (GetU24(pc + 1) == targetPc)
                return true;
        }
        else if (IsJumpOrCall(m_Code[pc]))
        {
            if ((pc + 2 + GetS16(pc + 1) + 1) == targetPc)
                return true;
        }

        return false;
    }

    std::string DisassemblerGTA5::MakePattern(uint32_t start, int len) const
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

            if (opcode == Opcodes::NATIVE)
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
            else if (IsWildcard(opcode))
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

    bool DisassemblerGTA5::IsPatternUnique(uint32_t pc, int patternLength) const
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
                    if (a == Opcodes::NATIVE && k > 1)
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

    Disassembler::StringSeachPattern DisassemblerGTA5::MakeStringSearchPatterns(const std::string& value) const
    {
        StringSeachPattern result;

        std::string str = value;
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        auto indices = m_Program->FindStringIndices(str);
        if (indices.empty())
            return result;

        auto buildNormal = [&](uint32_t index) {
            std::vector<std::optional<uint8_t>> p;

            auto pushLE = [&](uint32_t val, int bytes) {
                for (int i = 0; i < bytes; i++)
                    p.push_back(static_cast<uint8_t>((val >> (8 * i)) & 0xFF));
            };

            if (index < 0x08)
            {
                p.push_back(Opcodes::PUSH_CONST_0 + index);
            }
            else if (index < 0x100)
            {
                p.push_back(Opcodes::PUSH_CONST_U8);
                pushLE(index, 1);
            }
            else if (index < 0x8000)
            {
                p.push_back(Opcodes::PUSH_CONST_S16);
                pushLE(index, 2);
            }
            else if (index < 0x1000000)
            {
                p.push_back(Opcodes::PUSH_CONST_U24);
                pushLE(index, 3);
            }
            else
            {
                p.push_back(Opcodes::PUSH_CONST_U32);
                pushLE(index, 4);
            }

            p.push_back(Opcodes::STRING);
            return p;
        };

        auto buildPeephole = [&](uint32_t index) {
            StringSeachPattern out;

            if (index > 0xFF) // only U8 constants
                return out;

            uint8_t idx = static_cast<uint8_t>(index);

            // PUSH_CONST_U8_U8 <wild> <index> STRING
            {
                std::vector<std::optional<uint8_t>> p;
                p.push_back(Opcodes::PUSH_CONST_U8_U8);
                p.push_back(std::nullopt); // first U8 (unknown)
                p.push_back(idx);          // second U8 = string index
                p.push_back(Opcodes::STRING);
                out.push_back(std::move(p));
            }

            // PUSH_CONST_U8_U8_U8 <wild> <wild> <index> STRING
            {
                std::vector<std::optional<uint8_t>> p;
                p.push_back(Opcodes::PUSH_CONST_U8_U8_U8);
                p.push_back(std::nullopt); // first U8 (unknown)
                p.push_back(std::nullopt); // second U8 (unknown)
                p.push_back(idx);          // third U8 = string index
                p.push_back(Opcodes::STRING);
                out.push_back(std::move(p));
            }

            return out;
        };

        // Build all pattern variants
        for (uint32_t idx : indices)
        {
            result.push_back(buildNormal(idx));

            // peephole variants (if any)
            auto p = buildPeephole(idx);
            result.insert(result.end(), p.begin(), p.end());
        }

        return result;
    }

    const char* DisassemblerGTA5::GetInstructionDescription(uint8_t opcode) const
    {
        if (opcode >= m_InstructionTable.size())
            return "???";

        return m_InstructionTable[opcode].Description;
    }

    void DisassemblerGTA5::BuildFunction(uint32_t pc)
    {
        if (pc >= m_Code.size() || GetU8(pc) != Opcodes::ENTER)
            return;

        uint32_t start = pc;
        uint8_t argCount = GetU8(pc + 1);
        uint16_t frameSize = GetS16(pc + 2);
        uint8_t nameLen = GetU8(pc + 4);

        std::string name = GetFunctionName(pc, nameLen);

        uint32_t pos = pc + GetInstructionSize(pc);

        uint32_t lastLeave = 0;
        uint8_t retCount = 0;
        while (pos < m_Code.size())
        {
            uint8_t op = GetU8(pos);
            int size = GetInstructionSize(pos);

            if (op == Opcodes::LEAVE)
            {
                uint32_t next = pos + size;
                uint8_t nextOp = (next < m_Code.size()) ? GetU8(next) : 0xFF;

                // If next op is ENTER, this is the last LEAVE of the function
                if (nextOp == Opcodes::ENTER || next >= m_Code.size())
                {
                    lastLeave = pos;
                    retCount = GetU8(pos + 2);
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

    std::string DisassemblerGTA5::DecodeInstructionInternal(int index) const
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
                instr << std::dec << GetU16(offset);
                offset += 2;
                break;
            case 'c': // S16
                instr << std::dec << GetS16(offset);
                offset += 2;
                break;
            case 'd': // U24
            {
                uint32_t val = GetU24(offset);
                if (op == Opcodes::CALL) // Print CALL as hex
                {
                    instr << "0x" << std::uppercase << std::hex << val;

                    if (auto func = GetFunctionForPc(val))
                    {
                        if (!func->Name.empty())
                            instr << " // " << func->Name;
                    }
                }
                else
                {
                    instr << std::dec << val;
                }
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
                uint8_t native = GetU8(offset++);
                uint32_t argCount = (native >> 2) & 0x3F;
                uint32_t retCount = native & 3;
                uint32_t index = (GetU8(offset++) << 8) | GetU8(offset++);

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
                auto strIndex = GetStringIndex(m_Instructions[index - 1]);

                if (strIndex.has_value() && strIndex < m_Program->GetStringsSize())
                {
                    auto str = m_Program->GetString(*strIndex);
                    auto label = g_Game->GetTextLabel(JOAAT(str));

                    if (!label.empty())
                        instr << "\"" << str << "\"" << " // GXT: " << label;
                    else
                        instr << "\"" << str << "\"";
                }
                else
                {
                    instr << "<invalid>";
                }
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