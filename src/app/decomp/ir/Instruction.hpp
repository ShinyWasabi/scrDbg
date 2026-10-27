#pragma once

namespace scrDbgApp::Decomp
{
    enum class IrOp : uint8_t
    {
        NOP,
        IADD,
        ISUB,
        IMUL,
        IDIV,
        IMOD,
        INOT,
        INEG,
        IEQ,
        INE,
        IGT,
        IGE,
        ILT,
        ILE,
        FADD,
        FSUB,
        FMUL,
        FDIV,
        FMOD,
        FNEG,
        FEQ,
        FNE,
        FGT,
        FGE,
        FLT,
        FLE,
        VADD,
        VSUB,
        VMUL,
        VDIV,
        VNEG,
        IAND,
        IOR,
        IXOR,
        I2F,
        F2I,
        F2V,
        PUSH_INT,   // Ints
        PUSH_FLOAT, // Float
        DUP,
        DROP,
        NATIVE, // Imm = native index, ArgCount, RetCount
        ENTER,
        LEAVE, // ArgCount, RetCount
        LOAD,
        STORE,
        STORE_REV,
        LOAD_N,
        STORE_N,
        ARRAY,       // Imm = element size
        ARRAY_LOAD,  // Imm = element size
        ARRAY_STORE, // Imm = element size
        LOCAL,       // Imm = frame index
        LOCAL_LOAD,
        LOCAL_STORE,
        STATIC, // Imm = static index
        STATIC_LOAD,
        STATIC_STORE,
        GLOBAL, // Imm = global index
        GLOBAL_LOAD,
        GLOBAL_STORE,
        IADD_IMM, // Imm
        IMUL_IMM, // Imm
        IOFFSET,
        IOFFSET_IMM,       // Imm
        IOFFSET_IMM_LOAD,  // Imm
        IOFFSET_IMM_STORE, // Imm
        J,                 // Target
        JZ,                // Target
        IEQ_JZ,            // Target
        INE_JZ,            // Target
        IGT_JZ,            // Target
        IGE_JZ,            // Target
        ILT_JZ,            // Target
        ILE_JZ,            // Target
        CALL,              // Target
        SWITCH,            // Cases
        STRING,
        STRINGHASH,
        TEXT_LABEL_ASSIGN_STRING, // Imm = buffer size
        TEXT_LABEL_ASSIGN_INT,    // Imm = buffer size
        TEXT_LABEL_APPEND_STRING, // Imm = buffer size
        TEXT_LABEL_APPEND_INT,    // Imm = buffer size
        TEXT_LABEL_COPY,
        CATCH,
        THROW,
        CALLINDIRECT,
        IS_BIT_SET,
        UNKNOWN // Byte the lifter could not decode
    };

    const char* GetIrOpName(IrOp op);

    struct SwitchCase
    {
        int32_t Value;
        uint32_t Target;
    };

    struct Instruction
    {
        IrOp Op = IrOp::NOP;
        IrOp OriginalOp = IrOp::NOP; // passes that need to know what a NOP'd slot was read this
        uint8_t RawOpcode = 0;
        uint32_t Offset = 0;
        uint32_t Length = 1;
        int64_t Imm = 0;
        uint32_t Target = 0;
        uint8_t ArgCount = 0;
        uint8_t RetCount = 0;
        float Float = 0.0f;
        std::vector<int32_t> Ints;
        std::vector<SwitchCase> Cases;

        Instruction() = default;

        Instruction(IrOp op, uint8_t rawOpcode, uint32_t offset, uint32_t length)
            : Op(op),
              OriginalOp(op),
              RawOpcode(rawOpcode),
              Offset(offset),
              Length(length)
        {
        }

        void Nop()
        {
            Op = IrOp::NOP;
        }

        uint32_t NextOffset() const
        {
            return Offset + Length;
        }

        bool IsJump() const
        {
            return OriginalOp >= IrOp::J && OriginalOp <= IrOp::ILE_JZ;
        }

        bool IsConditionalJump() const
        {
            return Op >= IrOp::JZ && Op <= IrOp::ILE_JZ;
        }

        bool IsBackwardJump() const
        {
            return Op == IrOp::J && Target <= Offset;
        }

        // Jump whose target is the instruction right after it
        bool IsNullJump() const
        {
            return IsJump() && Target == NextOffset();
        }
    };
}