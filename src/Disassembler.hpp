#pragma once

namespace rage
{
	class scrProgram;
}

namespace scrDbg
{
	class ScriptDisassembler
	{
	public:
		struct FunctionInfo
		{
			uint32_t Start;
			uint32_t End;
			uint32_t Length;
			uint32_t FrameSize;
			uint32_t ArgCount;
			uint32_t RetCount;
			std::string Name;
		};

		struct DecodedInstruction
		{
			std::string Address;
			std::string Bytes;
			std::string Instruction;
		};

		static uint8_t ReadByte(const std::vector<uint8_t>& code, uint32_t pc);
		static uint16_t ReadU16(const std::vector<uint8_t>& code, uint32_t pc);
		static int16_t ReadS16(const std::vector<uint8_t>& code, uint32_t pc);
		static uint32_t ReadU24(const std::vector<uint8_t>& code, uint32_t pc);
		static uint32_t ReadU32(const std::vector<uint8_t>& code, uint32_t pc);
		static float ReadF32(const std::vector<uint8_t>& code, uint32_t pc);

		static bool IsJumpInstruction(uint8_t opcode);
		static const char* GetInstructionDesc(uint8_t opcode);
		static std::string GetFunctionName(const std::vector<uint8_t>& code, uint32_t pc, uint32_t size, int funcIndex);
		static int GetInstructionSize(const std::vector<uint8_t>& code, uint32_t pc);
		static int GetNextStringIndex(const std::vector<uint8_t>& code, uint32_t pc, int current = -1);
		static FunctionInfo GetFunctionInfo(const std::vector<uint8_t>& code, uint32_t pc, int funcIndex = -1);
		static DecodedInstruction DecodeInstruction(const std::vector<uint8_t>& code, std::uint32_t pc, const rage::scrProgram& program, int stringIndex = -1, int funcIndex = -1);

	private:
		struct InstructionInfo
		{
			const char* Name;
			const char* Desc;
			const char* OperandFmt;
		};

		static inline constexpr std::array<InstructionInfo, 131> m_InstructionTable = {{
			{"NOP", "Does nothing.", ""},
			{"IADD", "Adds the top two integers on the stack and pushes the result.", ""},
			{"ISUB", "Subtracts the top integer from the next on the stack and pushes the result.", ""},
			{"IMUL", "Multiplies the top two integers on the stack and pushes the result.", ""},
			{"IDIV", "Divides the next integer on the stack by the top one and pushes the result.", ""},
			{"IMOD", "Computes the remainder of the next integer divided by the top one and pushes it.", ""},
			{"INOT", "Pushes 1 if the top integer is 0, otherwise pushes 0.", ""},
			{"INEG", "Negates the integer at the top of the stack.", ""},
			{"IEQ", "Pushes 1 if the top two integers are equal, otherwise pushes 0.", ""},
			{"INE", "Pushes 1 if the top two integers are not equal, otherwise pushes 0.", ""},
			{"IGT", "Pushes 1 if the first integer is greater than the second, otherwise pushes 0.", ""},
			{"IGE", "Pushes 1 if the first integer is greater than or equal to the second, otherwise pushes 0.", ""},
			{"ILT", "Pushes 1 if the first integer is less than the second, otherwise pushes 0.", ""},
			{"ILE", "Pushes 1 if the first integer is less than or equal to the second, otherwise pushes 0.", ""},
			{"FADD", "Adds the top two floats on the stack and pushes the result.", ""},
			{"FSUB", "Subtracts the top float from the next on the stack and pushes the result.", ""},
			{"FMUL", "Multiplies the top two floats on the stack and pushes the result.", ""},
			{"FDIV", "Divides the next float by the top one and pushes the result.", ""},
			{"FMOD", "Computes the remainder of the next float divided by the top one and pushes it.", ""},
			{"FNEG", "Negates the float at the top of the stack.", ""},
			{"FEQ", "Pushes 1 if the top two floats are equal, otherwise pushes 0.", ""},
			{"FNE", "Pushes 1 if the top two floats are not equal, otherwise pushes 0.", ""},
			{"FGT", "Pushes 1 if the first float is greater than the second, otherwise pushes 0.", ""},
			{"FGE", "Pushes 1 if the first float is greater than or equal to the second, otherwise pushes 0.", ""},
			{"FLT", "Pushes 1 if the first float is less than the second, otherwise pushes 0.", ""},
			{"FLE", "Pushes 1 if the first float is less than or equal to the second, otherwise pushes 0.", ""},
			{"VADD", "Adds the top two vectors on the stack and pushes the result.", ""},
			{"VSUB", "Subtracts the top vector from the next on the stack and pushes the result.", ""},
			{"VMUL", "Multiplies the top two vectors on the stack and pushes the result.", ""},
			{"VDIV", "Divides the next vector by the top one and pushes the result.", ""},
			{"VNEG", "Negates the vector at the top of the stack.", ""},
			{"IAND", "Performs a bitwise AND on the top two integers and pushes the result.", ""},
			{"IOR", "Performs a bitwise OR on the top two integers and pushes the result.", ""},
			{"IXOR", "Performs a bitwise XOR on the top two integers and pushes the result.", ""},
			{"I2F", "Converts the top integer to a float and pushes it.", ""},
			{"F2I", "Converts the top float to an integer and pushes it.", ""},
			{"F2V", "Converts the top float into a vector of three identical float values and pushes it.", ""},
			{"PUSH_CONST_U8", "Pushes the next byte after the opcode onto the stack.", "a"},
			{"PUSH_CONST_U8_U8", "Pushes the next two bytes after the opcode onto the stack.", "aa"},
			{"PUSH_CONST_U8_U8_U8", "Pushes the next three bytes after the opcode onto the stack.", "aaa"},
			{"PUSH_CONST_U32", "Pushes the 4-byte integer following the opcode onto the stack.", "e"},
			{"PUSH_CONST_F", "Pushes the 4-byte float following the opcode onto the stack.", "f"},
			{"DUP", "Duplicates the top item on the stack.", ""},
			{"DROP", "Removes the top item from the stack.", ""},
			{"NATIVE", "Calls a native command using argument and return counts from the next byte, and a hash index from the next two bytes.", "N"},
			{"ENTER", "Marks the start of a function; defines argument count, local variable count, and name size to skip.", "abn"},
			{"LEAVE", "Marks the end of a function; restores the caller's frame, pops arguments, pushes return values onto the stack, and returns to the caller if there's any.", "aa"},
			{"LOAD", "Pops a pointer and pushes the value it points to.", ""},
			{"STORE", "Stores the second stack item's value at the address pointed to by the top item.", ""},
			{"STORE_REV", "Stores the top item's value at the address of the next item without popping it.", ""},
			{"LOAD_N", "Loads a specified number of items from a memory address onto the stack.", ""},
			{"STORE_N", "Stores a specified number of items from the stack into a memory address.", ""},
			{"ARRAY_U8", "Pushes the pointer to an array element by index, using an element size defined by the next byte.", "a"},
			{"ARRAY_U8_LOAD", "Pushes the array element at the given index onto the stack; element size is defined by the next byte.", "a"},
			{"ARRAY_U8_STORE", "Stores a value in the array at the given index; element size is defined by the next byte.", "a"},
			{"LOCAL_U8", "Pushes a pointer to a local variable indexed by the next byte.", "a"},
			{"LOCAL_U8_LOAD", "Pushes the value of a local variable indexed by the next byte.", "a"},
			{"LOCAL_U8_STORE", "Stores the top stack value into a local variable indexed by the next byte.", "a"},
			{"STATIC_U8", "Pushes a pointer to a static variable indexed by the next byte.", "a"},
			{"STATIC_U8_LOAD", "Pushes the value of a static variable indexed by the next byte.", "a"},
			{"STATIC_U8_STORE", "Stores the top stack value into a static variable indexed by the next byte.", "a"},
			{"IADD_U8", "Adds the next byte to the integer at the top of the stack and pushes the result.", "a"},
			{"IMUL_U8", "Multiplies the next byte with the integer at the top of the stack and pushes the result.", "a"},
			{"IOFFSET", "Pushes a pointer to a struct element indexed by the popped index * 8.", ""},
			{"IOFFSET_U8", "Pushes a pointer to a struct element at the offset (byte * 8) defined by the next byte.", "a"},
			{"IOFFSET_U8_LOAD", "Pushes the value of a struct element at offset (byte * 8) defined by the next byte.", "a"},
			{"IOFFSET_U8_STORE", "Stores a value into a struct element at offset (byte * 8) defined by the next byte.", "a"},
			{"PUSH_CONST_S16", "Pushes the signed short (2 bytes) following the opcode onto the stack.", "c"},
			{"IADD_S16", "Adds the short following the opcode to the integer at the top of the stack and pushes the result.", "c"},
			{"IMUL_S16", "Multiplies the short following the opcode with the integer at the top of the stack and pushes the result.", "c"},
			{"IOFFSET_S16", "Pushes a pointer to a struct element at offset (short * 8) defined by the next two bytes.", "c"},
			{"IOFFSET_S16_LOAD", "Pushes the value of a struct element at offset (short * 8) defined by the next two bytes.", "c"},
			{"IOFFSET_S16_STORE", "Stores a value into a struct element at offset (short * 8) defined by the next two bytes.", "c"},
			{"ARRAY_U16", "Pushes a pointer to an array element using an index and 2-byte element size.", "b"},
			{"ARRAY_U16_LOAD", "Pushes the value of an array element using an index and 2-byte element size.", "b"},
			{"ARRAY_U16_STORE", "Stores a value in an array element using an index and 2-byte element size.", "b"},
			{"LOCAL_U16", "Pushes a pointer to a local variable indexed by the next two bytes.", "b"},
			{"LOCAL_U16_LOAD", "Pushes the value of a local variable indexed by the next two bytes.", "b"},
			{"LOCAL_U16_STORE", "Stores the top stack value into a local variable indexed by the next two bytes.", "b"},
			{"STATIC_U16", "Pushes a pointer to a static variable indexed by the next two bytes.", "b"},
			{"STATIC_U16_LOAD", "Pushes the value of a static variable indexed by the next two bytes.", "b"},
			{"STATIC_U16_STORE", "Stores the top stack value into a static variable indexed by the next two bytes.", "b"},
			{"GLOBAL_U16", "Pushes a pointer to a global variable indexed by the next two bytes.", "b"},
			{"GLOBAL_U16_LOAD", "Pushes the value of a global variable indexed by the next two bytes.", "b"},
			{"GLOBAL_U16_STORE", "Stores the top stack value into a global variable indexed by the next two bytes.", "b"},
			{"J", "Performs an unconditional relative jump by the signed short following the opcode.", "r"},
			{"JZ", "Performs a relative jump if the top stack value is zero; offset defined by the following short.", "r"},
			{"IEQ_JZ", "Performs a relative jump if the top two integers are not equal; offset defined by the following short.", "r"},
			{"INE_JZ", "Performs a relative jump if the top two integers are equal; offset defined by the following short.", "r"},
			{"IGT_JZ", "Performs a relative jump if the first integer is less than or equal to the second; offset defined by the following short.", "r"},
			{"IGE_JZ", "Performs a relative jump if the first integer is less than the second; offset defined by the following short.", "r"},
			{"ILT_JZ", "Performs a relative jump if the first integer is greater than or equal to the second; offset defined by the following short.", "r"},
			{"ILE_JZ", "Performs a relative jump if the first integer is greater than the second; offset defined by the following short.", "r"},
			{"CALL", "Calls a function within the script; address is defined by the next three bytes.", "d"},
			{"STATIC_U24", "Pushes a pointer to a static variable indexed by the next three bytes.", "d"},
			{"STATIC_U24_LOAD", "Pushes the value of a static variable indexed by the next three bytes.", "d"},
			{"STATIC_U24_STORE", "Stores the top stack value into a static variable indexed by the next three bytes.", "d"},
			{"GLOBAL_U24", "Pushes a pointer to a global variable indexed by the next three bytes.", "d"},
			{"GLOBAL_U24_LOAD", "Pushes the value of a global variable indexed by the next three bytes.", "d"},
			{"GLOBAL_U24_STORE", "Stores the top stack value into a global variable indexed by the next three bytes.", "d"},
			{"PUSH_CONST_U24", "Pushes the unsigned 24-bit integer following the opcode onto the stack.", "d"},
			{"SWITCH", "Performs a switch jump based on a comparison value; table and offsets follow the opcode.", "x"},
			{"STRING", "Pushes a pointer to a string from the string table using the index at the top of the stack.", "s"},
			{"STRINGHASH", "Hashes the string pointed to by the top stack value using JOAAT algorithm and pushes the hash.", ""},
			{"TEXT_LABEL_ASSIGN_STRING", "Copies a string into a destination buffer; buffer size is given by the next byte.", "a"},
			{"TEXT_LABEL_ASSIGN_INT", "Converts an integer to a string and stores it in a destination buffer; size given by the next byte.", "a"},
			{"TEXT_LABEL_APPEND_STRING", "Appends a string to an existing buffer; buffer size is given by the next byte.", "a"},
			{"TEXT_LABEL_APPEND_INT", "Appends an integer (as a string) to an existing buffer; buffer size is given by the next byte.", "a"},
			{"TEXT_LABEL_COPY", "Copies memory from one buffer to another multiple times and appends a null terminator.", ""},
			{"CATCH", "Defines a protected code region for handling script errors (unused in release builds).", ""},
			{"THROW", "Marks an exception handling block relative to a CATCH region (unused in release builds).", ""},
			{"CALLINDIRECT", "Calls a script function whose address is taken from the integer at the top of the stack.", ""},
			{"PUSH_CONST_M1", "Pushes -1 onto the stack.", ""},
			{"PUSH_CONST_0", "Pushes 0 onto the stack.", ""},
			{"PUSH_CONST_1", "Pushes 1 onto the stack.", ""},
			{"PUSH_CONST_2", "Pushes 2 onto the stack.", ""},
			{"PUSH_CONST_3", "Pushes 3 onto the stack.", ""},
			{"PUSH_CONST_4", "Pushes 4 onto the stack.", ""},
			{"PUSH_CONST_5", "Pushes 5 onto the stack.", ""},
			{"PUSH_CONST_6", "Pushes 6 onto the stack.", ""},
			{"PUSH_CONST_7", "Pushes 7 onto the stack.", ""},
			{"PUSH_CONST_FM1", "Pushes -1.0 onto the stack.", ""},
			{"PUSH_CONST_F0", "Pushes 0.0 onto the stack.", ""},
			{"PUSH_CONST_F1", "Pushes 1.0 onto the stack.", ""},
			{"PUSH_CONST_F2", "Pushes 2.0 onto the stack.", ""},
			{"PUSH_CONST_F3", "Pushes 3.0 onto the stack.", ""},
			{"PUSH_CONST_F4", "Pushes 4.0 onto the stack.", ""},
			{"PUSH_CONST_F5", "Pushes 5.0 onto the stack.", ""},
			{"PUSH_CONST_F6", "Pushes 6.0 onto the stack.", ""},
			{"PUSH_CONST_F7", "Pushes 7.0 onto the stack.", ""},
			{"IS_BIT_SET", "Checks if a specific bit is set in a value using value & (1 << bit) and pushes the result.", ""},
		}};
	};
}