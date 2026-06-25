#include "DisassemblerGTA4.hpp"

namespace scrDbgApp
{
    std::optional<Disassembler::FunctionInfo> DisassemblerGTA4::GetFunctionForPc(uint32_t pc) const
    {
        auto it = m_PcToEntry.find(pc);
        if (it == m_PcToEntry.end())
            return std::nullopt;

        uint32_t entryPc = it->second;
        for (const auto& func : m_Functions)
        {
            if (func.Start == entryPc)
                return func;
        }

        return std::nullopt;
    }

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

    Disassembler::BinarySearchPattern DisassemblerGTA4::MakeStringSearchPatterns(const std::string& value) const
    {
        BinarySearchPattern result;

        uint32_t pc = 0;
        while (pc < m_Code.size())
        {
            Opcodes op = static_cast<Opcodes>(GetU8(pc));
            int insnSize = GetInstructionSize(pc);

            if (op == Opcodes::STRING)
            {
                uint8_t len = GetU8(pc + 1);
                if (len > 0 && pc + 2 + len <= m_Code.size())
                {
                    std::string str(reinterpret_cast<const char*>(&m_Code[pc + 2]), len);
                    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

                    if (str.find(value) != std::string::npos)
                    {
                        std::vector<std::optional<uint8_t>> pattern;
                        pattern.push_back(static_cast<uint8_t>(Opcodes::STRING));
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

    const char* DisassemblerGTA4::GetInstructionDescription(uint8_t opcode) const
    {
        if (opcode >= m_InstructionTable.size())
            return "???";

        return m_InstructionTable[opcode].Description;
    }

    void DisassemblerGTA4::BuildFunction(uint32_t pc)
    {
        if (pc >= m_Code.size() || static_cast<Opcodes>(GetU8(pc)) != Opcodes::ENTER)
            return;

        const uint32_t entryPc = pc;
        const uint8_t argCount = GetU8(pc + 1);
        const uint16_t frameSize = GetU16(pc + 2);

        std::unordered_map<uint32_t, int> visited;
        std::vector<uint32_t> worklist;

        uint32_t postEnter = entryPc + GetInstructionSize(entryPc);
        worklist.push_back(postEnter);

        uint32_t lastLeavePc = 0;
        uint8_t retCount = 0;

        while (!worklist.empty())
        {
            uint32_t cur = worklist.back();
            worklist.pop_back();

            if (cur >= m_Code.size() || visited.count(cur))
                continue;

            int insnSize = GetInstructionSize(cur);
            visited[cur] = insnSize;

            Opcodes op = static_cast<Opcodes>(GetU8(cur));

            switch (op)
            {
            case Opcodes::LEAVE:
            {
                if (cur >= lastLeavePc)
                {
                    lastLeavePc = cur;
                    retCount = GetU8(cur + 2);
                }
                break;
            }
            case Opcodes::J:
            {
                worklist.push_back(GetU32(cur + 1));
                break;
            }
            case Opcodes::JZ:
            case Opcodes::JNZ:
            {
                worklist.push_back(GetU32(cur + 1));
                worklist.push_back(cur + insnSize);
                break;
            }
            case Opcodes::SWITCH:
            {
                uint8_t count = GetU8(cur + 1);
                for (int i = 0; i < count; i++)
                {
                    uint32_t target = GetU32(cur + 2 + i * 8 + 4);
                    worklist.push_back(target);
                }
                worklist.push_back(cur + insnSize);
                break;
            }
            default:
            {
                worklist.push_back(cur + insnSize);
                break;
            }
            }
        }

        if (lastLeavePc == 0)
            return;

        uint32_t totalBytes = GetInstructionSize(entryPc);
        m_PcToEntry[entryPc] = static_cast<int>(entryPc);
        for (auto& [visitedPc, size] : visited)
        {
            totalBytes += size;
            m_PcToEntry[visitedPc] = static_cast<int>(entryPc);
        }

        std::ostringstream nameStr;
        nameStr << "sub_" << std::uppercase << std::hex << entryPc;

        FunctionInfo info{};
        info.Start = entryPc;
        info.End = lastLeavePc;
        info.Length = totalBytes;
        info.ArgCount = argCount;
        info.FrameSize = frameSize;
        info.RetCount = retCount;
        info.Name = nameStr.str();
        m_Functions.push_back(info);
    }

    std::string DisassemblerGTA4::DecodeInstructionInternal(int index) const
    {
        auto& insnPc = m_Instructions[index];

        uint8_t op = GetU8(insnPc);
        if (op >= m_InstructionTable.size())
            return "???";

        const auto& insnTable = m_InstructionTable[op];

        std::ostringstream instr;
        instr << insnTable.Name << " ";

        uint32_t offset = insnPc + 1;

        const char* fmt = insnTable.OperandFmt;
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
                {
                    instr << "0x" << std::uppercase << std::hex << val;

                    if (op == Opcodes::CALL)
                    {
                        if (auto func = GetFunctionForPc(val))
                        {
                            if (!func->Name.empty())
                                instr << " // " << func->Name;
                        }
                    }
                }
                else
                {
                    instr << std::dec << val;
                }
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

                instr << std::dec << static_cast<int>(argCount) << ", " << frameSize;
                if (auto func = GetFunctionForPc(offset - 4))
                {
                    if (!func->Name.empty())
                        instr << ", " << func->Name;
                }
                break;
            }
            }

            if (*fmt)
                instr << ", ";
        }

        return instr.str();
    }
}