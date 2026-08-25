#pragma once
#include "Opcodes.hpp"

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

    static inline constexpr uint16_t g_OpcodesRDR2Map[] = {
        static_cast<uint16_t>(OpcodesRDR2::NOP),
        static_cast<uint16_t>(OpcodesRDR2::IADD),
        static_cast<uint16_t>(OpcodesRDR2::ISUB),
        static_cast<uint16_t>(OpcodesRDR2::IMUL),
        static_cast<uint16_t>(OpcodesRDR2::IDIV),
        static_cast<uint16_t>(OpcodesRDR2::IMOD),
        static_cast<uint16_t>(OpcodesRDR2::INOT),
        static_cast<uint16_t>(OpcodesRDR2::INEG),
        static_cast<uint16_t>(OpcodesRDR2::IEQ),
        static_cast<uint16_t>(OpcodesRDR2::INE),
        static_cast<uint16_t>(OpcodesRDR2::IGT),
        static_cast<uint16_t>(OpcodesRDR2::IGE),
        static_cast<uint16_t>(OpcodesRDR2::ILT),
        static_cast<uint16_t>(OpcodesRDR2::ILE),
        static_cast<uint16_t>(OpcodesRDR2::FADD),
        static_cast<uint16_t>(OpcodesRDR2::FSUB),
        static_cast<uint16_t>(OpcodesRDR2::FMUL),
        static_cast<uint16_t>(OpcodesRDR2::FDIV),
        static_cast<uint16_t>(OpcodesRDR2::FMOD),
        static_cast<uint16_t>(OpcodesRDR2::FNEG),
        static_cast<uint16_t>(OpcodesRDR2::FEQ),
        static_cast<uint16_t>(OpcodesRDR2::FNE),
        static_cast<uint16_t>(OpcodesRDR2::FGT),
        static_cast<uint16_t>(OpcodesRDR2::FGE),
        static_cast<uint16_t>(OpcodesRDR2::FLT),
        static_cast<uint16_t>(OpcodesRDR2::FLE),
        static_cast<uint16_t>(OpcodesRDR2::VADD),
        static_cast<uint16_t>(OpcodesRDR2::VSUB),
        static_cast<uint16_t>(OpcodesRDR2::VMUL),
        static_cast<uint16_t>(OpcodesRDR2::VDIV),
        static_cast<uint16_t>(OpcodesRDR2::VNEG),
        static_cast<uint16_t>(OpcodesRDR2::IAND),
        static_cast<uint16_t>(OpcodesRDR2::IOR),
        static_cast<uint16_t>(OpcodesRDR2::IXOR),
        static_cast<uint16_t>(OpcodesRDR2::J),
        static_cast<uint16_t>(OpcodesRDR2::JZ),
        static_cast<uint16_t>(OpcodesRDR2::I2F),
        static_cast<uint16_t>(OpcodesRDR2::F2I),
        static_cast<uint16_t>(OpcodesRDR2::F2V),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_S16),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_U32),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_F),
        static_cast<uint16_t>(OpcodesRDR2::DUP),
        static_cast<uint16_t>(OpcodesRDR2::DROP),
        static_cast<uint16_t>(OpcodesRDR2::NATIVE),
        static_cast<uint16_t>(OpcodesRDR2::CALL),
        static_cast<uint16_t>(OpcodesRDR2::ENTER),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE),
        static_cast<uint16_t>(OpcodesRDR2::LOAD),
        static_cast<uint16_t>(OpcodesRDR2::STORE),
        static_cast<uint16_t>(OpcodesRDR2::STORE_REV),
        static_cast<uint16_t>(OpcodesRDR2::SWITCH),
        static_cast<uint16_t>(OpcodesRDR2::STRING),
        static_cast<uint16_t>(OpcodesRDR2::TEXT_LABEL_ASSIGN_STRING),
        static_cast<uint16_t>(OpcodesRDR2::TEXT_LABEL_ASSIGN_INT),
        static_cast<uint16_t>(OpcodesRDR2::TEXT_LABEL_APPEND_STRING),
        static_cast<uint16_t>(OpcodesRDR2::TEXT_LABEL_APPEND_INT),
        static_cast<uint16_t>(OpcodesRDR2::CATCH),
        static_cast<uint16_t>(OpcodesRDR2::THROW),
        static_cast<uint16_t>(OpcodesRDR2::TEXT_LABEL_COPY),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_M1),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_0),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_1),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_2),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_3),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_4),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_5),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_6),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_7),
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
        static_cast<uint16_t>(OpcodesRDR2::ARRAY),
        static_cast<uint16_t>(OpcodesRDR2::_NULL),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_U8),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_U8_U8),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_U8_U8_U8),
        static_cast<uint16_t>(OpcodesRDR2::ARRAY_U8),
        static_cast<uint16_t>(OpcodesRDR2::ARRAY_U8_LOAD),
        static_cast<uint16_t>(OpcodesRDR2::ARRAY_U8_STORE),
        static_cast<uint16_t>(OpcodesRDR2::LOCAL_U8),
        static_cast<uint16_t>(OpcodesRDR2::LOCAL_U8_LOAD),
        static_cast<uint16_t>(OpcodesRDR2::LOCAL_U8_STORE),
        static_cast<uint16_t>(OpcodesRDR2::STATIC_U8),
        static_cast<uint16_t>(OpcodesRDR2::STATIC_U8_LOAD),
        static_cast<uint16_t>(OpcodesRDR2::STATIC_U8_STORE),
        static_cast<uint16_t>(OpcodesRDR2::IADD_U8),
        static_cast<uint16_t>(OpcodesRDR2::IMUL_U8),
        static_cast<uint16_t>(OpcodesRDR2::IOFFSET_U8_LOAD),
        static_cast<uint16_t>(OpcodesRDR2::IOFFSET_U8_STORE),
        static_cast<uint16_t>(OpcodesRDR2::IADD_S16),
        static_cast<uint16_t>(OpcodesRDR2::IMUL_S16),
        static_cast<uint16_t>(OpcodesRDR2::IOFFSET_S16_LOAD),
        static_cast<uint16_t>(OpcodesRDR2::IOFFSET_S16_STORE),
        static_cast<uint16_t>(OpcodesRDR2::ARRAY_U16),
        static_cast<uint16_t>(OpcodesRDR2::ARRAY_U16_LOAD),
        static_cast<uint16_t>(OpcodesRDR2::ARRAY_U16_STORE),
        static_cast<uint16_t>(OpcodesRDR2::LOCAL_U16),
        static_cast<uint16_t>(OpcodesRDR2::LOCAL_U16_LOAD),
        static_cast<uint16_t>(OpcodesRDR2::LOCAL_U16_STORE),
        static_cast<uint16_t>(OpcodesRDR2::STATIC_U16),
        static_cast<uint16_t>(OpcodesRDR2::STATIC_U16_LOAD),
        static_cast<uint16_t>(OpcodesRDR2::STATIC_U16_STORE),
        static_cast<uint16_t>(OpcodesRDR2::GLOBAL_U16),
        static_cast<uint16_t>(OpcodesRDR2::GLOBAL_U16_LOAD),
        static_cast<uint16_t>(OpcodesRDR2::GLOBAL_U16_STORE),
        static_cast<uint16_t>(OpcodesRDR2::GLOBAL_U24),
        static_cast<uint16_t>(OpcodesRDR2::GLOBAL_U24_LOAD),
        static_cast<uint16_t>(OpcodesRDR2::GLOBAL_U24_STORE),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_U24),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_FM1),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_F0),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_F1),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_F2),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_F3),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_F4),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_F5),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_F6),
        static_cast<uint16_t>(OpcodesRDR2::PUSH_CONST_F7),
        static_cast<uint16_t>(OpcodesRDR2::LOAD_N),
        static_cast<uint16_t>(OpcodesRDR2::STORE_N),
        static_cast<uint16_t>(OpcodesRDR2::CALLINDIRECT),
        UNAVAILABLE_OP, // IOFFSET
        UNAVAILABLE_OP, // IOFFSET_U8
        UNAVAILABLE_OP, // IOFFSET_S16
        UNAVAILABLE_OP, // IEQ_JZ
        UNAVAILABLE_OP, // INE_JZ
        UNAVAILABLE_OP, // IGT_JZ
        UNAVAILABLE_OP, // IGE_JZ
        UNAVAILABLE_OP, // ILT_JZ
        UNAVAILABLE_OP, // ILE_JZ
        UNAVAILABLE_OP, // STATIC_U24
        UNAVAILABLE_OP, // STATIC_U24_LOAD
        UNAVAILABLE_OP, // STATIC_U24_STORE
        UNAVAILABLE_OP, // STRINGHASH
        UNAVAILABLE_OP, // IS_BIT_SET
        UNAVAILABLE_OP, // INVALID_OP_77
        UNAVAILABLE_OP, // INVALID_OP_78
        UNAVAILABLE_OP, // INVALID_OP_79
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_1),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_2),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_3),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_4),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_5),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_6),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_7),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_8),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_9),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_A),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_B),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_C),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_D),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_E),
        static_cast<uint16_t>(OpcodesRDR2::CALL_U8H_F),
        static_cast<uint16_t>(OpcodesRDR2::INE_J),
        static_cast<uint16_t>(OpcodesRDR2::IEQ_J),
        static_cast<uint16_t>(OpcodesRDR2::ILE_J),
        static_cast<uint16_t>(OpcodesRDR2::ILT_J),
        static_cast<uint16_t>(OpcodesRDR2::IGE_J),
        static_cast<uint16_t>(OpcodesRDR2::IGT_J),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_0_0),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_0_1),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_0_2),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_0_3),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_1_0),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_1_1),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_1_2),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_1_3),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_2_0),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_2_1),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_2_2),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_2_3),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_3_0),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_3_1),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_3_2),
        static_cast<uint16_t>(OpcodesRDR2::LEAVE_3_3),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_RET),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_0),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_1),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_2),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_3),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_4),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_5),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_6),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_7),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_8),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_9),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_A),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_B),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_C),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_D),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_E),
        static_cast<uint16_t>(OpcodesRDR2::PATCH_TRAP_F),
        static_cast<uint16_t>(OpcodesRDR2::CALL_PATCH),
        static_cast<uint16_t>(OpcodesRDR2::CALL_OUT_OF_PATCH),
        static_cast<uint16_t>(OpcodesRDR2::REF_LOAD),
        static_cast<uint16_t>(OpcodesRDR2::REF_STORE),
        static_cast<uint16_t>(OpcodesRDR2::VECTOR_STORE),
        static_cast<uint16_t>(OpcodesRDR2::VECTOR_MAKE),
        UNAVAILABLE_OP, // LOCAL_LOAD_S
        UNAVAILABLE_OP, // LOCAL_STORE_S
        UNAVAILABLE_OP, // LOCAL_STORE_SR
        UNAVAILABLE_OP, // STATIC_LOAD_S
        UNAVAILABLE_OP, // STATIC_STORE_S
        UNAVAILABLE_OP, // STATIC_STORE_SR
        UNAVAILABLE_OP, // LOAD_N_S
        UNAVAILABLE_OP, // GLOBAL_STORE_S
        UNAVAILABLE_OP, // GLOBAL_STORE_SR
        UNAVAILABLE_OP, // GLOBAL_LOAD_S
        UNAVAILABLE_OP, // STORE_N_S
        UNAVAILABLE_OP, // STORE_N_SR
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