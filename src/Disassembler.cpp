#include "Disassembler.hpp"
#include "core/Process.hpp"
#include "game/gta/Natives.hpp"
#include "game/gta/TextLabels.hpp"
#include "game/rage/Joaat.hpp"
#include "game/rage/scrProgram.hpp"
#include "game/rage/Opcode.hpp"

namespace scrDbg
{
	const std::array<ScriptDisassembler::InstructionInfo, 131> ScriptDisassembler::m_InstructionTable = {{
		{"NOP", "Does nothing.", "", 1},
		{"IADD", "Adds the top two integers on the stack and pushes the result.", "", 1},
		{"ISUB", "Subtracts the top integer from the next on the stack and pushes the result.", "", 1},
		{"IMUL", "Multiplies the top two integers on the stack and pushes the result.", "", 1},
		{"IDIV", "Divides the next integer on the stack by the top one and pushes the result.", "", 1},
		{"IMOD", "Computes the remainder of the next integer divided by the top one and pushes it.", "", 1},
		{"INOT", "Pushes 1 if the top integer is 0, otherwise pushes 0.", "", 1},
		{"INEG", "Negates the integer at the top of the stack.", "", 1},
		{"IEQ", "Pushes 1 if the top two integers are equal, otherwise pushes 0.", "", 1},
		{"INE", "Pushes 1 if the top two integers are not equal, otherwise pushes 0.", "", 1},
		{"IGT", "Pushes 1 if the first integer is greater than the second, otherwise pushes 0.", "", 1},
		{"IGE", "Pushes 1 if the first integer is greater than or equal to the second, otherwise pushes 0.", "", 1},
		{"ILT", "Pushes 1 if the first integer is less than the second, otherwise pushes 0.", "", 1},
		{"ILE", "Pushes 1 if the first integer is less than or equal to the second, otherwise pushes 0.", "", 1},
		{"FADD", "Adds the top two floats on the stack and pushes the result.", "", 1},
		{"FSUB", "Subtracts the top float from the next on the stack and pushes the result.", "", 1},
		{"FMUL", "Multiplies the top two floats on the stack and pushes the result.", "", 1},
		{"FDIV", "Divides the next float by the top one and pushes the result.", "", 1},
		{"FMOD", "Computes the remainder of the next float divided by the top one and pushes it.", "", 1},
		{"FNEG", "Negates the float at the top of the stack.", "", 1},
		{"FEQ", "Pushes 1 if the top two floats are equal, otherwise pushes 0.", "", 1},
		{"FNE", "Pushes 1 if the top two floats are not equal, otherwise pushes 0.", "", 1},
		{"FGT", "Pushes 1 if the first float is greater than the second, otherwise pushes 0.", "", 1},
		{"FGE", "Pushes 1 if the first float is greater than or equal to the second, otherwise pushes 0.", "", 1},
		{"FLT", "Pushes 1 if the first float is less than the second, otherwise pushes 0.", "", 1},
		{"FLE", "Pushes 1 if the first float is less than or equal to the second, otherwise pushes 0.", "", 1},
		{"VADD", "Adds the top two vectors on the stack and pushes the result.", "", 1},
		{"VSUB", "Subtracts the top vector from the next on the stack and pushes the result.", "", 1},
		{"VMUL", "Multiplies the top two vectors on the stack and pushes the result.", "", 1},
		{"VDIV", "Divides the next vector by the top one and pushes the result.", "", 1},
		{"VNEG", "Negates the vector at the top of the stack.", "", 1},
		{"IAND", "Performs a bitwise AND on the top two integers and pushes the result.", "", 1},
		{"IOR", "Performs a bitwise OR on the top two integers and pushes the result.", "", 1},
		{"IXOR", "Performs a bitwise XOR on the top two integers and pushes the result.", "", 1},
		{"I2F", "Converts the top integer to a float and pushes it.", "", 1},
		{"F2I", "Converts the top float to an integer and pushes it.", "", 1},
		{"F2V", "Converts the top float into a vector of three identical float values and pushes it.", "", 1},
		{"PUSH_CONST_U8", "Pushes the next byte after the opcode onto the stack.", "a", 2},
		{"PUSH_CONST_U8_U8", "Pushes the next two bytes after the opcode onto the stack.", "aa", 3},
		{"PUSH_CONST_U8_U8_U8", "Pushes the next three bytes after the opcode onto the stack.", "aaa", 4},
		{"PUSH_CONST_U32", "Pushes the 4-byte integer following the opcode onto the stack.", "e", 5},
		{"PUSH_CONST_F", "Pushes the 4-byte float following the opcode onto the stack.", "f", 5},
		{"DUP", "Duplicates the top item on the stack.", "", 1},
		{"DROP", "Removes the top item from the stack.", "", 1},
		{"NATIVE", "Calls a native command using argument and return counts from the next byte, and a hash index from the next two bytes.", "N", 4},
		{"ENTER", "Marks the start of a function; defines argument count, local variable count, and name size to skip.", "aan", 0},
		{"LEAVE", "Marks the end of a function; restores the caller's frame, pops arguments, pushes return values onto the stack, and returns to the caller if there's any.", "aa", 3},
		{"LOAD", "Pops a pointer and pushes the value it points to.", "", 1},
		{"STORE", "Stores the second stack item's value at the address pointed to by the top item.", "", 1},
		{"STORE_REV", "Stores the top item's value at the address of the next item without popping it.", "", 1},
		{"LOAD_N", "Loads a specified number of items from a memory address onto the stack.", "", 1},
		{"STORE_N", "Stores a specified number of items from the stack into a memory address.", "", 1},
		{"ARRAY_U8", "Pushes the pointer to an array element by index, using an element size defined by the next byte.", "a", 2},
		{"ARRAY_U8_LOAD", "Pushes the array element at the given index onto the stack; element size is defined by the next byte.", "a", 2},
		{"ARRAY_U8_STORE", "Stores a value in the array at the given index; element size is defined by the next byte.", "a", 2},
		{"LOCAL_U8", "Pushes a pointer to a local variable indexed by the next byte.", "a", 2},
		{"LOCAL_U8_LOAD", "Pushes the value of a local variable indexed by the next byte.", "a", 2},
		{"LOCAL_U8_STORE", "Stores the top stack value into a local variable indexed by the next byte.", "a", 2},
		{"STATIC_U8", "Pushes a pointer to a static variable indexed by the next byte.", "a", 2},
		{"STATIC_U8_LOAD", "Pushes the value of a static variable indexed by the next byte.", "a", 2},
		{"STATIC_U8_STORE", "Stores the top stack value into a static variable indexed by the next byte.", "a", 2},
		{"IADD_U8", "Adds the next byte to the integer at the top of the stack and pushes the result.", "a", 2},
		{"IMUL_U8", "Multiplies the next byte with the integer at the top of the stack and pushes the result.", "a", 2},
		{"IOFFSET", "Pushes a pointer to a struct element indexed by the popped index * 8.", "", 1},
		{"IOFFSET_U8", "Pushes a pointer to a struct element at the offset (byte * 8) defined by the next byte.", "a", 2},
		{"IOFFSET_U8_LOAD", "Pushes the value of a struct element at offset (byte * 8) defined by the next byte.", "a", 2},
		{"IOFFSET_U8_STORE", "Stores a value into a struct element at offset (byte * 8) defined by the next byte.", "a", 2},
		{"PUSH_CONST_S16", "Pushes the signed short (2 bytes) following the opcode onto the stack.", "c", 3},
		{"IADD_S16", "Adds the short following the opcode to the integer at the top of the stack and pushes the result.", "c", 3},
		{"IMUL_S16", "Multiplies the short following the opcode with the integer at the top of the stack and pushes the result.", "c", 3},
		{"IOFFSET_S16", "Pushes a pointer to a struct element at offset (short * 8) defined by the next two bytes.", "c", 3},
		{"IOFFSET_S16_LOAD", "Pushes the value of a struct element at offset (short * 8) defined by the next two bytes.", "c", 3},
		{"IOFFSET_S16_STORE", "Stores a value into a struct element at offset (short * 8) defined by the next two bytes.", "c", 3},
		{"ARRAY_U16", "Pushes a pointer to an array element using an index and 2-byte element size.", "b", 3},
		{"ARRAY_U16_LOAD", "Pushes the value of an array element using an index and 2-byte element size.", "b", 3},
		{"ARRAY_U16_STORE", "Stores a value in an array element using an index and 2-byte element size.", "b", 3},
		{"LOCAL_U16", "Pushes a pointer to a local variable indexed by the next two bytes.", "b", 3},
		{"LOCAL_U16_LOAD", "Pushes the value of a local variable indexed by the next two bytes.", "b", 3},
		{"LOCAL_U16_STORE", "Stores the top stack value into a local variable indexed by the next two bytes.", "b", 3},
		{"STATIC_U16", "Pushes a pointer to a static variable indexed by the next two bytes.", "b", 3},
		{"STATIC_U16_LOAD", "Pushes the value of a static variable indexed by the next two bytes.", "b", 3},
		{"STATIC_U16_STORE", "Stores the top stack value into a static variable indexed by the next two bytes.", "b", 3},
		{"GLOBAL_U16", "Pushes a pointer to a global variable indexed by the next two bytes.", "b", 3},
		{"GLOBAL_U16_LOAD", "Pushes the value of a global variable indexed by the next two bytes.", "b", 3},
		{"GLOBAL_U16_STORE", "Stores the top stack value into a global variable indexed by the next two bytes.", "b", 3},
		{"J", "Performs an unconditional relative jump by the signed short following the opcode.", "r", 3},
		{"JZ", "Performs a relative jump if the top stack value is zero; offset defined by the following short.", "r", 3},
		{"IEQ_JZ", "Performs a relative jump if the top two integers are not equal; offset defined by the following short.", "r", 3},
		{"INE_JZ", "Performs a relative jump if the top two integers are equal; offset defined by the following short.", "r", 3},
		{"IGT_JZ", "Performs a relative jump if the first integer is less than or equal to the second; offset defined by the following short.", "r", 3},
		{"IGE_JZ", "Performs a relative jump if the first integer is less than the second; offset defined by the following short.", "r", 3},
		{"ILT_JZ", "Performs a relative jump if the first integer is greater than or equal to the second; offset defined by the following short.", "r", 3},
		{"ILE_JZ", "Performs a relative jump if the first integer is greater than the second; offset defined by the following short.", "r", 3},
		{"CALL", "Calls a function within the script; address is defined by the next three bytes.", "d", 4},
		{"STATIC_U24", "Pushes a pointer to a static variable indexed by the next three bytes.", "d", 4},
		{"STATIC_U24_LOAD", "Pushes the value of a static variable indexed by the next three bytes.", "d", 4},
		{"STATIC_U24_STORE", "Stores the top stack value into a static variable indexed by the next three bytes.", "d", 4},
		{"GLOBAL_U24", "Pushes a pointer to a global variable indexed by the next three bytes.", "d", 4},
		{"GLOBAL_U24_LOAD", "Pushes the value of a global variable indexed by the next three bytes.", "d", 4},
		{"GLOBAL_U24_STORE", "Stores the top stack value into a global variable indexed by the next three bytes.", "d", 4},
		{"PUSH_CONST_U24", "Pushes the unsigned 24-bit integer following the opcode onto the stack.", "d", 4},
		{"SWITCH", "Performs a switch jump based on a comparison value; table and offsets follow the opcode.", "x", 0},
		{"STRING", "Pushes a pointer to a string from the string table using the index at the top of the stack.", "s", 1},
		{"STRINGHASH", "Hashes the string pointed to by the top stack value using JOAAT algorithm and pushes the hash.", "", 1},
		{"TEXT_LABEL_ASSIGN_STRING", "Copies a string into a destination buffer; buffer size is given by the next byte.", "a", 2},
		{"TEXT_LABEL_ASSIGN_INT", "Converts an integer to a string and stores it in a destination buffer; size given by the next byte.", "a", 2},
		{"TEXT_LABEL_APPEND_STRING", "Appends a string to an existing buffer; buffer size is given by the next byte.", "a", 2},
		{"TEXT_LABEL_APPEND_INT", "Appends an integer (as a string) to an existing buffer; buffer size is given by the next byte.", "a", 2},
		{"TEXT_LABEL_COPY", "Copies memory from one buffer to another multiple times and appends a null terminator.", "", 1},
		{"CATCH", "Defines a protected code region for handling script errors (unused in release builds).", "", 1},
		{"THROW", "Marks an exception handling block relative to a CATCH region (unused in release builds).", "", 1},
		{"CALLINDIRECT", "Calls a script function whose address is taken from the integer at the top of the stack.", "", 1},
		{"PUSH_CONST_M1", "Pushes -1 onto the stack.", "", 1},
		{"PUSH_CONST_0", "Pushes 0 onto the stack.", "", 1},
		{"PUSH_CONST_1", "Pushes 1 onto the stack.", "", 1},
		{"PUSH_CONST_2", "Pushes 2 onto the stack.", "", 1},
		{"PUSH_CONST_3", "Pushes 3 onto the stack.", "", 1},
		{"PUSH_CONST_4", "Pushes 4 onto the stack.", "", 1},
		{"PUSH_CONST_5", "Pushes 5 onto the stack.", "", 1},
		{"PUSH_CONST_6", "Pushes 6 onto the stack.", "", 1},
		{"PUSH_CONST_7", "Pushes 7 onto the stack.", "", 1},
		{"PUSH_CONST_FM1", "Pushes -1.0 onto the stack.", "", 1},
		{"PUSH_CONST_F0", "Pushes 0.0 onto the stack.", "", 1},
		{"PUSH_CONST_F1", "Pushes 1.0 onto the stack.", "", 1},
		{"PUSH_CONST_F2", "Pushes 2.0 onto the stack.", "", 1},
		{"PUSH_CONST_F3", "Pushes 3.0 onto the stack.", "", 1},
		{"PUSH_CONST_F4", "Pushes 4.0 onto the stack.", "", 1},
		{"PUSH_CONST_F5", "Pushes 5.0 onto the stack.", "", 1},
		{"PUSH_CONST_F6", "Pushes 6.0 onto the stack.", "", 1},
		{"PUSH_CONST_F7", "Pushes 7.0 onto the stack.", "", 1},
		{"IS_BIT_SET", "Checks if a specific bit is set in a value using value & (1 << bit) and pushes the result.", "", 1},
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

	bool ScriptDisassembler::IsJumpInstruction(uint8_t opcode)
	{
		if (opcode >= m_InstructionTable.size())
			return false;

		const char* operands = m_InstructionTable[opcode].OperandFmt;
		while (*operands)
		{
			if (*operands == 'r')
				return true;
			operands++;
		}

		return false;
	}

	const char* ScriptDisassembler::GetInstructionDesc(uint8_t opcode)
	{
		if (opcode >= m_InstructionTable.size())
			return "???";

	    return m_InstructionTable[opcode].Desc;
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

		return "[INVALID_NAME]";
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

	ScriptDisassembler::FunctionInfo ScriptDisassembler::GetFunctionInfo(const std::vector<uint8_t>& code, uint32_t pc, int funcIndex)
	{
		if (pc >= code.size() || ReadByte(code, pc) != rage::scrOpcode::ENTER)
			return {};

		uint32_t start = pc;
		uint8_t argCount = ReadByte(code, pc + 1);
		uint16_t localCount = ReadS16(code, pc + 2);
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
		info.LocalCount = localCount;
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
					instr << "[INVALID_INDEX]";
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