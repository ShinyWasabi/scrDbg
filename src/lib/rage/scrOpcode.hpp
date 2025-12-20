#pragma once

namespace rage
{
    enum scrOpcode : std::uint8_t
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
        PUSH_CONST_U8,
        PUSH_CONST_U8_U8,
        PUSH_CONST_U8_U8_U8,
        PUSH_CONST_U32,
        PUSH_CONST_F,
        DUP,
        DROP,
        NATIVE,
        ENTER,
        LEAVE,
        LOAD,
        STORE,
        STORE_REV,
        LOAD_N,
        STORE_N,
        ARRAY_U8,
        ARRAY_U8_LOAD,
        ARRAY_U8_STORE,
        LOCAL_U8,
        LOCAL_U8_LOAD,
        LOCAL_U8_STORE,
        STATIC_U8,
        STATIC_U8_LOAD,
        STATIC_U8_STORE,
        IADD_U8,
        IMUL_U8,
        IOFFSET,
        IOFFSET_U8,
        IOFFSET_U8_LOAD,
        IOFFSET_U8_STORE,
        PUSH_CONST_S16,
        IADD_S16,
        IMUL_S16,
        IOFFSET_S16,
        IOFFSET_S16_LOAD,
        IOFFSET_S16_STORE,
        ARRAY_U16,
        ARRAY_U16_LOAD,
        ARRAY_U16_STORE,
        LOCAL_U16,
        LOCAL_U16_LOAD,
        LOCAL_U16_STORE,
        STATIC_U16,
        STATIC_U16_LOAD,
        STATIC_U16_STORE,
        GLOBAL_U16,
        GLOBAL_U16_LOAD,
        GLOBAL_U16_STORE,
        J,
        JZ,
        IEQ_JZ,
        INE_JZ,
        IGT_JZ,
        IGE_JZ,
        ILT_JZ,
        ILE_JZ,
        CALL,
        STATIC_U24,
        STATIC_U24_LOAD,
        STATIC_U24_STORE,
        GLOBAL_U24,
        GLOBAL_U24_LOAD,
        GLOBAL_U24_STORE,
        PUSH_CONST_U24,
        SWITCH,
        STRING,
        STRINGHASH,
        TEXT_LABEL_ASSIGN_STRING,
        TEXT_LABEL_ASSIGN_INT,
        TEXT_LABEL_APPEND_STRING,
        TEXT_LABEL_APPEND_INT,
        TEXT_LABEL_COPY,
        CATCH,
        THROW,
        CALLINDIRECT,
        PUSH_CONST_M1,
        PUSH_CONST_0,
        PUSH_CONST_1,
        PUSH_CONST_2,
        PUSH_CONST_3,
        PUSH_CONST_4,
        PUSH_CONST_5,
        PUSH_CONST_6,
        PUSH_CONST_7,
        PUSH_CONST_FM1,
        PUSH_CONST_F0,
        PUSH_CONST_F1,
        PUSH_CONST_F2,
        PUSH_CONST_F3,
        PUSH_CONST_F4,
        PUSH_CONST_F5,
        PUSH_CONST_F6,
        PUSH_CONST_F7,
        IS_BIT_SET
    };

    inline static constexpr int GetInstructionSize(const std::uint8_t* code)
    {
        uint8_t opcode = code[0];

        switch (opcode)
        {
        case scrOpcode::NOP:
            return 1;
        case scrOpcode::IADD:
            return 1;
        case scrOpcode::ISUB:
            return 1;
        case scrOpcode::IMUL:
            return 1;
        case scrOpcode::IDIV:
            return 1;
        case scrOpcode::IMOD:
            return 1;
        case scrOpcode::INOT:
            return 1;
        case scrOpcode::INEG:
            return 1;
        case scrOpcode::IEQ:
            return 1;
        case scrOpcode::INE:
            return 1;
        case scrOpcode::IGT:
            return 1;
        case scrOpcode::IGE:
            return 1;
        case scrOpcode::ILT:
            return 1;
        case scrOpcode::ILE:
            return 1;
        case scrOpcode::FADD:
            return 1;
        case scrOpcode::FSUB:
            return 1;
        case scrOpcode::FMUL:
            return 1;
        case scrOpcode::FDIV:
            return 1;
        case scrOpcode::FMOD:
            return 1;
        case scrOpcode::FNEG:
            return 1;
        case scrOpcode::FEQ:
            return 1;
        case scrOpcode::FNE:
            return 1;
        case scrOpcode::FGT:
            return 1;
        case scrOpcode::FGE:
            return 1;
        case scrOpcode::FLT:
            return 1;
        case scrOpcode::FLE:
            return 1;
        case scrOpcode::VADD:
            return 1;
        case scrOpcode::VSUB:
            return 1;
        case scrOpcode::VMUL:
            return 1;
        case scrOpcode::VDIV:
            return 1;
        case scrOpcode::VNEG:
            return 1;
        case scrOpcode::IAND:
            return 1;
        case scrOpcode::IOR:
            return 1;
        case scrOpcode::IXOR:
            return 1;
        case scrOpcode::I2F:
            return 1;
        case scrOpcode::F2I:
            return 1;
        case scrOpcode::F2V:
            return 1;
        case scrOpcode::PUSH_CONST_U8:
            return 2;
        case scrOpcode::PUSH_CONST_U8_U8:
            return 3;
        case scrOpcode::PUSH_CONST_U8_U8_U8:
            return 4;
        case scrOpcode::PUSH_CONST_U32:
            return 5;
        case scrOpcode::PUSH_CONST_F:
            return 5;
        case scrOpcode::DUP:
            return 1;
        case scrOpcode::DROP:
            return 1;
        case scrOpcode::NATIVE:
            return 4;
        case scrOpcode::ENTER:
            return 5 + code[4];
        case scrOpcode::LEAVE:
            return 3;
        case scrOpcode::LOAD:
            return 1;
        case scrOpcode::STORE:
            return 1;
        case scrOpcode::STORE_REV:
            return 1;
        case scrOpcode::LOAD_N:
            return 1;
        case scrOpcode::STORE_N:
            return 1;
        case scrOpcode::ARRAY_U8:
            return 2;
        case scrOpcode::ARRAY_U8_LOAD:
            return 2;
        case scrOpcode::ARRAY_U8_STORE:
            return 2;
        case scrOpcode::LOCAL_U8:
            return 2;
        case scrOpcode::LOCAL_U8_LOAD:
            return 2;
        case scrOpcode::LOCAL_U8_STORE:
            return 2;
        case scrOpcode::STATIC_U8:
            return 2;
        case scrOpcode::STATIC_U8_LOAD:
            return 2;
        case scrOpcode::STATIC_U8_STORE:
            return 2;
        case scrOpcode::IADD_U8:
            return 2;
        case scrOpcode::IMUL_U8:
            return 2;
        case scrOpcode::IOFFSET:
            return 1;
        case scrOpcode::IOFFSET_U8:
            return 2;
        case scrOpcode::IOFFSET_U8_LOAD:
            return 2;
        case scrOpcode::IOFFSET_U8_STORE:
            return 2;
        case scrOpcode::PUSH_CONST_S16:
            return 3;
        case scrOpcode::IADD_S16:
            return 3;
        case scrOpcode::IMUL_S16:
            return 3;
        case scrOpcode::IOFFSET_S16:
            return 3;
        case scrOpcode::IOFFSET_S16_LOAD:
            return 3;
        case scrOpcode::IOFFSET_S16_STORE:
            return 3;
        case scrOpcode::ARRAY_U16:
            return 3;
        case scrOpcode::ARRAY_U16_LOAD:
            return 3;
        case scrOpcode::ARRAY_U16_STORE:
            return 3;
        case scrOpcode::LOCAL_U16:
            return 3;
        case scrOpcode::LOCAL_U16_LOAD:
            return 3;
        case scrOpcode::LOCAL_U16_STORE:
            return 3;
        case scrOpcode::STATIC_U16:
            return 3;
        case scrOpcode::STATIC_U16_LOAD:
            return 3;
        case scrOpcode::STATIC_U16_STORE:
            return 3;
        case scrOpcode::GLOBAL_U16:
            return 3;
        case scrOpcode::GLOBAL_U16_LOAD:
            return 3;
        case scrOpcode::GLOBAL_U16_STORE:
            return 3;
        case scrOpcode::J:
            return 3;
        case scrOpcode::JZ:
            return 3;
        case scrOpcode::IEQ_JZ:
            return 3;
        case scrOpcode::INE_JZ:
            return 3;
        case scrOpcode::IGT_JZ:
            return 3;
        case scrOpcode::IGE_JZ:
            return 3;
        case scrOpcode::ILT_JZ:
            return 3;
        case scrOpcode::ILE_JZ:
            return 3;
        case scrOpcode::CALL:
            return 4;
        case scrOpcode::STATIC_U24:
            return 4;
        case scrOpcode::STATIC_U24_LOAD:
            return 4;
        case scrOpcode::STATIC_U24_STORE:
            return 4;
        case scrOpcode::GLOBAL_U24:
            return 4;
        case scrOpcode::GLOBAL_U24_LOAD:
            return 4;
        case scrOpcode::GLOBAL_U24_STORE:
            return 4;
        case scrOpcode::PUSH_CONST_U24:
            return 4;
        case scrOpcode::SWITCH:
            return 2 + code[1] * 6;
        case scrOpcode::STRING:
            return 1;
        case scrOpcode::STRINGHASH:
            return 1;
        case scrOpcode::TEXT_LABEL_ASSIGN_STRING:
            return 2;
        case scrOpcode::TEXT_LABEL_ASSIGN_INT:
            return 2;
        case scrOpcode::TEXT_LABEL_APPEND_STRING:
            return 2;
        case scrOpcode::TEXT_LABEL_APPEND_INT:
            return 2;
        case scrOpcode::TEXT_LABEL_COPY:
            return 1;
        case scrOpcode::CATCH:
            return 1;
        case scrOpcode::THROW:
            return 1;
        case scrOpcode::CALLINDIRECT:
            return 1;
        case scrOpcode::PUSH_CONST_M1:
            return 1;
        case scrOpcode::PUSH_CONST_0:
            return 1;
        case scrOpcode::PUSH_CONST_1:
            return 1;
        case scrOpcode::PUSH_CONST_2:
            return 1;
        case scrOpcode::PUSH_CONST_3:
            return 1;
        case scrOpcode::PUSH_CONST_4:
            return 1;
        case scrOpcode::PUSH_CONST_5:
            return 1;
        case scrOpcode::PUSH_CONST_6:
            return 1;
        case scrOpcode::PUSH_CONST_7:
            return 1;
        case scrOpcode::PUSH_CONST_FM1:
            return 1;
        case scrOpcode::PUSH_CONST_F0:
            return 1;
        case scrOpcode::PUSH_CONST_F1:
            return 1;
        case scrOpcode::PUSH_CONST_F2:
            return 1;
        case scrOpcode::PUSH_CONST_F3:
            return 1;
        case scrOpcode::PUSH_CONST_F4:
            return 1;
        case scrOpcode::PUSH_CONST_F5:
            return 1;
        case scrOpcode::PUSH_CONST_F6:
            return 1;
        case scrOpcode::PUSH_CONST_F7:
            return 1;
        case scrOpcode::IS_BIT_SET:
            return 1;
        }

        return 1;
    }
}