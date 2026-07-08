#pragma once

namespace scrDbgApp
{
    enum class OpcodesRDR3 : uint8_t
    {
        NOP = 19,
        IADD = 67,
        ISUB = 16,
        IMUL = 30,
        IDIV = 89,
        IMOD = 71,
        INOT = 5,
        INEG = 43,
        IEQ = 11,
        INE = 28,
        IGT = 32,
        IGE = 126,
        ILT = 53,
        ILE = 119,
        FADD = 113,
        FSUB = 81,
        FMUL = 36,
        FDIV = 12,
        FMOD = 27,
        FNEG = 101,
        FEQ = 125,
        FNE = 87,
        FGT = 49,
        FGE = 22,
        FLT = 52,
        FLE = 132,
        VADD = 96,
        VSUB = 25,
        VMUL = 7,
        VDIV = 18,
        VNEG = 97,
        IAND = 105,
        IOR = 48,
        IXOR = 73,
        I2F = 54,
        F2I = 69,
        F2V = 56,
        PUSH_CONST_U8 = 109,
        PUSH_CONST_U8_U8 = 111,
        PUSH_CONST_U8_U8_U8 = 123,
        PUSH_CONST_U32 = 55,
        PUSH_CONST_F = 134,
        DUP = 106,
        DROP = 65,
        NATIVE = 3,
        ENTER = 34,
        LEAVE = 80,
        LOAD = 118,
        STORE = 50,
        STORE_REV = 61,
        LOAD_N = 45,
        STORE_N = 6,
        ARRAY_U8 = 99,
        ARRAY_U8_LOAD = 23,
        ARRAY_U8_STORE = 100,
        LOCAL_U8 = 75,
        LOCAL_U8_LOAD = 102,
        LOCAL_U8_STORE = 103,
        STATIC_U8 = 137,
        STATIC_U8_LOAD = 84,
        STATIC_U8_STORE = 78,
        IADD_U8 = 92,
        IMUL_U8 = 20,
        IOFFSET = 86,
        IOFFSET_U8 = 128,
        IOFFSET_U8_LOAD = 39,
        IOFFSET_U8_STORE = 108,
        PUSH_CONST_S16 = 37,
        IADD_S16 = 59,
        IMUL_S16 = 127,
        IOFFSET_S16 = 24,
        IOFFSET_S16_LOAD = 120,
        IOFFSET_S16_STORE = 140,
        ARRAY_U16 = 64,
        ARRAY_U16_LOAD = 2,
        ARRAY_U16_STORE = 10,
        LOCAL_U16 = 88,
        LOCAL_U16_LOAD = 1,
        LOCAL_U16_STORE = 68,
        STATIC_U16 = 70,
        STATIC_U16_LOAD = 58,
        STATIC_U16_STORE = 95,
        GLOBAL_U16 = 135,
        GLOBAL_U16_LOAD = 112,
        GLOBAL_U16_STORE = 74,
        J = 104,
        JZ = 139,
        IEQ_JZ = 21,
        INE_JZ = 114,
        IGT_JZ = 46,
        IGE_JZ = 117,
        ILT_JZ = 138,
        ILE_JZ = 35,
        CALL = 57,
        GLOBAL_U24 = 93,
        GLOBAL_U24_LOAD = 133,
        GLOBAL_U24_STORE = 38,
        PUSH_CONST_U24 = 33,
        SWITCH = 60,
        STRING = 4,
        STRINGHASH = 129,
        TEXT_LABEL_ASSIGN_STRING = 31,
        TEXT_LABEL_ASSIGN_INT = 121,
        TEXT_LABEL_APPEND_STRING = 94,
        TEXT_LABEL_APPEND_INT = 41,
        TEXT_LABEL_COPY = 26,
        CATCH = 110,
        THROW = 85,
        CALLINDIRECT = 141,
        PUSH_CONST_M1 = 8,
        PUSH_CONST_0 = 47,
        PUSH_CONST_1 = 9,
        PUSH_CONST_2 = 17,
        PUSH_CONST_3 = 29,
        PUSH_CONST_4 = 66,
        PUSH_CONST_5 = 98,
        PUSH_CONST_6 = 77,
        PUSH_CONST_7 = 13,
        PUSH_CONST_FM1 = 76,
        PUSH_CONST_F0 = 115,
        PUSH_CONST_F1 = 72,
        PUSH_CONST_F2 = 91,
        PUSH_CONST_F3 = 44,
        PUSH_CONST_F4 = 90,
        PUSH_CONST_F5 = 124,
        PUSH_CONST_F6 = 122,
        PUSH_CONST_F7 = 51,
        LOCAL_LOAD_S = 15,
        LOCAL_STORE_S = 0,
        LOCAL_STORE_SR = 82,
        STATIC_LOAD_S = 14,
        STATIC_STORE_S = 79,
        STATIC_STORE_SR = 116,
        LOAD_N_S = 131,
        STORE_N_S = 83,
        STORE_N_SR = 130,
        GLOBAL_LOAD_S = 136,
        GLOBAL_STORE_S = 63,
        GLOBAL_STORE_SR = 42,
        STATIC_U24 = 62,
        STATIC_U24_LOAD = 107,
        STATIC_U24_STORE = 40
    };

    static int GetInsnSizeRDR3(const uint8_t* code, uint32_t pc)
    {
        OpcodesRDR3 op = static_cast<OpcodesRDR3>(code[pc]);

        switch (op)
        {
        case OpcodesRDR3::PUSH_CONST_U8:
        case OpcodesRDR3::ARRAY_U8:
        case OpcodesRDR3::ARRAY_U8_LOAD:
        case OpcodesRDR3::ARRAY_U8_STORE:
        case OpcodesRDR3::LOCAL_U8:
        case OpcodesRDR3::LOCAL_U8_LOAD:
        case OpcodesRDR3::LOCAL_U8_STORE:
        case OpcodesRDR3::STATIC_U8:
        case OpcodesRDR3::STATIC_U8_LOAD:
        case OpcodesRDR3::STATIC_U8_STORE:
        case OpcodesRDR3::IADD_U8:
        case OpcodesRDR3::IMUL_U8:
        case OpcodesRDR3::IOFFSET_U8:
        case OpcodesRDR3::IOFFSET_U8_LOAD:
        case OpcodesRDR3::IOFFSET_U8_STORE:
        case OpcodesRDR3::TEXT_LABEL_ASSIGN_STRING:
        case OpcodesRDR3::TEXT_LABEL_ASSIGN_INT:
        case OpcodesRDR3::TEXT_LABEL_APPEND_STRING:
        case OpcodesRDR3::TEXT_LABEL_APPEND_INT:
            return 2;
        case OpcodesRDR3::PUSH_CONST_U8_U8:
        case OpcodesRDR3::LEAVE:
        case OpcodesRDR3::PUSH_CONST_S16:
        case OpcodesRDR3::IADD_S16:
        case OpcodesRDR3::IMUL_S16:
        case OpcodesRDR3::IOFFSET_S16:
        case OpcodesRDR3::IOFFSET_S16_LOAD:
        case OpcodesRDR3::IOFFSET_S16_STORE:
        case OpcodesRDR3::ARRAY_U16:
        case OpcodesRDR3::ARRAY_U16_LOAD:
        case OpcodesRDR3::ARRAY_U16_STORE:
        case OpcodesRDR3::LOCAL_U16:
        case OpcodesRDR3::LOCAL_U16_LOAD:
        case OpcodesRDR3::LOCAL_U16_STORE:
        case OpcodesRDR3::STATIC_U16:
        case OpcodesRDR3::STATIC_U16_LOAD:
        case OpcodesRDR3::STATIC_U16_STORE:
        case OpcodesRDR3::GLOBAL_U16:
        case OpcodesRDR3::GLOBAL_U16_LOAD:
        case OpcodesRDR3::GLOBAL_U16_STORE:
        case OpcodesRDR3::J:
        case OpcodesRDR3::JZ:
        case OpcodesRDR3::IEQ_JZ:
        case OpcodesRDR3::INE_JZ:
        case OpcodesRDR3::IGT_JZ:
        case OpcodesRDR3::IGE_JZ:
        case OpcodesRDR3::ILT_JZ:
        case OpcodesRDR3::ILE_JZ:
            return 3;
        case OpcodesRDR3::PUSH_CONST_U8_U8_U8:
        case OpcodesRDR3::NATIVE:
        case OpcodesRDR3::CALL:
        case OpcodesRDR3::STATIC_U24:
        case OpcodesRDR3::STATIC_U24_LOAD:
        case OpcodesRDR3::STATIC_U24_STORE:
        case OpcodesRDR3::GLOBAL_U24:
        case OpcodesRDR3::GLOBAL_U24_LOAD:
        case OpcodesRDR3::GLOBAL_U24_STORE:
        case OpcodesRDR3::PUSH_CONST_U24:
            return 4;
        case OpcodesRDR3::PUSH_CONST_U32:
        case OpcodesRDR3::PUSH_CONST_F:
            return 5;
        case OpcodesRDR3::ENTER:
            return 5 + code[pc + 4];
        case OpcodesRDR3::SWITCH:
            return 3 + *(uint16_t*)&code[pc + 1] * 6;
        }

        return 1;
    }
}