#pragma once
#include "Opcodes.hpp"

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

    static inline constexpr uint16_t g_OpcodesRDR3Map[] = {
        static_cast<uint16_t>(OpcodesRDR3::NOP),
        static_cast<uint16_t>(OpcodesRDR3::IADD),
        static_cast<uint16_t>(OpcodesRDR3::ISUB),
        static_cast<uint16_t>(OpcodesRDR3::IMUL),
        static_cast<uint16_t>(OpcodesRDR3::IDIV),
        static_cast<uint16_t>(OpcodesRDR3::IMOD),
        static_cast<uint16_t>(OpcodesRDR3::INOT),
        static_cast<uint16_t>(OpcodesRDR3::INEG),
        static_cast<uint16_t>(OpcodesRDR3::IEQ),
        static_cast<uint16_t>(OpcodesRDR3::INE),
        static_cast<uint16_t>(OpcodesRDR3::IGT),
        static_cast<uint16_t>(OpcodesRDR3::IGE),
        static_cast<uint16_t>(OpcodesRDR3::ILT),
        static_cast<uint16_t>(OpcodesRDR3::ILE),
        static_cast<uint16_t>(OpcodesRDR3::FADD),
        static_cast<uint16_t>(OpcodesRDR3::FSUB),
        static_cast<uint16_t>(OpcodesRDR3::FMUL),
        static_cast<uint16_t>(OpcodesRDR3::FDIV),
        static_cast<uint16_t>(OpcodesRDR3::FMOD),
        static_cast<uint16_t>(OpcodesRDR3::FNEG),
        static_cast<uint16_t>(OpcodesRDR3::FEQ),
        static_cast<uint16_t>(OpcodesRDR3::FNE),
        static_cast<uint16_t>(OpcodesRDR3::FGT),
        static_cast<uint16_t>(OpcodesRDR3::FGE),
        static_cast<uint16_t>(OpcodesRDR3::FLT),
        static_cast<uint16_t>(OpcodesRDR3::FLE),
        static_cast<uint16_t>(OpcodesRDR3::VADD),
        static_cast<uint16_t>(OpcodesRDR3::VSUB),
        static_cast<uint16_t>(OpcodesRDR3::VMUL),
        static_cast<uint16_t>(OpcodesRDR3::VDIV),
        static_cast<uint16_t>(OpcodesRDR3::VNEG),
        static_cast<uint16_t>(OpcodesRDR3::IAND),
        static_cast<uint16_t>(OpcodesRDR3::IOR),
        static_cast<uint16_t>(OpcodesRDR3::IXOR),
        static_cast<uint16_t>(OpcodesRDR3::J),
        static_cast<uint16_t>(OpcodesRDR3::JZ),
        static_cast<uint16_t>(OpcodesRDR3::I2F),
        static_cast<uint16_t>(OpcodesRDR3::F2I),
        static_cast<uint16_t>(OpcodesRDR3::F2V),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_S16),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_U32),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_F),
        static_cast<uint16_t>(OpcodesRDR3::DUP),
        static_cast<uint16_t>(OpcodesRDR3::DROP),
        static_cast<uint16_t>(OpcodesRDR3::NATIVE),
        static_cast<uint16_t>(OpcodesRDR3::CALL),
        static_cast<uint16_t>(OpcodesRDR3::ENTER),
        static_cast<uint16_t>(OpcodesRDR3::LEAVE),
        static_cast<uint16_t>(OpcodesRDR3::LOAD),
        static_cast<uint16_t>(OpcodesRDR3::STORE),
        static_cast<uint16_t>(OpcodesRDR3::STORE_REV),
        static_cast<uint16_t>(OpcodesRDR3::SWITCH),
        static_cast<uint16_t>(OpcodesRDR3::STRING),
        static_cast<uint16_t>(OpcodesRDR3::TEXT_LABEL_ASSIGN_STRING),
        static_cast<uint16_t>(OpcodesRDR3::TEXT_LABEL_ASSIGN_INT),
        static_cast<uint16_t>(OpcodesRDR3::TEXT_LABEL_APPEND_STRING),
        static_cast<uint16_t>(OpcodesRDR3::TEXT_LABEL_APPEND_INT),
        static_cast<uint16_t>(OpcodesRDR3::CATCH),
        static_cast<uint16_t>(OpcodesRDR3::THROW),
        static_cast<uint16_t>(OpcodesRDR3::TEXT_LABEL_COPY),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_M1),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_0),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_1),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_2),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_3),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_4),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_5),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_6),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_7),
        UNAVAILABLE_OP, // JNZ
        UNAVAILABLE_OP, // LOCAL_0
        UNAVAILABLE_OP, // LOCAL_1
        UNAVAILABLE_OP, // LOCAL_2
        UNAVAILABLE_OP, // LOCAL_3
        UNAVAILABLE_OP, // LOCAL_4
        UNAVAILABLE_OP, // LOCAL_5
        UNAVAILABLE_OP, // LOCAL_6
        UNAVAILABLE_OP, // LOCAL_7
        UNAVAILABLE_OP, // LOCAL
        UNAVAILABLE_OP, // STATIC
        UNAVAILABLE_OP, // GLOBAL
        UNAVAILABLE_OP, // PUSH_CONST_M16
        UNAVAILABLE_OP, // PUSH_CONST_M15
        UNAVAILABLE_OP, // PUSH_CONST_M14
        UNAVAILABLE_OP, // PUSH_CONST_M13
        UNAVAILABLE_OP, // PUSH_CONST_M12
        UNAVAILABLE_OP, // PUSH_CONST_M11
        UNAVAILABLE_OP, // PUSH_CONST_M10
        UNAVAILABLE_OP, // PUSH_CONST_M9
        UNAVAILABLE_OP, // PUSH_CONST_M8
        UNAVAILABLE_OP, // PUSH_CONST_M7
        UNAVAILABLE_OP, // PUSH_CONST_M6
        UNAVAILABLE_OP, // PUSH_CONST_M5
        UNAVAILABLE_OP, // PUSH_CONST_M4
        UNAVAILABLE_OP, // PUSH_CONST_M3
        UNAVAILABLE_OP, // PUSH_CONST_M2
        UNAVAILABLE_OP, // PUSH_CONST_8
        UNAVAILABLE_OP, // PUSH_CONST_9
        UNAVAILABLE_OP, // PUSH_CONST_10
        UNAVAILABLE_OP, // PUSH_CONST_11
        UNAVAILABLE_OP, // PUSH_CONST_12
        UNAVAILABLE_OP, // PUSH_CONST_13
        UNAVAILABLE_OP, // PUSH_CONST_14
        UNAVAILABLE_OP, // PUSH_CONST_15
        UNAVAILABLE_OP, // PUSH_CONST_16
        UNAVAILABLE_OP, // PUSH_CONST_17
        UNAVAILABLE_OP, // PUSH_CONST_18
        UNAVAILABLE_OP, // PUSH_CONST_19
        UNAVAILABLE_OP, // PUSH_CONST_20
        UNAVAILABLE_OP, // PUSH_CONST_21
        UNAVAILABLE_OP, // PUSH_CONST_22
        UNAVAILABLE_OP, // PUSH_CONST_23
        UNAVAILABLE_OP, // PUSH_CONST_24
        UNAVAILABLE_OP, // PUSH_CONST_25
        UNAVAILABLE_OP, // PUSH_CONST_26
        UNAVAILABLE_OP, // PUSH_CONST_27
        UNAVAILABLE_OP, // PUSH_CONST_28
        UNAVAILABLE_OP, // PUSH_CONST_29
        UNAVAILABLE_OP, // PUSH_CONST_30
        UNAVAILABLE_OP, // PUSH_CONST_31
        UNAVAILABLE_OP, // PUSH_CONST_32
        UNAVAILABLE_OP, // PUSH_CONST_33
        UNAVAILABLE_OP, // PUSH_CONST_34
        UNAVAILABLE_OP, // PUSH_CONST_35
        UNAVAILABLE_OP, // PUSH_CONST_36
        UNAVAILABLE_OP, // PUSH_CONST_37
        UNAVAILABLE_OP, // PUSH_CONST_38
        UNAVAILABLE_OP, // PUSH_CONST_39
        UNAVAILABLE_OP, // PUSH_CONST_40
        UNAVAILABLE_OP, // PUSH_CONST_41
        UNAVAILABLE_OP, // PUSH_CONST_42
        UNAVAILABLE_OP, // PUSH_CONST_43
        UNAVAILABLE_OP, // PUSH_CONST_44
        UNAVAILABLE_OP, // PUSH_CONST_45
        UNAVAILABLE_OP, // PUSH_CONST_46
        UNAVAILABLE_OP, // PUSH_CONST_47
        UNAVAILABLE_OP, // PUSH_CONST_48
        UNAVAILABLE_OP, // PUSH_CONST_49
        UNAVAILABLE_OP, // PUSH_CONST_50
        UNAVAILABLE_OP, // PUSH_CONST_51
        UNAVAILABLE_OP, // PUSH_CONST_52
        UNAVAILABLE_OP, // PUSH_CONST_53
        UNAVAILABLE_OP, // PUSH_CONST_54
        UNAVAILABLE_OP, // PUSH_CONST_55
        UNAVAILABLE_OP, // PUSH_CONST_56
        UNAVAILABLE_OP, // PUSH_CONST_57
        UNAVAILABLE_OP, // PUSH_CONST_58
        UNAVAILABLE_OP, // PUSH_CONST_59
        UNAVAILABLE_OP, // PUSH_CONST_60
        UNAVAILABLE_OP, // PUSH_CONST_61
        UNAVAILABLE_OP, // PUSH_CONST_62
        UNAVAILABLE_OP, // PUSH_CONST_63
        UNAVAILABLE_OP, // PUSH_CONST_64
        UNAVAILABLE_OP, // PUSH_CONST_65
        UNAVAILABLE_OP, // PUSH_CONST_66
        UNAVAILABLE_OP, // PUSH_CONST_67
        UNAVAILABLE_OP, // PUSH_CONST_68
        UNAVAILABLE_OP, // PUSH_CONST_69
        UNAVAILABLE_OP, // PUSH_CONST_70
        UNAVAILABLE_OP, // PUSH_CONST_71
        UNAVAILABLE_OP, // PUSH_CONST_72
        UNAVAILABLE_OP, // PUSH_CONST_73
        UNAVAILABLE_OP, // PUSH_CONST_74
        UNAVAILABLE_OP, // PUSH_CONST_75
        UNAVAILABLE_OP, // PUSH_CONST_76
        UNAVAILABLE_OP, // PUSH_CONST_77
        UNAVAILABLE_OP, // PUSH_CONST_78
        UNAVAILABLE_OP, // PUSH_CONST_79
        UNAVAILABLE_OP, // PUSH_CONST_80
        UNAVAILABLE_OP, // PUSH_CONST_81
        UNAVAILABLE_OP, // PUSH_CONST_82
        UNAVAILABLE_OP, // PUSH_CONST_83
        UNAVAILABLE_OP, // PUSH_CONST_84
        UNAVAILABLE_OP, // PUSH_CONST_85
        UNAVAILABLE_OP, // PUSH_CONST_86
        UNAVAILABLE_OP, // PUSH_CONST_87
        UNAVAILABLE_OP, // PUSH_CONST_88
        UNAVAILABLE_OP, // PUSH_CONST_89
        UNAVAILABLE_OP, // PUSH_CONST_90
        UNAVAILABLE_OP, // PUSH_CONST_91
        UNAVAILABLE_OP, // PUSH_CONST_92
        UNAVAILABLE_OP, // PUSH_CONST_93
        UNAVAILABLE_OP, // PUSH_CONST_94
        UNAVAILABLE_OP, // PUSH_CONST_95
        UNAVAILABLE_OP, // PUSH_CONST_96
        UNAVAILABLE_OP, // PUSH_CONST_97
        UNAVAILABLE_OP, // PUSH_CONST_98
        UNAVAILABLE_OP, // PUSH_CONST_99
        UNAVAILABLE_OP, // PUSH_CONST_100
        UNAVAILABLE_OP, // PUSH_CONST_101
        UNAVAILABLE_OP, // PUSH_CONST_102
        UNAVAILABLE_OP, // PUSH_CONST_103
        UNAVAILABLE_OP, // PUSH_CONST_104
        UNAVAILABLE_OP, // PUSH_CONST_105
        UNAVAILABLE_OP, // PUSH_CONST_106
        UNAVAILABLE_OP, // PUSH_CONST_107
        UNAVAILABLE_OP, // PUSH_CONST_108
        UNAVAILABLE_OP, // PUSH_CONST_109
        UNAVAILABLE_OP, // PUSH_CONST_110
        UNAVAILABLE_OP, // PUSH_CONST_111
        UNAVAILABLE_OP, // PUSH_CONST_112
        UNAVAILABLE_OP, // PUSH_CONST_113
        UNAVAILABLE_OP, // PUSH_CONST_114
        UNAVAILABLE_OP, // PUSH_CONST_115
        UNAVAILABLE_OP, // PUSH_CONST_116
        UNAVAILABLE_OP, // PUSH_CONST_117
        UNAVAILABLE_OP, // PUSH_CONST_118
        UNAVAILABLE_OP, // PUSH_CONST_119
        UNAVAILABLE_OP, // PUSH_CONST_120
        UNAVAILABLE_OP, // PUSH_CONST_121
        UNAVAILABLE_OP, // PUSH_CONST_122
        UNAVAILABLE_OP, // PUSH_CONST_123
        UNAVAILABLE_OP, // PUSH_CONST_124
        UNAVAILABLE_OP, // PUSH_CONST_125
        UNAVAILABLE_OP, // PUSH_CONST_126
        UNAVAILABLE_OP, // PUSH_CONST_127
        UNAVAILABLE_OP, // PUSH_CONST_128
        UNAVAILABLE_OP, // PUSH_CONST_129
        UNAVAILABLE_OP, // PUSH_CONST_130
        UNAVAILABLE_OP, // PUSH_CONST_131
        UNAVAILABLE_OP, // PUSH_CONST_132
        UNAVAILABLE_OP, // PUSH_CONST_133
        UNAVAILABLE_OP, // PUSH_CONST_134
        UNAVAILABLE_OP, // PUSH_CONST_135
        UNAVAILABLE_OP, // PUSH_CONST_136
        UNAVAILABLE_OP, // PUSH_CONST_137
        UNAVAILABLE_OP, // PUSH_CONST_138
        UNAVAILABLE_OP, // PUSH_CONST_139
        UNAVAILABLE_OP, // PUSH_CONST_140
        UNAVAILABLE_OP, // PUSH_CONST_141
        UNAVAILABLE_OP, // PUSH_CONST_142
        UNAVAILABLE_OP, // PUSH_CONST_143
        UNAVAILABLE_OP, // PUSH_CONST_144
        UNAVAILABLE_OP, // PUSH_CONST_145
        UNAVAILABLE_OP, // PUSH_CONST_146
        UNAVAILABLE_OP, // PUSH_CONST_147
        UNAVAILABLE_OP, // PUSH_CONST_148
        UNAVAILABLE_OP, // PUSH_CONST_149
        UNAVAILABLE_OP, // PUSH_CONST_150
        UNAVAILABLE_OP, // PUSH_CONST_151
        UNAVAILABLE_OP, // PUSH_CONST_152
        UNAVAILABLE_OP, // PUSH_CONST_153
        UNAVAILABLE_OP, // PUSH_CONST_154
        UNAVAILABLE_OP, // PUSH_CONST_155
        UNAVAILABLE_OP, // PUSH_CONST_156
        UNAVAILABLE_OP, // PUSH_CONST_157
        UNAVAILABLE_OP, // PUSH_CONST_158
        UNAVAILABLE_OP, // PUSH_CONST_159
        UNAVAILABLE_OP, // ARRAY_LOAD
        UNAVAILABLE_OP, // ARRAY_STORE
        UNAVAILABLE_OP, // GETXPROTECT
        UNAVAILABLE_OP, // SETXPROTECT
        UNAVAILABLE_OP, // REFXPROTECT
        UNAVAILABLE_OP, // EXIT
        UNAVAILABLE_OP, // ARRAY
        UNAVAILABLE_OP, // _NULL
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_U8),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_U8_U8),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_U8_U8_U8),
        static_cast<uint16_t>(OpcodesRDR3::ARRAY_U8),
        static_cast<uint16_t>(OpcodesRDR3::ARRAY_U8_LOAD),
        static_cast<uint16_t>(OpcodesRDR3::ARRAY_U8_STORE),
        static_cast<uint16_t>(OpcodesRDR3::LOCAL_U8),
        static_cast<uint16_t>(OpcodesRDR3::LOCAL_U8_LOAD),
        static_cast<uint16_t>(OpcodesRDR3::LOCAL_U8_STORE),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_U8),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_U8_LOAD),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_U8_STORE),
        static_cast<uint16_t>(OpcodesRDR3::IADD_U8),
        static_cast<uint16_t>(OpcodesRDR3::IMUL_U8),
        static_cast<uint16_t>(OpcodesRDR3::IOFFSET_U8_LOAD),
        static_cast<uint16_t>(OpcodesRDR3::IOFFSET_U8_STORE),
        static_cast<uint16_t>(OpcodesRDR3::IADD_S16),
        static_cast<uint16_t>(OpcodesRDR3::IMUL_S16),
        static_cast<uint16_t>(OpcodesRDR3::IOFFSET_S16_LOAD),
        static_cast<uint16_t>(OpcodesRDR3::IOFFSET_S16_STORE),
        static_cast<uint16_t>(OpcodesRDR3::ARRAY_U16),
        static_cast<uint16_t>(OpcodesRDR3::ARRAY_U16_LOAD),
        static_cast<uint16_t>(OpcodesRDR3::ARRAY_U16_STORE),
        static_cast<uint16_t>(OpcodesRDR3::LOCAL_U16),
        static_cast<uint16_t>(OpcodesRDR3::LOCAL_U16_LOAD),
        static_cast<uint16_t>(OpcodesRDR3::LOCAL_U16_STORE),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_U16),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_U16_LOAD),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_U16_STORE),
        static_cast<uint16_t>(OpcodesRDR3::GLOBAL_U16),
        static_cast<uint16_t>(OpcodesRDR3::GLOBAL_U16_LOAD),
        static_cast<uint16_t>(OpcodesRDR3::GLOBAL_U16_STORE),
        static_cast<uint16_t>(OpcodesRDR3::GLOBAL_U24),
        static_cast<uint16_t>(OpcodesRDR3::GLOBAL_U24_LOAD),
        static_cast<uint16_t>(OpcodesRDR3::GLOBAL_U24_STORE),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_U24),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_FM1),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_F0),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_F1),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_F2),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_F3),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_F4),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_F5),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_F6),
        static_cast<uint16_t>(OpcodesRDR3::PUSH_CONST_F7),
        static_cast<uint16_t>(OpcodesRDR3::LOAD_N),
        static_cast<uint16_t>(OpcodesRDR3::STORE_N),
        static_cast<uint16_t>(OpcodesRDR3::CALLINDIRECT),
        static_cast<uint16_t>(OpcodesRDR3::IOFFSET),
        static_cast<uint16_t>(OpcodesRDR3::IOFFSET_U8),
        static_cast<uint16_t>(OpcodesRDR3::IOFFSET_S16),
        static_cast<uint16_t>(OpcodesRDR3::IEQ_JZ),
        static_cast<uint16_t>(OpcodesRDR3::INE_JZ),
        static_cast<uint16_t>(OpcodesRDR3::IGT_JZ),
        static_cast<uint16_t>(OpcodesRDR3::IGE_JZ),
        static_cast<uint16_t>(OpcodesRDR3::ILT_JZ),
        static_cast<uint16_t>(OpcodesRDR3::ILE_JZ),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_U24),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_U24_LOAD),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_U24_STORE),
        static_cast<uint16_t>(OpcodesRDR3::STRINGHASH),
        UNAVAILABLE_OP, // IS_BIT_SET
        UNAVAILABLE_OP, // INVALID_OP_77
        UNAVAILABLE_OP, // INVALID_OP_78
        UNAVAILABLE_OP, // INVALID_OP_79
        UNAVAILABLE_OP, // CALL_U8H_1
        UNAVAILABLE_OP, // CALL_U8H_2
        UNAVAILABLE_OP, // CALL_U8H_3
        UNAVAILABLE_OP, // CALL_U8H_4
        UNAVAILABLE_OP, // CALL_U8H_5
        UNAVAILABLE_OP, // CALL_U8H_6
        UNAVAILABLE_OP, // CALL_U8H_7
        UNAVAILABLE_OP, // CALL_U8H_8
        UNAVAILABLE_OP, // CALL_U8H_9
        UNAVAILABLE_OP, // CALL_U8H_A
        UNAVAILABLE_OP, // CALL_U8H_B
        UNAVAILABLE_OP, // CALL_U8H_C
        UNAVAILABLE_OP, // CALL_U8H_D
        UNAVAILABLE_OP, // CALL_U8H_E
        UNAVAILABLE_OP, // CALL_U8H_F
        UNAVAILABLE_OP, // INE_J
        UNAVAILABLE_OP, // IEQ_J
        UNAVAILABLE_OP, // ILE_J
        UNAVAILABLE_OP, // ILT_J
        UNAVAILABLE_OP, // IGE_J
        UNAVAILABLE_OP, // IGT_J
        UNAVAILABLE_OP, // LEAVE_0_0
        UNAVAILABLE_OP, // LEAVE_0_1
        UNAVAILABLE_OP, // LEAVE_0_2
        UNAVAILABLE_OP, // LEAVE_0_3
        UNAVAILABLE_OP, // LEAVE_1_0
        UNAVAILABLE_OP, // LEAVE_1_1
        UNAVAILABLE_OP, // LEAVE_1_2
        UNAVAILABLE_OP, // LEAVE_1_3
        UNAVAILABLE_OP, // LEAVE_2_0
        UNAVAILABLE_OP, // LEAVE_2_1
        UNAVAILABLE_OP, // LEAVE_2_2
        UNAVAILABLE_OP, // LEAVE_2_3
        UNAVAILABLE_OP, // LEAVE_3_0
        UNAVAILABLE_OP, // LEAVE_3_1
        UNAVAILABLE_OP, // LEAVE_3_2
        UNAVAILABLE_OP, // LEAVE_3_3
        UNAVAILABLE_OP, // PATCH_RET
        UNAVAILABLE_OP, // PATCH_TRAP_0
        UNAVAILABLE_OP, // PATCH_TRAP_1
        UNAVAILABLE_OP, // PATCH_TRAP_2
        UNAVAILABLE_OP, // PATCH_TRAP_3
        UNAVAILABLE_OP, // PATCH_TRAP_4
        UNAVAILABLE_OP, // PATCH_TRAP_5
        UNAVAILABLE_OP, // PATCH_TRAP_6
        UNAVAILABLE_OP, // PATCH_TRAP_7
        UNAVAILABLE_OP, // PATCH_TRAP_8
        UNAVAILABLE_OP, // PATCH_TRAP_9
        UNAVAILABLE_OP, // PATCH_TRAP_A
        UNAVAILABLE_OP, // PATCH_TRAP_B
        UNAVAILABLE_OP, // PATCH_TRAP_C
        UNAVAILABLE_OP, // PATCH_TRAP_D
        UNAVAILABLE_OP, // PATCH_TRAP_E
        UNAVAILABLE_OP, // PATCH_TRAP_F
        UNAVAILABLE_OP, // CALL_PATCH
        UNAVAILABLE_OP, // CALL_OUT_OF_PATCH
        UNAVAILABLE_OP, // REF_LOAD
        UNAVAILABLE_OP, // REF_STORE
        UNAVAILABLE_OP, // VECTOR_STORE
        UNAVAILABLE_OP, // VECTOR_MAKE
        static_cast<uint16_t>(OpcodesRDR3::LOCAL_LOAD_S),
        static_cast<uint16_t>(OpcodesRDR3::LOCAL_STORE_S),
        static_cast<uint16_t>(OpcodesRDR3::LOCAL_STORE_SR),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_LOAD_S),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_STORE_S),
        static_cast<uint16_t>(OpcodesRDR3::STATIC_STORE_SR),
        static_cast<uint16_t>(OpcodesRDR3::LOAD_N_S),
        static_cast<uint16_t>(OpcodesRDR3::GLOBAL_STORE_S),
        static_cast<uint16_t>(OpcodesRDR3::GLOBAL_STORE_SR),
        static_cast<uint16_t>(OpcodesRDR3::GLOBAL_LOAD_S),
        static_cast<uint16_t>(OpcodesRDR3::STORE_N_S),
        static_cast<uint16_t>(OpcodesRDR3::STORE_N_SR),
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