#include "Instruction.hpp"

namespace scrDbgApp::Decomp
{
    const char* GetIrOpName(IrOp op)
    {
        static constexpr const char* names[] = {
            "NOP", "IADD", "ISUB", "IMUL", "IDIV", "IMOD", "INOT", "INEG", "IEQ", "INE", "IGT", "IGE", "ILT", "ILE",
            "FADD", "FSUB", "FMUL", "FDIV", "FMOD", "FNEG", "FEQ", "FNE", "FGT", "FGE", "FLT", "FLE",
            "VADD", "VSUB", "VMUL", "VDIV", "VNEG", "IAND", "IOR", "IXOR", "I2F", "F2I", "F2V",
            "PUSH_INT", "PUSH_FLOAT", "DUP", "DROP", "NATIVE", "ENTER", "LEAVE",
            "LOAD", "STORE", "STORE_REV", "LOAD_N", "STORE_N",
            "ARRAY", "ARRAY_LOAD", "ARRAY_STORE", "LOCAL", "LOCAL_LOAD", "LOCAL_STORE",
            "STATIC", "STATIC_LOAD", "STATIC_STORE", "GLOBAL", "GLOBAL_LOAD", "GLOBAL_STORE",
            "IADD_IMM", "IMUL_IMM", "IOFFSET", "IOFFSET_IMM", "IOFFSET_IMM_LOAD", "IOFFSET_IMM_STORE",
            "J", "JZ", "JNZ", "IEQ_JZ", "INE_JZ", "IGT_JZ", "IGE_JZ", "ILT_JZ", "ILE_JZ", "CALL", "SWITCH",
            "STRING", "STRINGHASH", "TEXT_LABEL_ASSIGN_STRING", "TEXT_LABEL_ASSIGN_INT", "TEXT_LABEL_APPEND_STRING",
            "TEXT_LABEL_APPEND_INT", "TEXT_LABEL_COPY", "CATCH", "THROW", "CALLINDIRECT", "IS_BIT_SET",
            "LOCAL_LOAD_S", "LOCAL_STORE_S", "LOCAL_STORE_SR", "STATIC_LOAD_S", "STATIC_STORE_S", "STATIC_STORE_SR",
            "GLOBAL_LOAD_S", "GLOBAL_STORE_S", "GLOBAL_STORE_SR", "LOAD_N_S", "STORE_N_S", "STORE_N_SR",
            "LOCAL_STACK", "STATIC_STACK", "GLOBAL_STACK", "ARRAY_STACK", "PUSH_STRING", "PUSH_NULL", "GETXPROTECT", "SETXPROTECT", "REFXPROTECT", "VECTOR_MAKE", "VECTOR_STORE", "PUSH_ARRAY", "UNKNOWN"};

        // static_assert(sizeof(names) / sizeof(names[0]) == static_cast<size_t>(IrOp::UNKNOWN) + 1);

        auto index = static_cast<size_t>(op);
        return index < sizeof(names) / sizeof(names[0]) ? names[index] : "???";
    }
}