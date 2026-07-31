#include "DisassemblerGTA5.hpp"
#include "opcodes/OpcodesGTA5.hpp"

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
        OpcodesGTA5 op = static_cast<OpcodesGTA5>(GetU8(pc));

        switch (op)
        {
        case OpcodesGTA5::PUSH_CONST_0:
            return 0;
        case OpcodesGTA5::PUSH_CONST_1:
            return 1;
        case OpcodesGTA5::PUSH_CONST_2:
            return 2;
        case OpcodesGTA5::PUSH_CONST_3:
            return 3;
        case OpcodesGTA5::PUSH_CONST_4:
            return 4;
        case OpcodesGTA5::PUSH_CONST_5:
            return 5;
        case OpcodesGTA5::PUSH_CONST_6:
            return 6;
        case OpcodesGTA5::PUSH_CONST_7:
            return 7;
        case OpcodesGTA5::PUSH_CONST_U8:
            return GetU8(pc + 1);

        // Handle peephole optimizations
        case OpcodesGTA5::PUSH_CONST_U8_U8:
            return GetU8(pc + 2);
        case OpcodesGTA5::PUSH_CONST_U8_U8_U8:
            return GetU8(pc + 3);

        case OpcodesGTA5::PUSH_CONST_S16:
            return GetS16(pc + 1);
        case OpcodesGTA5::PUSH_CONST_U24:
            return GetU24(pc + 1);
        case OpcodesGTA5::PUSH_CONST_U32:
            return GetU32(pc + 1);
        }

        return std::nullopt;
    }

    int DisassemblerGTA5::GetInstructionSize(uint32_t pc) const
    {
        return GetInsnSizeGTA5(m_Code.data(), pc);
    }

    bool DisassemblerGTA5::IsJumpOrCall(uint8_t op) const
    {
        switch (static_cast<OpcodesGTA5>(op))
        {
        case OpcodesGTA5::J:
        case OpcodesGTA5::JZ:
        case OpcodesGTA5::IEQ_JZ:
        case OpcodesGTA5::INE_JZ:
        case OpcodesGTA5::IGT_JZ:
        case OpcodesGTA5::IGE_JZ:
        case OpcodesGTA5::ILT_JZ:
        case OpcodesGTA5::ILE_JZ:
        case OpcodesGTA5::CALL:
            return true;
        }

        return false;
    }

    uint32_t DisassemblerGTA5::GetJumpTarget(uint32_t pc) const
    {
        OpcodesGTA5 op = static_cast<OpcodesGTA5>(GetU8(pc));

        if (op == OpcodesGTA5::CALL)
            return GetU24(pc + 1);

        return pc + 2 + GetS16(pc + 1) + 1;
    }

    bool DisassemblerGTA5::IsWildcard(uint8_t op) const
    {
        switch (static_cast<OpcodesGTA5>(op))
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

    bool DisassemblerGTA5::IsXrefToPc(uint32_t pc, uint32_t targetPc) const
    {
        if (m_Code[pc] == static_cast<uint8_t>(OpcodesGTA5::CALL) || m_Code[pc] == static_cast<uint8_t>(OpcodesGTA5::PUSH_CONST_U24)) // check for function pointers
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

            if (opcode == static_cast<uint8_t>(OpcodesGTA5::NATIVE))
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
                    if (a == static_cast<uint8_t>(OpcodesGTA5::NATIVE) && k > 1)
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

    Disassembler::BinarySearchPattern DisassemblerGTA5::MakeStringSearchPatterns(const std::string& value) const
    {
        BinarySearchPattern result;

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
                p.push_back(static_cast<uint8_t>(OpcodesGTA5::PUSH_CONST_0) + index);
            }
            else if (index < 0x100)
            {
                p.push_back(static_cast<uint8_t>(OpcodesGTA5::PUSH_CONST_U8));
                pushLE(index, 1);
            }
            else if (index < 0x8000)
            {
                p.push_back(static_cast<uint8_t>(OpcodesGTA5::PUSH_CONST_S16));
                pushLE(index, 2);
            }
            else if (index < 0x1000000)
            {
                p.push_back(static_cast<uint8_t>(OpcodesGTA5::PUSH_CONST_U24));
                pushLE(index, 3);
            }
            else
            {
                p.push_back(static_cast<uint8_t>(OpcodesGTA5::PUSH_CONST_U32));
                pushLE(index, 4);
            }

            p.push_back(static_cast<uint8_t>(OpcodesGTA5::STRING));
            return p;
        };

        auto buildPeephole = [&](uint32_t index) {
            BinarySearchPattern out;

            if (index > 0xFF) // only U8 constants
                return out;

            uint8_t idx = static_cast<uint8_t>(index);

            // PUSH_CONST_U8_U8 <wild> <index> STRING
            {
                std::vector<std::optional<uint8_t>> p;
                p.push_back(static_cast<uint8_t>(OpcodesGTA5::PUSH_CONST_U8_U8));
                p.push_back(std::nullopt); // first U8 (unknown)
                p.push_back(idx);          // second U8 = string index
                p.push_back(static_cast<uint8_t>(OpcodesGTA5::STRING));
                out.push_back(std::move(p));
            }

            // PUSH_CONST_U8_U8_U8 <wild> <wild> <index> STRING
            {
                std::vector<std::optional<uint8_t>> p;
                p.push_back(static_cast<uint8_t>(OpcodesGTA5::PUSH_CONST_U8_U8_U8));
                p.push_back(std::nullopt); // first U8 (unknown)
                p.push_back(std::nullopt); // second U8 (unknown)
                p.push_back(idx);          // third U8 = string index
                p.push_back(static_cast<uint8_t>(OpcodesGTA5::STRING));
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
        if (pc >= m_Code.size() || GetU8(pc) != static_cast<uint8_t>(OpcodesGTA5::ENTER))
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

            if (op == static_cast<uint8_t>(OpcodesGTA5::LEAVE))
            {
                uint32_t next = pos + size;
                uint8_t nextOp = (next < m_Code.size()) ? GetU8(next) : 0xFF;

                // If next op is ENTER, this is the last LEAVE of the function
                if (nextOp == static_cast<uint8_t>(OpcodesGTA5::ENTER) || next >= m_Code.size())
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
        auto& insnPc = m_Instructions[index];

        uint8_t op = GetU8(insnPc);
        if (op >= m_InstructionTable.size())
            return "???";

        const auto& insnTable = m_InstructionTable[op];

        std::string result;
        result.reserve(512);

        result += insnTable.Name;
        result += ' ';

        uint32_t offset = insnPc + 1;

        char buf[128];

        auto fmt = insnTable.OperandFmt;
        while (*fmt)
        {
            switch (*fmt++)
            {
            case 'a': // U8
            {
                std::snprintf(buf, sizeof(buf), "%u", GetU8(offset++));
                result += buf;
                break;
            }
            case 'b': // U16
            {
                std::snprintf(buf, sizeof(buf), "%u", GetU16(offset));
                result += buf;
                offset += 2;
                break;
            }
            case 'c': // S16
            {
                std::snprintf(buf, sizeof(buf), "%d", GetS16(offset));
                result += buf;
                offset += 2;
                break;
            }
            case 'd': // U24
            {
                uint32_t val = GetU24(offset);
                if (op == static_cast<uint8_t>(OpcodesGTA5::CALL))
                {
                    std::snprintf(buf, sizeof(buf), "0x%X", val);
                    result += buf;
                    if (auto func = GetFunctionForPc(val))
                    {
                        if (!func->Name.empty())
                        {
                            result += " // ";
                            result += func->Name;
                        }
                    }
                }
                else
                {
                    std::snprintf(buf, sizeof(buf), "%u", val);
                    result += buf;
                }
                offset += 3;
                break;
            }
            case 'e': // U32
            {
                std::snprintf(buf, sizeof(buf), "%u", GetU32(offset));
                result += buf;
                offset += 4;
                break;
            }
            case 'f': // FLOAT
            {
                std::snprintf(buf, sizeof(buf), "%g", GetF32(offset));
                result += buf;
                offset += 4;
                break;
            }
            case 'g': // REL
            {
                int16_t rel = GetS16(offset);
                uint32_t target = offset + 2 + rel;
                std::snprintf(buf, sizeof(buf), "0x%X (%+d)", target, rel);
                result += buf;
                offset += 2;
                break;
            }
            case 'h': // NATIVE
            {
                uint8_t native = GetU8(offset++);
                uint32_t argCount = (native >> 2) & 0x3F;
                uint32_t retCount = native & 3;
                uint32_t nativeIndex = (GetU8(offset++) << 8) | GetU8(offset++);

                uint64_t handler = m_Program->GetNative(nativeIndex);
                uint64_t hash = g_Game->GetNativeHashByHandler(handler);

                std::snprintf(buf, sizeof(buf), "%u, %u, %u", argCount, retCount, nativeIndex);
                result += buf;

                if (handler && hash)
                {
                    result += " // ";

                    auto name = NativeDB::GetNameByHash(hash);
                    result += name.empty() ? "UNKNOWN_NATIVE" : name;

                    std::snprintf(buf, sizeof(buf), ", 0x%016llX", static_cast<uint64_t>(hash));
                    result += buf;

                    std::snprintf(buf, sizeof(buf), ", %s+0x%llX", Process::GetName().c_str(), static_cast<uintptr_t>(handler - Process::GetBaseAddress()));
                    result += buf;
                }
                break;
            }
            case 'i': // SWITCH
            {
                uint8_t count = GetU8(offset++);
                std::snprintf(buf, sizeof(buf), " [%u]", count);
                result += buf;
                for (int i = 0; i < count; i++)
                {
                    int32_t key = static_cast<int32_t>(GetU32(offset));
                    int16_t rel = GetS16(offset + 4);
                    std::snprintf(buf, sizeof(buf), " %d=0x%X", key, offset + 6 + rel);
                    result += buf;
                    offset += 6;
                    if (i != count - 1)
                        result += ",";
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

                    result += "\"";
                    result += str;
                    result += "\"";

                    if (!label.empty())
                    {
                        result += " // GXT: ";
                        result += label;
                    }
                }
                else
                {
                    result += "<invalid>";
                }
                break;
            }
            case 'n': // ENTER
            {
                uint8_t argCount = GetU8(offset++);
                uint16_t frameSize = GetU16(offset);
                offset += 2;
                uint8_t nameLen = GetU8(offset++);

                std::snprintf(buf, sizeof(buf), "%u, %u", argCount, frameSize);
                result += buf;

                if (auto func = GetFunctionForPc(offset - 5))
                {
                    if (!func->Name.empty())
                    {
                        result += ", ";
                        result += func->Name;
                    }
                }
                offset += nameLen;
                break;
            }
            }

            if (*fmt)
                result += ", ";
        }

        return result;
    }
}