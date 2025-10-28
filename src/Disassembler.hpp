#pragma once

namespace rage
{
	class scrProgram;
}

namespace scrDbg
{
	enum class OperandType : uint8_t
	{
		NONE,   // No operand
		U8,     // 1 byte unsigned
		S16,    // 2 byte signed
		U16,    // 2 byte unsigned
		U24,    // 3 byte unsigned
		U32,    // 4 byte unsigned
		FLOAT,  // 4 byte float
		REL,    // Relative jump
		NATIVE, // Native command call
		SWITCH, // Switch-case
		STRING, // String
		NAME    // Function name
	};

	class ScriptDisassembler
	{
	public:
		struct InstructionInfo
		{
			const char* Name;
			uint8_t Size;
			std::vector<OperandType> OperandType;
		};

		struct DecodedInstruction
		{
			std::string Address;
			std::string Bytes;
			std::string Instruction;
		};

		static uint8_t ReadByte(const std::vector<uint8_t>& code, uint32_t pc);
		static uint16_t ReadU16(const std::vector<uint8_t>& code, std::uint32_t pc);
		static int16_t ReadS16(const std::vector<uint8_t>& code, std::uint32_t pc);
		static uint32_t ReadU24(const std::vector<uint8_t>& code, std::uint32_t pc);
		static uint32_t ReadU32(const std::vector<uint8_t>& code, std::uint32_t pc);
		static float ReadF32(const std::vector<uint8_t>& code, std::uint32_t pc);

		static bool IsJumpInstruction(uint8_t opcode);

		static int GetInstructionSize(const std::vector<uint8_t>& code, std::uint32_t pc);
		static int GetNextStringIndex(const std::vector<uint8_t>& code, uint32_t pc, int current = -1);
		static DecodedInstruction DecodeInstruction(const rage::scrProgram& program, const std::vector<uint8_t>& code, std::uint32_t pc, int stringIndex = -1);

	private:
		static const std::array<ScriptDisassembler::InstructionInfo, 131> m_InstructionTable;
	};
}