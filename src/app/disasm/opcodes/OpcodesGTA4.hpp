#pragma once
#include "Opcodes.hpp"

namespace scrDbgApp
{
    enum class OpcodesGTA4 : uint8_t
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
        J,
        JZ,
        JNZ,
        I2F,
        F2I,
        F2V,
        PUSH_CONST_S16,
        PUSH_CONST_U32,
        PUSH_CONST_F,
        DUP,
        DROP,
        NATIVE,
        CALL,
        ENTER,
        LEAVE,
        LOAD,
        STORE,
        STORE_REV,
        ARRAY_LOAD,
        ARRAY_STORE,
        LOCAL_0,
        LOCAL_1,
        LOCAL_2,
        LOCAL_3,
        LOCAL_4,
        LOCAL_5,
        LOCAL_6,
        LOCAL_7,
        LOCAL,
        STATIC,
        GLOBAL,
        ARRAY,
        SWITCH,
        STRING,
        _NULL,
        TEXT_LABEL_ASSIGN_STRING,
        TEXT_LABEL_ASSIGN_INT,
        TEXT_LABEL_APPEND_STRING,
        TEXT_LABEL_APPEND_INT,
        CATCH,
        THROW,
        TEXT_LABEL_COPY,
        GETXPROTECT,
        SETXPROTECT,
        REFXPROTECT,
        EXIT,
        PUSH_CONST_M16,
        PUSH_CONST_M15,
        PUSH_CONST_M14,
        PUSH_CONST_M13,
        PUSH_CONST_M12,
        PUSH_CONST_M11,
        PUSH_CONST_M10,
        PUSH_CONST_M9,
        PUSH_CONST_M8,
        PUSH_CONST_M7,
        PUSH_CONST_M6,
        PUSH_CONST_M5,
        PUSH_CONST_M4,
        PUSH_CONST_M3,
        PUSH_CONST_M2,
        PUSH_CONST_M1,
        PUSH_CONST_0,
        PUSH_CONST_1,
        PUSH_CONST_2,
        PUSH_CONST_3,
        PUSH_CONST_4,
        PUSH_CONST_5,
        PUSH_CONST_6,
        PUSH_CONST_7,
        PUSH_CONST_8,
        PUSH_CONST_9,
        PUSH_CONST_10,
        PUSH_CONST_11,
        PUSH_CONST_12,
        PUSH_CONST_13,
        PUSH_CONST_14,
        PUSH_CONST_15,
        PUSH_CONST_16,
        PUSH_CONST_17,
        PUSH_CONST_18,
        PUSH_CONST_19,
        PUSH_CONST_20,
        PUSH_CONST_21,
        PUSH_CONST_22,
        PUSH_CONST_23,
        PUSH_CONST_24,
        PUSH_CONST_25,
        PUSH_CONST_26,
        PUSH_CONST_27,
        PUSH_CONST_28,
        PUSH_CONST_29,
        PUSH_CONST_30,
        PUSH_CONST_31,
        PUSH_CONST_32,
        PUSH_CONST_33,
        PUSH_CONST_34,
        PUSH_CONST_35,
        PUSH_CONST_36,
        PUSH_CONST_37,
        PUSH_CONST_38,
        PUSH_CONST_39,
        PUSH_CONST_40,
        PUSH_CONST_41,
        PUSH_CONST_42,
        PUSH_CONST_43,
        PUSH_CONST_44,
        PUSH_CONST_45,
        PUSH_CONST_46,
        PUSH_CONST_47,
        PUSH_CONST_48,
        PUSH_CONST_49,
        PUSH_CONST_50,
        PUSH_CONST_51,
        PUSH_CONST_52,
        PUSH_CONST_53,
        PUSH_CONST_54,
        PUSH_CONST_55,
        PUSH_CONST_56,
        PUSH_CONST_57,
        PUSH_CONST_58,
        PUSH_CONST_59,
        PUSH_CONST_60,
        PUSH_CONST_61,
        PUSH_CONST_62,
        PUSH_CONST_63,
        PUSH_CONST_64,
        PUSH_CONST_65,
        PUSH_CONST_66,
        PUSH_CONST_67,
        PUSH_CONST_68,
        PUSH_CONST_69,
        PUSH_CONST_70,
        PUSH_CONST_71,
        PUSH_CONST_72,
        PUSH_CONST_73,
        PUSH_CONST_74,
        PUSH_CONST_75,
        PUSH_CONST_76,
        PUSH_CONST_77,
        PUSH_CONST_78,
        PUSH_CONST_79,
        PUSH_CONST_80,
        PUSH_CONST_81,
        PUSH_CONST_82,
        PUSH_CONST_83,
        PUSH_CONST_84,
        PUSH_CONST_85,
        PUSH_CONST_86,
        PUSH_CONST_87,
        PUSH_CONST_88,
        PUSH_CONST_89,
        PUSH_CONST_90,
        PUSH_CONST_91,
        PUSH_CONST_92,
        PUSH_CONST_93,
        PUSH_CONST_94,
        PUSH_CONST_95,
        PUSH_CONST_96,
        PUSH_CONST_97,
        PUSH_CONST_98,
        PUSH_CONST_99,
        PUSH_CONST_100,
        PUSH_CONST_101,
        PUSH_CONST_102,
        PUSH_CONST_103,
        PUSH_CONST_104,
        PUSH_CONST_105,
        PUSH_CONST_106,
        PUSH_CONST_107,
        PUSH_CONST_108,
        PUSH_CONST_109,
        PUSH_CONST_110,
        PUSH_CONST_111,
        PUSH_CONST_112,
        PUSH_CONST_113,
        PUSH_CONST_114,
        PUSH_CONST_115,
        PUSH_CONST_116,
        PUSH_CONST_117,
        PUSH_CONST_118,
        PUSH_CONST_119,
        PUSH_CONST_120,
        PUSH_CONST_121,
        PUSH_CONST_122,
        PUSH_CONST_123,
        PUSH_CONST_124,
        PUSH_CONST_125,
        PUSH_CONST_126,
        PUSH_CONST_127,
        PUSH_CONST_128,
        PUSH_CONST_129,
        PUSH_CONST_130,
        PUSH_CONST_131,
        PUSH_CONST_132,
        PUSH_CONST_133,
        PUSH_CONST_134,
        PUSH_CONST_135,
        PUSH_CONST_136,
        PUSH_CONST_137,
        PUSH_CONST_138,
        PUSH_CONST_139,
        PUSH_CONST_140,
        PUSH_CONST_141,
        PUSH_CONST_142,
        PUSH_CONST_143,
        PUSH_CONST_144,
        PUSH_CONST_145,
        PUSH_CONST_146,
        PUSH_CONST_147,
        PUSH_CONST_148,
        PUSH_CONST_149,
        PUSH_CONST_150,
        PUSH_CONST_151,
        PUSH_CONST_152,
        PUSH_CONST_153,
        PUSH_CONST_154,
        PUSH_CONST_155,
        PUSH_CONST_156,
        PUSH_CONST_157,
        PUSH_CONST_158,
        PUSH_CONST_159
    };

    static inline constexpr uint16_t g_OpcodesGTA4Map[] = {
        static_cast<uint16_t>(OpcodesGTA4::NOP),
        static_cast<uint16_t>(OpcodesGTA4::IADD),
        static_cast<uint16_t>(OpcodesGTA4::ISUB),
        static_cast<uint16_t>(OpcodesGTA4::IMUL),
        static_cast<uint16_t>(OpcodesGTA4::IDIV),
        static_cast<uint16_t>(OpcodesGTA4::IMOD),
        static_cast<uint16_t>(OpcodesGTA4::INOT),
        static_cast<uint16_t>(OpcodesGTA4::INEG),
        static_cast<uint16_t>(OpcodesGTA4::IEQ),
        static_cast<uint16_t>(OpcodesGTA4::INE),
        static_cast<uint16_t>(OpcodesGTA4::IGT),
        static_cast<uint16_t>(OpcodesGTA4::IGE),
        static_cast<uint16_t>(OpcodesGTA4::ILT),
        static_cast<uint16_t>(OpcodesGTA4::ILE),
        static_cast<uint16_t>(OpcodesGTA4::FADD),
        static_cast<uint16_t>(OpcodesGTA4::FSUB),
        static_cast<uint16_t>(OpcodesGTA4::FMUL),
        static_cast<uint16_t>(OpcodesGTA4::FDIV),
        static_cast<uint16_t>(OpcodesGTA4::FMOD),
        static_cast<uint16_t>(OpcodesGTA4::FNEG),
        static_cast<uint16_t>(OpcodesGTA4::FEQ),
        static_cast<uint16_t>(OpcodesGTA4::FNE),
        static_cast<uint16_t>(OpcodesGTA4::FGT),
        static_cast<uint16_t>(OpcodesGTA4::FGE),
        static_cast<uint16_t>(OpcodesGTA4::FLT),
        static_cast<uint16_t>(OpcodesGTA4::FLE),
        static_cast<uint16_t>(OpcodesGTA4::VADD),
        static_cast<uint16_t>(OpcodesGTA4::VSUB),
        static_cast<uint16_t>(OpcodesGTA4::VMUL),
        static_cast<uint16_t>(OpcodesGTA4::VDIV),
        static_cast<uint16_t>(OpcodesGTA4::VNEG),
        static_cast<uint16_t>(OpcodesGTA4::IAND),
        static_cast<uint16_t>(OpcodesGTA4::IOR),
        static_cast<uint16_t>(OpcodesGTA4::IXOR),
        static_cast<uint16_t>(OpcodesGTA4::J),
        static_cast<uint16_t>(OpcodesGTA4::JZ),
        static_cast<uint16_t>(OpcodesGTA4::I2F),
        static_cast<uint16_t>(OpcodesGTA4::F2I),
        static_cast<uint16_t>(OpcodesGTA4::F2V),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_S16),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_U32),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_F),
        static_cast<uint16_t>(OpcodesGTA4::DUP),
        static_cast<uint16_t>(OpcodesGTA4::DROP),
        static_cast<uint16_t>(OpcodesGTA4::NATIVE),
        static_cast<uint16_t>(OpcodesGTA4::CALL),
        static_cast<uint16_t>(OpcodesGTA4::ENTER),
        static_cast<uint16_t>(OpcodesGTA4::LEAVE),
        static_cast<uint16_t>(OpcodesGTA4::LOAD),
        static_cast<uint16_t>(OpcodesGTA4::STORE),
        static_cast<uint16_t>(OpcodesGTA4::STORE_REV),
        static_cast<uint16_t>(OpcodesGTA4::SWITCH),
        static_cast<uint16_t>(OpcodesGTA4::STRING),
        static_cast<uint16_t>(OpcodesGTA4::TEXT_LABEL_ASSIGN_STRING),
        static_cast<uint16_t>(OpcodesGTA4::TEXT_LABEL_ASSIGN_INT),
        static_cast<uint16_t>(OpcodesGTA4::TEXT_LABEL_APPEND_STRING),
        static_cast<uint16_t>(OpcodesGTA4::TEXT_LABEL_APPEND_INT),
        static_cast<uint16_t>(OpcodesGTA4::CATCH),
        static_cast<uint16_t>(OpcodesGTA4::THROW),
        static_cast<uint16_t>(OpcodesGTA4::TEXT_LABEL_COPY),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M1),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_0),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_1),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_2),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_3),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_4),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_5),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_6),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_7),
        static_cast<uint16_t>(OpcodesGTA4::JNZ),
        static_cast<uint16_t>(OpcodesGTA4::LOCAL_0),
        static_cast<uint16_t>(OpcodesGTA4::LOCAL_1),
        static_cast<uint16_t>(OpcodesGTA4::LOCAL_2),
        static_cast<uint16_t>(OpcodesGTA4::LOCAL_3),
        static_cast<uint16_t>(OpcodesGTA4::LOCAL_4),
        static_cast<uint16_t>(OpcodesGTA4::LOCAL_5),
        static_cast<uint16_t>(OpcodesGTA4::LOCAL_6),
        static_cast<uint16_t>(OpcodesGTA4::LOCAL_7),
        static_cast<uint16_t>(OpcodesGTA4::LOCAL),
        static_cast<uint16_t>(OpcodesGTA4::STATIC),
        static_cast<uint16_t>(OpcodesGTA4::GLOBAL),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M16),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M15),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M14),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M13),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M12),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M11),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M10),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M9),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M8),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M7),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M6),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M5),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M4),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M3),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_M2),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_8),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_9),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_10),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_11),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_12),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_13),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_14),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_15),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_16),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_17),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_18),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_19),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_20),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_21),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_22),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_23),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_24),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_25),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_26),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_27),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_28),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_29),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_30),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_31),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_32),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_33),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_34),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_35),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_36),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_37),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_38),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_39),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_40),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_41),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_42),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_43),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_44),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_45),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_46),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_47),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_48),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_49),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_50),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_51),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_52),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_53),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_54),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_55),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_56),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_57),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_58),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_59),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_60),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_61),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_62),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_63),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_64),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_65),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_66),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_67),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_68),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_69),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_70),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_71),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_72),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_73),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_74),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_75),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_76),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_77),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_78),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_79),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_80),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_81),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_82),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_83),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_84),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_85),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_86),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_87),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_88),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_89),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_90),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_91),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_92),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_93),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_94),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_95),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_96),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_97),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_98),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_99),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_100),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_101),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_102),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_103),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_104),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_105),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_106),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_107),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_108),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_109),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_110),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_111),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_112),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_113),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_114),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_115),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_116),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_117),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_118),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_119),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_120),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_121),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_122),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_123),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_124),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_125),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_126),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_127),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_128),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_129),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_130),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_131),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_132),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_133),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_134),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_135),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_136),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_137),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_138),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_139),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_140),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_141),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_142),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_143),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_144),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_145),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_146),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_147),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_148),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_149),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_150),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_151),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_152),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_153),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_154),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_155),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_156),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_157),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_158),
        static_cast<uint16_t>(OpcodesGTA4::PUSH_CONST_159),
        static_cast<uint16_t>(OpcodesGTA4::ARRAY_LOAD),
        static_cast<uint16_t>(OpcodesGTA4::ARRAY_STORE),
        static_cast<uint16_t>(OpcodesGTA4::GETXPROTECT),
        static_cast<uint16_t>(OpcodesGTA4::SETXPROTECT),
        static_cast<uint16_t>(OpcodesGTA4::REFXPROTECT),
        static_cast<uint16_t>(OpcodesGTA4::EXIT),
        static_cast<uint16_t>(OpcodesGTA4::ARRAY),
        static_cast<uint16_t>(OpcodesGTA4::_NULL),
        UNAVAILABLE_OP, // PUSH_CONST_U8
        UNAVAILABLE_OP, // PUSH_CONST_U8_U8
        UNAVAILABLE_OP, // PUSH_CONST_U8_U8_U8
        UNAVAILABLE_OP, // ARRAY_U8
        UNAVAILABLE_OP, // ARRAY_U8_LOAD
        UNAVAILABLE_OP, // ARRAY_U8_STORE
        UNAVAILABLE_OP, // LOCAL_U8
        UNAVAILABLE_OP, // LOCAL_U8_LOAD
        UNAVAILABLE_OP, // LOCAL_U8_STORE
        UNAVAILABLE_OP, // STATIC_U8
        UNAVAILABLE_OP, // STATIC_U8_LOAD
        UNAVAILABLE_OP, // STATIC_U8_STORE
        UNAVAILABLE_OP, // IADD_U8
        UNAVAILABLE_OP, // IMUL_U8
        UNAVAILABLE_OP, // IOFFSET_U8_LOAD
        UNAVAILABLE_OP, // IOFFSET_U8_STORE
        UNAVAILABLE_OP, // IADD_S16
        UNAVAILABLE_OP, // IMUL_S16
        UNAVAILABLE_OP, // IOFFSET_S16_LOAD
        UNAVAILABLE_OP, // IOFFSET_S16_STORE
        UNAVAILABLE_OP, // ARRAY_U16
        UNAVAILABLE_OP, // ARRAY_U16_LOAD
        UNAVAILABLE_OP, // ARRAY_U16_STORE
        UNAVAILABLE_OP, // LOCAL_U16
        UNAVAILABLE_OP, // LOCAL_U16_LOAD
        UNAVAILABLE_OP, // LOCAL_U16_STORE
        UNAVAILABLE_OP, // STATIC_U16
        UNAVAILABLE_OP, // STATIC_U16_LOAD
        UNAVAILABLE_OP, // STATIC_U16_STORE
        UNAVAILABLE_OP, // GLOBAL_U16
        UNAVAILABLE_OP, // GLOBAL_U16_LOAD
        UNAVAILABLE_OP, // GLOBAL_U16_STORE
        UNAVAILABLE_OP, // GLOBAL_U24
        UNAVAILABLE_OP, // GLOBAL_U24_LOAD
        UNAVAILABLE_OP, // GLOBAL_U24_STORE
        UNAVAILABLE_OP, // PUSH_CONST_U24
        UNAVAILABLE_OP, // PUSH_CONST_FM1
        UNAVAILABLE_OP, // PUSH_CONST_F0
        UNAVAILABLE_OP, // PUSH_CONST_F1
        UNAVAILABLE_OP, // PUSH_CONST_F2
        UNAVAILABLE_OP, // PUSH_CONST_F3
        UNAVAILABLE_OP, // PUSH_CONST_F4
        UNAVAILABLE_OP, // PUSH_CONST_F5
        UNAVAILABLE_OP, // PUSH_CONST_F6
        UNAVAILABLE_OP, // PUSH_CONST_F7
        UNAVAILABLE_OP, // LOAD_N
        UNAVAILABLE_OP, // STORE_N
        UNAVAILABLE_OP, // CALLINDIRECT
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

    static int GetInsnSizeGTA4(const uint8_t* code, uint32_t pc)
    {
        OpcodesGTA4 op = static_cast<OpcodesGTA4>(code[pc]);

        switch (op)
        {
        case OpcodesGTA4::J:
        case OpcodesGTA4::JZ:
        case OpcodesGTA4::JNZ:
        case OpcodesGTA4::PUSH_CONST_U32:
        case OpcodesGTA4::PUSH_CONST_F:
        case OpcodesGTA4::CALL:
            return 5;
        case OpcodesGTA4::PUSH_CONST_S16:
            return 3;
        case OpcodesGTA4::NATIVE:
            return 7;
        case OpcodesGTA4::ENTER:
            return 4;
        case OpcodesGTA4::LEAVE:
            return 3;
        case OpcodesGTA4::TEXT_LABEL_ASSIGN_STRING:
        case OpcodesGTA4::TEXT_LABEL_ASSIGN_INT:
        case OpcodesGTA4::TEXT_LABEL_APPEND_STRING:
        case OpcodesGTA4::TEXT_LABEL_APPEND_INT:
            return 2;
        case OpcodesGTA4::STRING:
            return 2 + code[pc + 1];
        case OpcodesGTA4::SWITCH:
            return 2 + code[pc + 1] * 8;
        }

        return 1;
    }
}