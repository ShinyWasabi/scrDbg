#include "Disassembler.hpp"
#include "core/Process.hpp"
#include "game/gta/Natives.hpp"
#include "game/gta/TextLabels.hpp"
#include "game/rage/Joaat.hpp"
#include "game/rage/scrProgram.hpp"
#include "game/rage/Opcode.hpp"

namespace scrDbg
{
	uint8_t ScriptDisassembler::ReadByte(const std::vector<uint8_t>& code, uint32_t pc)
	{
		return (pc < code.size()) ? code[pc] : 0xFF;
	}

	uint16_t ScriptDisassembler::ReadU16(const std::vector<uint8_t>& code, uint32_t pc)
	{
		return static_cast<uint16_t>(ReadByte(code, pc) | (ReadByte(code, pc + 1) << 8));
	}

	int16_t ScriptDisassembler::ReadS16(const std::vector<uint8_t>& code, uint32_t pc)
	{
		return static_cast<int16_t>(ReadU16(code, pc));
	}

	uint32_t ScriptDisassembler::ReadU24(const std::vector<uint8_t>& code, uint32_t pc)
	{
		return static_cast<uint32_t>(ReadByte(code, pc) | (ReadByte(code, pc + 1) << 8) | (ReadByte(code, pc + 2) << 16));
	}

	uint32_t ScriptDisassembler::ReadU32(const std::vector<uint8_t>& code, uint32_t pc)
	{
		return static_cast<uint32_t>(ReadByte(code, pc) | (ReadByte(code, pc + 1) << 8) | (ReadByte(code, pc + 2) << 16) | (ReadByte(code, pc + 3) << 24));
	}

	float ScriptDisassembler::ReadF32(const std::vector<uint8_t>& code, uint32_t pc)
	{
		float f;
		uint32_t val = ReadU32(code, pc);
		std::memcpy(&f, &val, sizeof(float));
		return f;
	}

	bool ScriptDisassembler::IsJumpInstruction(uint8_t opcode)
	{
		if (opcode >= m_InstructionTable.size())
			return false;

		const char* fmt = m_InstructionTable[opcode].OperandFmt;
		while (*fmt)
		{
			if (*fmt == 'r')
				return true;
			fmt++;
		}

		return false;
	}

	const char* ScriptDisassembler::GetInstructionDesc(uint8_t opcode)
	{
		if (opcode >= m_InstructionTable.size())
			return "???";

	    return m_InstructionTable[opcode].Desc;
	}

	int ScriptDisassembler::GetInstructionSize(const std::vector<uint8_t>& code, uint32_t pc)
	{
		uint8_t op = ReadByte(code, pc);

		if (op >= m_InstructionTable.size())
			return 1;

		if (op == rage::scrOpcode::ENTER)
			return 5 + ReadByte(code, pc + 4);
		else if (op == rage::scrOpcode::SWITCH)
			return 2 + ReadByte(code, pc + 1) * 6;

		const auto& insn = m_InstructionTable[op];

		uint8_t size = 1;
		for (const char* fmt = insn.OperandFmt; *fmt; ++fmt)
		{
			switch (*fmt)
			{
			case 'a': size += 1; break;
			case 'b': size += 2; break;
			case 'c': size += 2; break;
			case 'd': size += 3; break;
			case 'e': size += 4; break;
			case 'f': size += 4; break;
			case 'r': size += 2; break;
			case 'N': size += 3; break;
			}
		}

		return size;
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

	int ScriptDisassembler::GetNextStringIndex(const std::vector<uint8_t>& code, uint32_t pc, int current)
	{
		uint8_t op = ReadByte(code, pc);

		switch (op)
		{
		case rage::scrOpcode::PUSH_CONST_0:   return 0;
		case rage::scrOpcode::PUSH_CONST_1:   return 1;
		case rage::scrOpcode::PUSH_CONST_2:   return 2;
		case rage::scrOpcode::PUSH_CONST_3:   return 3;
		case rage::scrOpcode::PUSH_CONST_4:   return 4;
		case rage::scrOpcode::PUSH_CONST_5:   return 5;
		case rage::scrOpcode::PUSH_CONST_6:   return 6;
		case rage::scrOpcode::PUSH_CONST_7:   return 7;
		case rage::scrOpcode::PUSH_CONST_U8:  return ReadByte(code, pc + 1);
		case rage::scrOpcode::PUSH_CONST_S16: return ReadS16(code, pc + 1);
		case rage::scrOpcode::PUSH_CONST_U24: return ReadU24(code, pc + 1);
		case rage::scrOpcode::PUSH_CONST_U32: return ReadU32(code, pc + 1);
		default: if (op != rage::scrOpcode::STRING) return -1;
		}

		return current;
	}

	ScriptDisassembler::FunctionInfo ScriptDisassembler::GetFunctionInfo(const std::vector<uint8_t>& code, uint32_t pc, int funcIndex)
	{
		if (pc >= code.size() || ReadByte(code, pc) != rage::scrOpcode::ENTER)
			return {};

		uint32_t start = pc;
		uint8_t argCount = ReadByte(code, pc + 1);
		uint16_t frameSize = ReadS16(code, pc + 2);
		uint8_t nameLen = ReadByte(code, pc + 4);

		std::string name = GetFunctionName(code, pc + 5, nameLen, funcIndex);

		uint32_t pos = pc + GetInstructionSize(code, pc);
		uint32_t lastLeave = 0;
		uint8_t retCount = 0;

		while (pos < code.size())
		{
			uint8_t op = ReadByte(code, pos);
			int size = GetInstructionSize(code, pos);

			if (op == rage::scrOpcode::LEAVE)
			{
				uint32_t next = pos + size;
				uint8_t nextOp = (next < code.size()) ? ReadByte(code, next) : 0xFF;

				// If next op is ENTER, this is the last LEAVE of the function
				if (nextOp == rage::scrOpcode::ENTER || next >= code.size())
				{
					lastLeave = pos;
					retCount = ReadByte(code, pos + 2);
					break;
				}
			}

			pos += size;
		}

		ScriptDisassembler::FunctionInfo info{};
		info.Start = start;
		info.End = lastLeave;
		info.Length = lastLeave + GetInstructionSize(code, lastLeave) - start;
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
		int size = GetInstructionSize(code, pc);
		for (int i = 0; i < size; ++i)
			bytes << std::setw(2) << static_cast<int>(ReadByte(code, pc + i)) << " ";
		result.Bytes = bytes.str();

		uint8_t op = ReadByte(code, pc);
		if (op >= m_InstructionTable.size())
		{
			result.Instruction = "???";
			return result;
		}

		const auto& insn = m_InstructionTable[op];

		std::ostringstream instr;
		instr << insn.Name << " ";

		uint32_t offset = pc + 1;

		auto operands = insn.OperandFmt;
		while (*operands)
		{
			switch (*operands++)
			{
			case 'a': // U8
				instr << std::dec << static_cast<int>(ReadByte(code, offset++));
				break;
			case 'b': // U16
				instr << std::dec << ReadU16(code, offset);
				offset += 2;
				break;
			case 'c': // S16
				instr << std::dec << ReadS16(code, offset);
				offset += 2;
				break;
			case 'd': // U24
			{
				uint32_t val = ReadU24(code, offset);
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
				instr << std::dec << ReadU32(code, offset);
				offset += 4;
				break;
			case 'f': // FLOAT
				instr << ReadF32(code, offset);
				offset += 4;
				break;
			case 'r': // REL
			{
				int16_t rel = ReadS16(code, offset);
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
			case 'N': // NATIVE
			{
				uint8_t native = ReadByte(code, offset++);
				uint32_t argCount = (native >> 2) & 0x3F;
				uint32_t retCount = native & 3;
				uint32_t index = (ReadByte(code, offset++) << 8) | ReadByte(code, offset++);

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
			case 'x': // SWITCH
			{
				uint8_t cases = ReadByte(code, offset++);
				instr << " [" << std::dec << static_cast<int>(cases) << "]";
				for (int j = 0; j < cases && j < 4; ++j)
				{
					uint32_t key = ReadU32(code, offset);
					int16_t rel = ReadS16(code, offset + 4);
					instr << " " << std::uppercase << std::hex << key << "=0x" << (offset + 6 + rel);
					offset += 6;
					if (j != cases - 1 && j != 3)
						instr << ",";
				}
				if (cases > 4)
					instr << " ...";
				break;
			}
			case 's': // STRING
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
			case 'n': // NAME
			{
				uint8_t nameLen = ReadByte(code, offset);
				offset += 1;

				instr << GetFunctionName(code, offset, nameLen, funcIndex);

				offset += nameLen;
				break;
			}
			}

			if (*operands)
				instr << ", ";
		}

		result.Instruction = instr.str();
		return result;
	}
}