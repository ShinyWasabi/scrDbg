#pragma once
#include "Opcodes.hpp"

namespace scrDbgApp
{
    enum class OpcodesGTA5 : uint8_t
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

    static inline constexpr uint16_t g_OpcodesGTA5Map[] = {
        static_cast<uint16_t>(OpcodesGTA5::NOP),
        static_cast<uint16_t>(OpcodesGTA5::IADD),
        static_cast<uint16_t>(OpcodesGTA5::ISUB),
        static_cast<uint16_t>(OpcodesGTA5::IMUL),
        static_cast<uint16_t>(OpcodesGTA5::IDIV),
        static_cast<uint16_t>(OpcodesGTA5::IMOD),
        static_cast<uint16_t>(OpcodesGTA5::INOT),
        static_cast<uint16_t>(OpcodesGTA5::INEG),
        static_cast<uint16_t>(OpcodesGTA5::IEQ),
        static_cast<uint16_t>(OpcodesGTA5::INE),
        static_cast<uint16_t>(OpcodesGTA5::IGT),
        static_cast<uint16_t>(OpcodesGTA5::IGE),
        static_cast<uint16_t>(OpcodesGTA5::ILT),
        static_cast<uint16_t>(OpcodesGTA5::ILE),
        static_cast<uint16_t>(OpcodesGTA5::FADD),
        static_cast<uint16_t>(OpcodesGTA5::FSUB),
        static_cast<uint16_t>(OpcodesGTA5::FMUL),
        static_cast<uint16_t>(OpcodesGTA5::FDIV),
        static_cast<uint16_t>(OpcodesGTA5::FMOD),
        static_cast<uint16_t>(OpcodesGTA5::FNEG),
        static_cast<uint16_t>(OpcodesGTA5::FEQ),
        static_cast<uint16_t>(OpcodesGTA5::FNE),
        static_cast<uint16_t>(OpcodesGTA5::FGT),
        static_cast<uint16_t>(OpcodesGTA5::FGE),
        static_cast<uint16_t>(OpcodesGTA5::FLT),
        static_cast<uint16_t>(OpcodesGTA5::FLE),
        static_cast<uint16_t>(OpcodesGTA5::VADD),
        static_cast<uint16_t>(OpcodesGTA5::VSUB),
        static_cast<uint16_t>(OpcodesGTA5::VMUL),
        static_cast<uint16_t>(OpcodesGTA5::VDIV),
        static_cast<uint16_t>(OpcodesGTA5::VNEG),
        static_cast<uint16_t>(OpcodesGTA5::IAND),
        static_cast<uint16_t>(OpcodesGTA5::IOR),
        static_cast<uint16_t>(OpcodesGTA5::IXOR),
        static_cast<uint16_t>(OpcodesGTA5::J),
        static_cast<uint16_t>(OpcodesGTA5::JZ),
        static_cast<uint16_t>(OpcodesGTA5::I2F),
        static_cast<uint16_t>(OpcodesGTA5::F2I),
        static_cast<uint16_t>(OpcodesGTA5::F2V),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_S16),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_U32),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_F),
        static_cast<uint16_t>(OpcodesGTA5::DUP),
        static_cast<uint16_t>(OpcodesGTA5::DROP),
        static_cast<uint16_t>(OpcodesGTA5::NATIVE),
        static_cast<uint16_t>(OpcodesGTA5::CALL),
        static_cast<uint16_t>(OpcodesGTA5::ENTER),
        static_cast<uint16_t>(OpcodesGTA5::LEAVE),
        static_cast<uint16_t>(OpcodesGTA5::LOAD),
        static_cast<uint16_t>(OpcodesGTA5::STORE),
        static_cast<uint16_t>(OpcodesGTA5::STORE_REV),
        static_cast<uint16_t>(OpcodesGTA5::SWITCH),
        static_cast<uint16_t>(OpcodesGTA5::STRING),
        static_cast<uint16_t>(OpcodesGTA5::TEXT_LABEL_ASSIGN_STRING),
        static_cast<uint16_t>(OpcodesGTA5::TEXT_LABEL_ASSIGN_INT),
        static_cast<uint16_t>(OpcodesGTA5::TEXT_LABEL_APPEND_STRING),
        static_cast<uint16_t>(OpcodesGTA5::TEXT_LABEL_APPEND_INT),
        static_cast<uint16_t>(OpcodesGTA5::CATCH),
        static_cast<uint16_t>(OpcodesGTA5::THROW),
        static_cast<uint16_t>(OpcodesGTA5::TEXT_LABEL_COPY),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_M1),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_0),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_1),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_2),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_3),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_4),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_5),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_6),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_7),
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
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_U8),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_U8_U8),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_U8_U8_U8),
        static_cast<uint16_t>(OpcodesGTA5::ARRAY_U8),
        static_cast<uint16_t>(OpcodesGTA5::ARRAY_U8_LOAD),
        static_cast<uint16_t>(OpcodesGTA5::ARRAY_U8_STORE),
        static_cast<uint16_t>(OpcodesGTA5::LOCAL_U8),
        static_cast<uint16_t>(OpcodesGTA5::LOCAL_U8_LOAD),
        static_cast<uint16_t>(OpcodesGTA5::LOCAL_U8_STORE),
        static_cast<uint16_t>(OpcodesGTA5::STATIC_U8),
        static_cast<uint16_t>(OpcodesGTA5::STATIC_U8_LOAD),
        static_cast<uint16_t>(OpcodesGTA5::STATIC_U8_STORE),
        static_cast<uint16_t>(OpcodesGTA5::IADD_U8),
        static_cast<uint16_t>(OpcodesGTA5::IMUL_U8),
        static_cast<uint16_t>(OpcodesGTA5::IOFFSET_U8_LOAD),
        static_cast<uint16_t>(OpcodesGTA5::IOFFSET_U8_STORE),
        static_cast<uint16_t>(OpcodesGTA5::IADD_S16),
        static_cast<uint16_t>(OpcodesGTA5::IMUL_S16),
        static_cast<uint16_t>(OpcodesGTA5::IOFFSET_S16_LOAD),
        static_cast<uint16_t>(OpcodesGTA5::IOFFSET_S16_STORE),
        static_cast<uint16_t>(OpcodesGTA5::ARRAY_U16),
        static_cast<uint16_t>(OpcodesGTA5::ARRAY_U16_LOAD),
        static_cast<uint16_t>(OpcodesGTA5::ARRAY_U16_STORE),
        static_cast<uint16_t>(OpcodesGTA5::LOCAL_U16),
        static_cast<uint16_t>(OpcodesGTA5::LOCAL_U16_LOAD),
        static_cast<uint16_t>(OpcodesGTA5::LOCAL_U16_STORE),
        static_cast<uint16_t>(OpcodesGTA5::STATIC_U16),
        static_cast<uint16_t>(OpcodesGTA5::STATIC_U16_LOAD),
        static_cast<uint16_t>(OpcodesGTA5::STATIC_U16_STORE),
        static_cast<uint16_t>(OpcodesGTA5::GLOBAL_U16),
        static_cast<uint16_t>(OpcodesGTA5::GLOBAL_U16_LOAD),
        static_cast<uint16_t>(OpcodesGTA5::GLOBAL_U16_STORE),
        static_cast<uint16_t>(OpcodesGTA5::GLOBAL_U24),
        static_cast<uint16_t>(OpcodesGTA5::GLOBAL_U24_LOAD),
        static_cast<uint16_t>(OpcodesGTA5::GLOBAL_U24_STORE),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_U24),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_FM1),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_F0),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_F1),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_F2),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_F3),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_F4),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_F5),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_F6),
        static_cast<uint16_t>(OpcodesGTA5::PUSH_CONST_F7),
        static_cast<uint16_t>(OpcodesGTA5::LOAD_N),
        static_cast<uint16_t>(OpcodesGTA5::STORE_N),
        static_cast<uint16_t>(OpcodesGTA5::CALLINDIRECT),
        static_cast<uint16_t>(OpcodesGTA5::IOFFSET),
        static_cast<uint16_t>(OpcodesGTA5::IOFFSET_U8),
        static_cast<uint16_t>(OpcodesGTA5::IOFFSET_S16),
        static_cast<uint16_t>(OpcodesGTA5::IEQ_JZ),
        static_cast<uint16_t>(OpcodesGTA5::INE_JZ),
        static_cast<uint16_t>(OpcodesGTA5::IGT_JZ),
        static_cast<uint16_t>(OpcodesGTA5::IGE_JZ),
        static_cast<uint16_t>(OpcodesGTA5::ILT_JZ),
        static_cast<uint16_t>(OpcodesGTA5::ILE_JZ),
        static_cast<uint16_t>(OpcodesGTA5::STATIC_U24),
        static_cast<uint16_t>(OpcodesGTA5::STATIC_U24_LOAD),
        static_cast<uint16_t>(OpcodesGTA5::STATIC_U24_STORE),
        static_cast<uint16_t>(OpcodesGTA5::STRINGHASH),
        static_cast<uint16_t>(OpcodesGTA5::IS_BIT_SET),
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

    static int GetInsnSizeGTA5(const uint8_t* code, uint32_t pc)
    {
        OpcodesGTA5 op = static_cast<OpcodesGTA5>(code[pc]);

        switch (op)
        {
        case OpcodesGTA5::PUSH_CONST_U8:
        case OpcodesGTA5::ARRAY_U8:
        case OpcodesGTA5::ARRAY_U8_LOAD:
        case OpcodesGTA5::ARRAY_U8_STORE:
        case OpcodesGTA5::LOCAL_U8:
        case OpcodesGTA5::LOCAL_U8_LOAD:
        case OpcodesGTA5::LOCAL_U8_STORE:
        case OpcodesGTA5::STATIC_U8:
        case OpcodesGTA5::STATIC_U8_LOAD:
        case OpcodesGTA5::STATIC_U8_STORE:
        case OpcodesGTA5::IADD_U8:
        case OpcodesGTA5::IMUL_U8:
        case OpcodesGTA5::IOFFSET_U8:
        case OpcodesGTA5::IOFFSET_U8_LOAD:
        case OpcodesGTA5::IOFFSET_U8_STORE:
        case OpcodesGTA5::TEXT_LABEL_ASSIGN_STRING:
        case OpcodesGTA5::TEXT_LABEL_ASSIGN_INT:
        case OpcodesGTA5::TEXT_LABEL_APPEND_STRING:
        case OpcodesGTA5::TEXT_LABEL_APPEND_INT:
            return 2;
        case OpcodesGTA5::PUSH_CONST_U8_U8:
        case OpcodesGTA5::LEAVE:
        case OpcodesGTA5::PUSH_CONST_S16:
        case OpcodesGTA5::IADD_S16:
        case OpcodesGTA5::IMUL_S16:
        case OpcodesGTA5::IOFFSET_S16:
        case OpcodesGTA5::IOFFSET_S16_LOAD:
        case OpcodesGTA5::IOFFSET_S16_STORE:
        case OpcodesGTA5::ARRAY_U16:
        case OpcodesGTA5::ARRAY_U16_LOAD:
        case OpcodesGTA5::ARRAY_U16_STORE:
        case OpcodesGTA5::LOCAL_U16:
        case OpcodesGTA5::LOCAL_U16_LOAD:
        case OpcodesGTA5::LOCAL_U16_STORE:
        case OpcodesGTA5::STATIC_U16:
        case OpcodesGTA5::STATIC_U16_LOAD:
        case OpcodesGTA5::STATIC_U16_STORE:
        case OpcodesGTA5::GLOBAL_U16:
        case OpcodesGTA5::GLOBAL_U16_LOAD:
        case OpcodesGTA5::GLOBAL_U16_STORE:
        case OpcodesGTA5::J:
        case OpcodesGTA5::JZ:
        case OpcodesGTA5::IEQ_JZ:
        case OpcodesGTA5::INE_JZ:
        case OpcodesGTA5::IGT_JZ:
        case OpcodesGTA5::IGE_JZ:
        case OpcodesGTA5::ILT_JZ:
        case OpcodesGTA5::ILE_JZ:
            return 3;
        case OpcodesGTA5::PUSH_CONST_U8_U8_U8:
        case OpcodesGTA5::NATIVE:
        case OpcodesGTA5::CALL:
        case OpcodesGTA5::STATIC_U24:
        case OpcodesGTA5::STATIC_U24_LOAD:
        case OpcodesGTA5::STATIC_U24_STORE:
        case OpcodesGTA5::GLOBAL_U24:
        case OpcodesGTA5::GLOBAL_U24_LOAD:
        case OpcodesGTA5::GLOBAL_U24_STORE:
        case OpcodesGTA5::PUSH_CONST_U24:
            return 4;
        case OpcodesGTA5::PUSH_CONST_U32:
        case OpcodesGTA5::PUSH_CONST_F:
            return 5;
        case OpcodesGTA5::ENTER:
            return 5 + code[pc + 4];
        case OpcodesGTA5::SWITCH:
            return 2 + code[pc + 1] * 6;
        }

        return 1;
    }
}