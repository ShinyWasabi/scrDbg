#pragma once

namespace scrDbgApp
{
    class Disassembler
    {
    public:
        using BinarySearchPattern = std::vector<std::vector<std::optional<uint8_t>>>;

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

        explicit Disassembler(std::unique_ptr<ScriptProgram> program);
        virtual ~Disassembler() = default;

        void Refresh();
        uint32_t GetHash() const;
        const std::vector<uint8_t>& GetCode() const;
        const ScriptProgram* GetProgram() const;
        int GetFunctionCount() const;
        FunctionInfo GetFunction(int index) const;
        virtual std::optional<FunctionInfo> GetFunctionForPc(uint32_t pc) const; // GTA 4 needs to override it
        int GetInstructionCount() const;
        uint32_t GetInstruction(int index) const;
        DecodedInstruction DecodeInstruction(int index) const;
        std::vector<uint32_t> ScanPattern(const std::vector<std::optional<uint8_t>>& pattern) const;

        virtual int GetInstructionSize(uint32_t pc) const = 0;
        virtual bool IsJumpOrCall(uint8_t op) const = 0;
        virtual uint32_t GetJumpTarget(uint32_t pc) const = 0;
        virtual bool IsWildcard(uint8_t op) const = 0;
        virtual bool IsXrefToPc(uint32_t pc, uint32_t targetPc) const = 0;
        virtual std::string MakePattern(uint32_t start, int len) const = 0;
        virtual bool IsPatternUnique(uint32_t start, int len) const = 0;
        virtual BinarySearchPattern MakeStringSearchPatterns(const std::string& value) const = 0;
        virtual const char* GetInstructionDescription(uint8_t op) const = 0;

    protected:
        struct InstructionTable
        {
            const char* Name;
            const char* Description;
            const char* OperandFmt;
        };

        virtual void BuildFunction(uint32_t pc) = 0;
        virtual std::string DecodeInstructionInternal(int index) const = 0;

        uint8_t GetU8(uint32_t pc) const
        {
            return (pc < m_Code.size()) ? m_Code[pc] : 0xFF;
        }

        uint16_t GetU16(uint32_t pc) const
        {
            return static_cast<uint16_t>(GetU8(pc) | (GetU8(pc + 1) << 8));
        }

        int16_t GetS16(uint32_t pc) const
        {
            return static_cast<int16_t>(GetU16(pc));
        }

        uint32_t GetU24(uint32_t pc) const
        {
            return static_cast<uint32_t>(GetU8(pc) | (GetU8(pc + 1) << 8) | (GetU8(pc + 2) << 16));
        }

        uint32_t GetU32(uint32_t pc) const
        {
            return static_cast<uint32_t>(GetU8(pc) | (GetU8(pc + 1) << 8) | (GetU8(pc + 2) << 16) | (GetU8(pc + 3) << 24));
        }

        float GetF32(uint32_t pc) const
        {
            float f;
            uint32_t val = GetU32(pc);
            std::memcpy(&f, &val, sizeof(float));
            return f;
        }

        uint32_t m_Hash;
        std::vector<uint8_t> m_Code;
        std::unique_ptr<ScriptProgram> m_Program;
        std::vector<FunctionInfo> m_Functions;
        std::vector<uint32_t> m_Instructions;
    };
}