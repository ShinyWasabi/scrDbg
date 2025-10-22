#include "Disassembler.hpp"
#include "rage/scrProgram.hpp"
#include "rage/Opcode.hpp"

namespace scrDbg
{
	const std::array<ScriptDisassembler::InstructionInfo, 131> ScriptDisassembler::m_InstructionTable = {{
		{"NOP", 1, {OperandType::NONE}},
		{"IADD", 1, {OperandType::NONE}},
		{"ISUB", 1, {OperandType::NONE}},
		{"IMUL", 1, {OperandType::NONE}},
		{"IDIV", 1, {OperandType::NONE}},
		{"IMOD", 1, {OperandType::NONE}},
		{"INOT", 1, {OperandType::NONE}},
		{"INEG", 1, {OperandType::NONE}},
		{"IEQ", 1, {OperandType::NONE}},
		{"INE", 1, {OperandType::NONE}},
		{"IGT", 1, {OperandType::NONE}},
		{"IGE", 1, {OperandType::NONE}},
		{"ILT", 1, {OperandType::NONE}},
		{"ILE", 1, {OperandType::NONE}},
		{"FADD", 1, {OperandType::NONE}},
		{"FSUB", 1, {OperandType::NONE}},
		{"FMUL", 1, {OperandType::NONE}},
		{"FDIV", 1, {OperandType::NONE}},
		{"FMOD", 1, {OperandType::NONE}},
		{"FNEG", 1, {OperandType::NONE}},
		{"FEQ", 1, {OperandType::NONE}},
		{"FNE", 1, {OperandType::NONE}},
		{"FGT", 1, {OperandType::NONE}},
		{"FGE", 1, {OperandType::NONE}},
		{"FLT", 1, {OperandType::NONE}},
		{"FLE", 1, {OperandType::NONE}},
		{"VADD", 1, {OperandType::NONE}},
		{"VSUB", 1, {OperandType::NONE}},
		{"VMUL", 1, {OperandType::NONE}},
		{"VDIV", 1, {OperandType::NONE}},
		{"VNEG", 1, {OperandType::NONE}},
		{"IAND", 1, {OperandType::NONE}},
		{"IOR", 1, {OperandType::NONE}},
		{"IXOR", 1, {OperandType::NONE}},
		{"I2F", 1, {OperandType::NONE}},
		{"F2I", 1, {OperandType::NONE}},
		{"F2V", 1, {OperandType::NONE}},
		{"PUSH_CONST_U8", 2, {OperandType::U8}},
		{"PUSH_CONST_U8_U8", 3, {OperandType::U8, OperandType::U8}},
		{"PUSH_CONST_U8_U8_U8", 4, {OperandType::U8, OperandType::U8, OperandType::U8}},
		{"PUSH_CONST_U32", 5, {OperandType::U32}},
		{"PUSH_CONST_F", 5, {OperandType::FLOAT}},
		{"DUP", 1, {OperandType::NONE}},
		{"DROP", 1, {OperandType::NONE}},
		{"NATIVE", 4, {OperandType::U8, OperandType::U8, OperandType::U8}},
		{"ENTER", 0, {OperandType::U8, OperandType::S16, OperandType::NAME}},
		{"LEAVE", 3, {OperandType::U8, OperandType::U8}},
		{"LOAD", 1, {OperandType::NONE}},
		{"STORE", 1, {OperandType::NONE}},
		{"STORE_REV", 1, {OperandType::NONE}},
		{"LOAD_N", 1, {OperandType::NONE}},
		{"STORE_N", 1, {OperandType::NONE}},
		{"ARRAY_U8", 2, {OperandType::U8}},
		{"ARRAY_U8_LOAD", 2, {OperandType::U8}},
		{"ARRAY_U8_STORE", 2, {OperandType::U8}},
		{"LOCAL_U8", 2, {OperandType::U8}},
		{"LOCAL_U8_LOAD", 2, {OperandType::U8}},
		{"LOCAL_U8_STORE", 2, {OperandType::U8}},
		{"STATIC_U8", 2, {OperandType::U8}},
		{"STATIC_U8_LOAD", 2, {OperandType::U8}},
		{"STATIC_U8_STORE", 2, {OperandType::U8}},
		{"IADD_U8", 2, {OperandType::U8}},
		{"IMUL_U8", 2, {OperandType::U8}},
		{"IOFFSET", 1, {OperandType::NONE}},
		{"IOFFSET_U8", 2, {OperandType::U8}},
		{"IOFFSET_U8_LOAD", 2, {OperandType::U8}},
		{"IOFFSET_U8_STORE", 2, {OperandType::U8}},
		{"PUSH_CONST_S16", 3, {OperandType::S16}},
		{"IADD_S16", 3, {OperandType::S16}},
		{"IMUL_S16", 3, {OperandType::S16}},
		{"IOFFSET_S16", 3, {OperandType::S16}},
		{"IOFFSET_S16_LOAD", 3, {OperandType::S16}},
		{"IOFFSET_S16_STORE", 3, {OperandType::S16}},
		{"ARRAY_U16", 3, {OperandType::U16}},
		{"ARRAY_U16_LOAD", 3, {OperandType::U16}},
		{"ARRAY_U16_STORE", 3, {OperandType::U16}},
		{"LOCAL_U16", 3, {OperandType::U16}},
		{"LOCAL_U16_LOAD", 3, {OperandType::U16}},
		{"LOCAL_U16_STORE", 3, {OperandType::U16}},
		{"STATIC_U16", 3, {OperandType::U16}},
		{"STATIC_U16_LOAD", 3, {OperandType::U16}},
		{"STATIC_U16_STORE", 3, {OperandType::U16}},
		{"GLOBAL_U16", 3, {OperandType::U16}},
		{"GLOBAL_U16_LOAD", 3, {OperandType::U16}},
		{"GLOBAL_U16_STORE", 3, {OperandType::U16}},
		{"J", 3, {OperandType::REL}},
		{"JZ", 3, {OperandType::REL}},
		{"IEQ_JZ", 3, {OperandType::REL}},
		{"INE_JZ", 3, {OperandType::REL}},
		{"IGT_JZ", 3, {OperandType::REL}},
		{"IGE_JZ", 3, {OperandType::REL}},
		{"ILT_JZ", 3, {OperandType::REL}},
		{"ILE_JZ", 3, {OperandType::REL}},
		{"CALL", 4, {OperandType::U24}},
		{"STATIC_U24", 4, {OperandType::U24}},
		{"STATIC_U24_LOAD", 4, {OperandType::U24}},
		{"STATIC_U24_STORE", 4, {OperandType::U24}},
		{"GLOBAL_U24", 4, {OperandType::U24}},
		{"GLOBAL_U24_LOAD", 4, {OperandType::U24}},
		{"GLOBAL_U24_STORE", 4, {OperandType::U24}},
		{"PUSH_CONST_U24", 4, {OperandType::U24}},
		{"SWITCH", 0, {OperandType::SWITCH}},
		{"STRING", 1, {OperandType::STRING}},
		{"STRINGHASH", 1, {OperandType::NONE}},
		{"TEXT_LABEL_ASSIGN_STRING", 2, {OperandType::U8}},
		{"TEXT_LABEL_ASSIGN_INT", 2, {OperandType::U8}},
		{"TEXT_LABEL_APPEND_STRING", 2, {OperandType::U8}},
		{"TEXT_LABEL_APPEND_INT", 2, {OperandType::U8}},
		{"TEXT_LABEL_COPY", 1, {OperandType::NONE}},
		{"CATCH", 1, {OperandType::NONE}},
		{"THROW", 1, {OperandType::NONE}},
		{"CALLINDIRECT", 1, {OperandType::NONE}},
		{"PUSH_CONST_M1", 1, {OperandType::NONE}},
		{"PUSH_CONST_0", 1, {OperandType::NONE}},
		{"PUSH_CONST_1", 1, {OperandType::NONE}},
		{"PUSH_CONST_2", 1, {OperandType::NONE}},
		{"PUSH_CONST_3", 1, {OperandType::NONE}},
		{"PUSH_CONST_4", 1, {OperandType::NONE}},
		{"PUSH_CONST_5", 1, {OperandType::NONE}},
		{"PUSH_CONST_6", 1, {OperandType::NONE}},
		{"PUSH_CONST_7", 1, {OperandType::NONE}},
		{"PUSH_CONST_FM1", 1, {OperandType::NONE}},
		{"PUSH_CONST_F0", 1, {OperandType::NONE}},
		{"PUSH_CONST_F1", 1, {OperandType::NONE}},
		{"PUSH_CONST_F2", 1, {OperandType::NONE}},
		{"PUSH_CONST_F3", 1, {OperandType::NONE}},
		{"PUSH_CONST_F4", 1, {OperandType::NONE}},
		{"PUSH_CONST_F5", 1, {OperandType::NONE}},
		{"PUSH_CONST_F6", 1, {OperandType::NONE}},
		{"PUSH_CONST_F7", 1, {OperandType::NONE}},
		{"IS_BIT_SET", 1, {OperandType::NONE}}
	}};

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

	int ScriptDisassembler::GetInstructionSize(const std::vector<uint8_t>& code, uint32_t pc)
	{
		uint8_t op = ReadByte(code, pc);

		if (op >= m_InstructionTable.size())
			return 1;

		const auto& insn = m_InstructionTable[op];
		if (insn.Size != 0)
			return insn.Size;

		if (op == rage::scrOpcode::ENTER)
		{
			uint8_t name = ReadByte(code, pc + 4);
			return 5 + name;
		}
		else if (op == rage::scrOpcode::SWITCH)
		{
			uint8_t cases = ReadByte(code, pc + 1);
			return 2 + cases * 6;
		}

		return 1;
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

	ScriptDisassembler::DecodedInstruction ScriptDisassembler::DecodeInstruction(const rage::scrProgram& program, const std::vector<uint8_t>& code, uint32_t pc, int stringIndex)
	{
		DecodedInstruction result;

		std::ostringstream addr;
		addr << "0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << pc;
		result.Address = addr.str();

		uint8_t op = ReadByte(code, pc);
		if (op >= m_InstructionTable.size())
		{
			result.Bytes = "";
			result.Instruction = "???";
			return result;
		}

		const auto& insn = m_InstructionTable[op];

		std::ostringstream bytes;
		bytes << std::hex << std::uppercase << std::setfill('0');
		int size = GetInstructionSize(code, pc);
		for (int i = 0; i < size; ++i)
			bytes << std::setw(2) << (int)ReadByte(code, pc + i) << " ";
		result.Bytes = bytes.str();

		std::ostringstream instr;
		instr << insn.Name << " ";

		uint32_t offset = pc + 1;
		for (size_t i = 0; i < insn.OperandType.size(); ++i)
		{
			auto operand = insn.OperandType[i];
			switch (operand)
			{
			case OperandType::NONE:
				break;
			case OperandType::U8:
				instr << std::dec << static_cast<int>(ReadByte(code, offset++));
				break;
			case OperandType::S16:
				instr << std::dec << ReadS16(code, offset);
				offset += 2;
				break;
			case OperandType::U16:
				instr << std::dec << ReadU16(code, offset);
				offset += 2;
				break;
			case OperandType::U24:
			{
				uint32_t val = ReadU24(code, offset);
				if (op == rage::scrOpcode::CALL) // Print CALL as hex
					instr << "0x" << std::uppercase << std::hex << val;
				else
					instr << std::dec << val;

				offset += 3;
				break;
			}
			case OperandType::U32:
				instr << std::dec << ReadU32(code, offset);
				offset += 4;
				break;
			case OperandType::FLOAT:
				instr << ReadF32(code, offset);
				offset += 4;
				break;
			case OperandType::REL:
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
			case OperandType::SWITCH:
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
			case OperandType::STRING:
			{
				if (stringIndex >= 0 && stringIndex < program.GetStringCount())
				{
					auto str = program.GetString(stringIndex);
					instr << "\"" << str << "\"";
				}
				else
				{
					instr << "[INVALID_INDEX]";
				}
				break;
			}
			case OperandType::NAME:
			{
				uint8_t nameLen = ReadByte(code, offset);
				offset += 1;

				if (nameLen > 0)
				{
					std::string name(reinterpret_cast<const char*>(&code[offset]), nameLen);

					// Remove profiler placeholders in case the script is compiled by RAGE script compiler
					if (name.size() >= 2 && name[0] == '_' && name[1] == '_')
						name.erase(0, 2);

					while (!name.empty() && (name.back() == '\0' || name.back() == ' '))
						name.pop_back();

					instr << "\"" << name << "\"";
				}
				else
				{
					instr << "[INVALID_NAME]";
				}

				offset += nameLen;
				break;
			}
			}

			if (i != insn.OperandType.size() - 1)
				instr << ", ";
		}

		result.Instruction = instr.str();
		return result;
	}
}