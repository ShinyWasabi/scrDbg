#pragma once

namespace scrDbgApp
{
    enum class OpcodesRDR2 : uint8_t
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
        IOFFSET_U8_LOAD,
        IOFFSET_U8_STORE,
        IMUL_U8,
        PUSH_CONST_S16,
        IADD_S16,
        IOFFSET_S16_LOAD,
        IOFFSET_S16_STORE,
        IMUL_S16,
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
        CALL,
        CALL_U8H_1,
        CALL_U8H_2,
        CALL_U8H_3,
        CALL_U8H_4,
        CALL_U8H_5,
        CALL_U8H_6,
        CALL_U8H_7,
        CALL_U8H_8,
        CALL_U8H_9,
        CALL_U8H_A,
        CALL_U8H_B,
        CALL_U8H_C,
        CALL_U8H_D,
        CALL_U8H_E,
        CALL_U8H_F,
        J,
        JZ,
        INE_J,
        IEQ_J,
        ILE_J,
        ILT_J,
        IGE_J,
        IGT_J,
        GLOBAL_U24,
        GLOBAL_U24_LOAD,
        GLOBAL_U24_STORE,
        PUSH_CONST_U24,
        SWITCH,
        STRING,
        ARRAY,
        _NULL,
        TEXT_LABEL_ASSIGN_STRING,
        TEXT_LABEL_ASSIGN_INT,
        TEXT_LABEL_APPEND_STRING,
        TEXT_LABEL_APPEND_INT,
        TEXT_LABEL_COPY,
        CATCH,
        THROW,
        CALLINDIRECT,
        LEAVE_0_0,
        LEAVE_0_1,
        LEAVE_0_2,
        LEAVE_0_3,
        LEAVE_1_0,
        LEAVE_1_1,
        LEAVE_1_2,
        LEAVE_1_3,
        LEAVE_2_0,
        LEAVE_2_1,
        LEAVE_2_2,
        LEAVE_2_3,
        LEAVE_3_0,
        LEAVE_3_1,
        LEAVE_3_2,
        LEAVE_3_3,
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
        PATCH_RET,
        PATCH_TRAP_0,
        PATCH_TRAP_1,
        PATCH_TRAP_2,
        PATCH_TRAP_3,
        PATCH_TRAP_4,
        PATCH_TRAP_5,
        PATCH_TRAP_6,
        PATCH_TRAP_7,
        PATCH_TRAP_8,
        PATCH_TRAP_9,
        PATCH_TRAP_A,
        PATCH_TRAP_B,
        PATCH_TRAP_C,
        PATCH_TRAP_D,
        PATCH_TRAP_E,
        PATCH_TRAP_F,
        CALL_PATCH,
        CALL_OUT_OF_PATCH,
        REF_LOAD,
        REF_STORE,
        VECTOR_STORE,
        VECTOR_MAKE
    };

    static int GetInsnSizeRDR2(const uint8_t* code, uint32_t pc)
    {
        OpcodesRDR2 op = static_cast<OpcodesRDR2>(code[pc]);

        switch (op)
        {
        case OpcodesRDR2::PUSH_CONST_U8:
        case OpcodesRDR2::ARRAY_U8:
        case OpcodesRDR2::ARRAY_U8_LOAD:
        case OpcodesRDR2::ARRAY_U8_STORE:
        case OpcodesRDR2::LOCAL_U8:
        case OpcodesRDR2::LOCAL_U8_LOAD:
        case OpcodesRDR2::LOCAL_U8_STORE:
        case OpcodesRDR2::STATIC_U8:
        case OpcodesRDR2::STATIC_U8_LOAD:
        case OpcodesRDR2::STATIC_U8_STORE:
        case OpcodesRDR2::IADD_U8:
        case OpcodesRDR2::IOFFSET_U8_LOAD:
        case OpcodesRDR2::IOFFSET_U8_STORE:
        case OpcodesRDR2::IMUL_U8:
        case OpcodesRDR2::TEXT_LABEL_ASSIGN_STRING:
        case OpcodesRDR2::TEXT_LABEL_ASSIGN_INT:
        case OpcodesRDR2::TEXT_LABEL_APPEND_STRING:
        case OpcodesRDR2::TEXT_LABEL_APPEND_INT:
            return 2;
        case OpcodesRDR2::PUSH_CONST_U8_U8:
        case OpcodesRDR2::NATIVE:
        case OpcodesRDR2::LEAVE:
        case OpcodesRDR2::PUSH_CONST_S16:
        case OpcodesRDR2::IADD_S16:
        case OpcodesRDR2::IOFFSET_S16_LOAD:
        case OpcodesRDR2::IOFFSET_S16_STORE:
        case OpcodesRDR2::IMUL_S16:
        case OpcodesRDR2::ARRAY_U16:
        case OpcodesRDR2::ARRAY_U16_LOAD:
        case OpcodesRDR2::ARRAY_U16_STORE:
        case OpcodesRDR2::LOCAL_U16:
        case OpcodesRDR2::LOCAL_U16_LOAD:
        case OpcodesRDR2::LOCAL_U16_STORE:
        case OpcodesRDR2::STATIC_U16:
        case OpcodesRDR2::STATIC_U16_LOAD:
        case OpcodesRDR2::STATIC_U16_STORE:
        case OpcodesRDR2::GLOBAL_U16:
        case OpcodesRDR2::GLOBAL_U16_LOAD:
        case OpcodesRDR2::GLOBAL_U16_STORE:
        case OpcodesRDR2::CALL:
        case OpcodesRDR2::CALL_U8H_1:
        case OpcodesRDR2::CALL_U8H_2:
        case OpcodesRDR2::CALL_U8H_3:
        case OpcodesRDR2::CALL_U8H_4:
        case OpcodesRDR2::CALL_U8H_5:
        case OpcodesRDR2::CALL_U8H_6:
        case OpcodesRDR2::CALL_U8H_7:
        case OpcodesRDR2::CALL_U8H_8:
        case OpcodesRDR2::CALL_U8H_9:
        case OpcodesRDR2::CALL_U8H_A:
        case OpcodesRDR2::CALL_U8H_B:
        case OpcodesRDR2::CALL_U8H_C:
        case OpcodesRDR2::CALL_U8H_D:
        case OpcodesRDR2::CALL_U8H_E:
        case OpcodesRDR2::CALL_U8H_F:
        case OpcodesRDR2::J:
        case OpcodesRDR2::JZ:
        case OpcodesRDR2::INE_J:
        case OpcodesRDR2::IEQ_J:
        case OpcodesRDR2::ILE_J:
        case OpcodesRDR2::ILT_J:
        case OpcodesRDR2::IGE_J:
        case OpcodesRDR2::IGT_J:
            return 3;
        case OpcodesRDR2::PUSH_CONST_U8_U8_U8:
        case OpcodesRDR2::GLOBAL_U24:
        case OpcodesRDR2::GLOBAL_U24_LOAD:
        case OpcodesRDR2::GLOBAL_U24_STORE:
        case OpcodesRDR2::PUSH_CONST_U24:
        case OpcodesRDR2::CALL_PATCH:
        case OpcodesRDR2::CALL_OUT_OF_PATCH:
            return 4;
        case OpcodesRDR2::PUSH_CONST_U32:
        case OpcodesRDR2::PUSH_CONST_F:
            return 5;
        case OpcodesRDR2::ENTER:
            return 5 + code[pc + 4];
        case OpcodesRDR2::SWITCH:
            return 2 + code[pc + 1] * 6;
        case OpcodesRDR2::STRING:
            return 2 + code[pc + 1];
        case OpcodesRDR2::ARRAY:
            return 5 + *(uint32_t*)&code[pc + 1];
        }

        return 1;
    }
}