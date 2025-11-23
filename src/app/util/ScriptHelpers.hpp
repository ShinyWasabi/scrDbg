#pragma once

namespace scrDbgApp::ScriptHelpers
{
    extern inline uint8_t ReadByte(const std::vector<uint8_t>& code, uint32_t pc);
    extern inline uint16_t ReadU16(const std::vector<uint8_t>& code, uint32_t pc);
    extern inline int16_t ReadS16(const std::vector<uint8_t>& code, uint32_t pc);
    extern inline uint32_t ReadU24(const std::vector<uint8_t>& code, uint32_t pc);
    extern inline uint32_t ReadU32(const std::vector<uint8_t>& code, uint32_t pc);
    extern inline float ReadF32(const std::vector<uint8_t>& code, uint32_t pc);

    extern inline bool IsJumpInstruction(uint8_t opcode);
    extern inline bool IsWildcardInstruction(uint8_t opcode);
    extern inline bool IsXrefToPc(const std::vector<uint8_t>& code, uint32_t pc, uint32_t targetPc);
    extern inline int GetInstructionSize(const std::vector<uint8_t>& code, uint32_t pc);

    extern std::string MakePattern(const std::vector<uint8_t>& code, uint32_t start, int len);
    extern bool IsPatternUnique(const std::vector<uint8_t>& code, uint32_t pc, int patternLength);
    extern std::vector<uint32_t> ScanPattern(const std::vector<uint8_t>& code, const std::vector<std::optional<uint8_t>>& pattern);
}