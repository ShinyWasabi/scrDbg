#include "ScriptDisassembler.hpp"
#include "core/Process.hpp"
#include "game/gta/Natives.hpp"
#include "game/gta/TextLabels.hpp"
#include "game/rage/Joaat.hpp"
#include "game/rage/scrOpcode.hpp"
#include "game/rage/scrProgram.hpp"
#include "util/ScriptHelpers.hpp"

namespace scrDbgApp
{
    const char* ScriptDisassembler::GetInstructionDescription(uint8_t opcode)
    {
        if (opcode >= m_InstructionTable.size())
            return "???";

        return m_InstructionTable[opcode].Description;
    }

    std::optional<int> ScriptDisassembler::UpdateStringIndex(const std::vector<uint8_t>& code, uint32_t pc)
    {
        uint8_t op = ScriptHelpers::ReadByte(code, pc);

        switch (op)
        {
        case rage::scrOpcode::PUSH_CONST_0:
            return 0;
        case rage::scrOpcode::PUSH_CONST_1:
            return 1;
        case rage::scrOpcode::PUSH_CONST_2:
            return 2;
        case rage::scrOpcode::PUSH_CONST_3:
            return 3;
        case rage::scrOpcode::PUSH_CONST_4:
            return 4;
        case rage::scrOpcode::PUSH_CONST_5:
            return 5;
        case rage::scrOpcode::PUSH_CONST_6:
            return 6;
        case rage::scrOpcode::PUSH_CONST_7:
            return 7;
        case rage::scrOpcode::PUSH_CONST_U8:
            return ScriptHelpers::ReadByte(code, pc + 1);

        // Handle peephole optimizations
        case rage::scrOpcode::PUSH_CONST_U8_U8:
            return ScriptHelpers::ReadByte(code, pc + 2);
        case rage::scrOpcode::PUSH_CONST_U8_U8_U8:
            return ScriptHelpers::ReadByte(code, pc + 3);

        case rage::scrOpcode::PUSH_CONST_S16:
            return ScriptHelpers::ReadS16(code, pc + 1);
        case rage::scrOpcode::PUSH_CONST_U24:
            return ScriptHelpers::ReadU24(code, pc + 1);
        case rage::scrOpcode::PUSH_CONST_U32:
            return ScriptHelpers::ReadU32(code, pc + 1);
        default:
            return std::nullopt;
        }
    }

    std::string ScriptDisassembler::GetFunctionName(const std::vector<uint8_t>& code, uint32_t pc, uint32_t size, int funcIndex)
    {
        if (size > 0)
        {
            std::string name(reinterpret_cast<const char*>(&code[pc]), size);

            // Remove profiler placeholders in case the script is compiled by RAGE script compiler
            if (name.size() >= 2 && name[0] == '_' && name[1] == '_')
                name.erase(0, 2);

            while (!name.empty() && (name.back() == '\0' || name.back() == ' '))
                name.pop_back();

            if (!name.empty())
                return name;
        }

        if (funcIndex >= 0)
            return "func_" + std::to_string(funcIndex);

        return "<invalid>";
    }

    ScriptDisassembler::FunctionInfo ScriptDisassembler::GetFunctionInfo(const std::vector<uint8_t>& code, uint32_t pc, int funcIndex)
    {
        if (pc >= code.size() || ScriptHelpers::ReadByte(code, pc) != rage::scrOpcode::ENTER)
            return {};

        uint32_t start = pc;
        uint8_t argCount = ScriptHelpers::ReadByte(code, pc + 1);
        uint16_t frameSize = ScriptHelpers::ReadS16(code, pc + 2);
        uint8_t nameLen = ScriptHelpers::ReadByte(code, pc + 4);

        std::string name = GetFunctionName(code, pc + 5, nameLen, funcIndex);

        uint32_t pos = pc + ScriptHelpers::GetInstructionSize(code, pc);

        uint32_t lastLeave = 0;
        uint8_t retCount = 0;
        while (pos < code.size())
        {
            uint8_t op = ScriptHelpers::ReadByte(code, pos);
            int size = ScriptHelpers::GetInstructionSize(code, pos);

            if (op == rage::scrOpcode::LEAVE)
            {
                uint32_t next = pos + size;
                uint8_t nextOp = (next < code.size()) ? ScriptHelpers::ReadByte(code, next) : 0xFF;

                // If next op is ENTER, this is the last LEAVE of the function
                if (nextOp == rage::scrOpcode::ENTER || next >= code.size())
                {
                    lastLeave = pos;
                    retCount = ScriptHelpers::ReadByte(code, pos + 2);
                    break;
                }
            }

            pos += size;
        }

        ScriptDisassembler::FunctionInfo info{};
        info.Start = start;
        info.End = lastLeave;
        info.Length = lastLeave + ScriptHelpers::GetInstructionSize(code, lastLeave) - start;
        info.ArgCount = argCount;
        info.FrameSize = frameSize;
        info.RetCount = retCount;
        info.Name = name;

        return info;
    }

    ScriptDisassembler::DecodedInstruction ScriptDisassembler::DecodeInstruction(const std::vector<uint8_t>& code, uint32_t pc, const rage::scrProgram& program, int stringIndex, int funcIndex)
    {
        DecodedInstruction result;

        std::ostringstream addr;
        addr << "0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << pc;
        result.Address = addr.str();

        std::ostringstream bytes;
        bytes << std::hex << std::uppercase << std::setfill('0');
        int size = ScriptHelpers::GetInstructionSize(code, pc);
        for (int i = 0; i < size; ++i)
            bytes << std::setw(2) << static_cast<int>(ScriptHelpers::ReadByte(code, pc + i)) << " ";
        result.Bytes = bytes.str();

        uint8_t op = ScriptHelpers::ReadByte(code, pc);
        if (op >= m_InstructionTable.size())
        {
            result.Instruction = "???";
            return result;
        }

        const auto& insn = m_InstructionTable[op];

        std::ostringstream instr;
        instr << insn.Name << " ";

        uint32_t offset = pc + 1;

        auto fmt = insn.OperandFmt;
        while (*fmt)
        {
            switch (*fmt++)
            {
            case 'a': // U8
                instr << std::dec << static_cast<int>(ScriptHelpers::ReadByte(code, offset++));
                break;
            case 'b': // U16
                instr << std::dec << ScriptHelpers::ReadU16(code, offset);
                offset += 2;
                break;
            case 'c': // S16
                instr << std::dec << ScriptHelpers::ReadS16(code, offset);
                offset += 2;
                break;
            case 'd': // U24
            {
                uint32_t val = ScriptHelpers::ReadU24(code, offset);
                if (op == rage::scrOpcode::CALL) // Print CALL as hex
                {
                    instr << "0x" << std::uppercase << std::hex << val;

                    auto funcInfo = ScriptDisassembler::GetFunctionInfo(code, val, funcIndex);
                    if (!funcInfo.Name.empty())
                        instr << " // " << funcInfo.Name;
                }
                else
                {
                    instr << std::dec << val;
                }
                offset += 3;
                break;
            }
            case 'e': // U32
                instr << std::dec << ScriptHelpers::ReadU32(code, offset);
                offset += 4;
                break;
            case 'f': // FLOAT
                instr << ScriptHelpers::ReadF32(code, offset);
                offset += 4;
                break;
            case 'g': // REL
            {
                int16_t rel = ScriptHelpers::ReadS16(code, offset);
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
                uint8_t native = ScriptHelpers::ReadByte(code, offset++);
                uint32_t argCount = (native >> 2) & 0x3F;
                uint32_t retCount = native & 3;
                uint32_t index = (ScriptHelpers::ReadByte(code, offset++) << 8) | ScriptHelpers::ReadByte(code, offset++);

                uint64_t handler = program.GetNative(index);
                uint64_t hash = gta::Natives::GetHashByHandler(handler);

                instr << argCount << ", " << retCount << ", " << index;
                if (handler && hash)
                {
                    std::ostringstream nativeStr;

                    auto name = gta::Natives::GetNameByHash(hash);
                    nativeStr << " // " << (name.empty() ? "UNKNOWN_NATIVE" : name);

                    nativeStr << ", 0x" << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << hash;
                    nativeStr << ", " << Process::GetName() << "+0x" << handler - Process::GetBaseAddress();

                    instr << nativeStr.str();
                }

                break;
            }
            case 'i': // SWITCH
            {
                uint8_t cases = ScriptHelpers::ReadByte(code, offset++);
                instr << " [" << std::dec << static_cast<int>(cases) << "]";
                for (int j = 0; j < cases && j < 4; ++j)
                {
                    uint32_t key = ScriptHelpers::ReadU32(code, offset);
                    int16_t rel = ScriptHelpers::ReadS16(code, offset + 4);
                    instr << " " << std::uppercase << std::hex << key << "=0x" << (offset + 6 + rel);
                    offset += 6;
                    if (j != cases - 1 && j != 3)
                        instr << ",";
                }
                if (cases > 4)
                    instr << " ...";
                break;
            }
            case 'm': // STRING
            {
                if (stringIndex >= 0 && stringIndex < program.GetStringCount())
                {
                    auto str = program.GetString(stringIndex);
                    auto label = gta::TextLabels::GetTextLabel(RAGE_JOAAT(str));

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
                uint8_t argCount = ScriptHelpers::ReadByte(code, offset++);
                uint16_t frameSize = ScriptHelpers::ReadU16(code, offset);
                offset += 2;
                uint8_t nameLen = ScriptHelpers::ReadByte(code, offset++);

                instr << std::dec << static_cast<int>(argCount) << ", " << frameSize;
                instr << ", " << GetFunctionName(code, offset, nameLen, funcIndex);

                offset += nameLen;
                break;
            }
            }

            if (*fmt)
                instr << ", ";
        }

        result.Instruction = instr.str();
        return result;
    }
}