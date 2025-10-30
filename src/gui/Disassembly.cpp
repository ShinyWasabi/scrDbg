#include "Disassembly.hpp"
#include "Disassembler.hpp"
#include "game/rage/Opcode.hpp"

namespace scrDbg
{
    DisassemblyModel::DisassemblyModel(ScriptLayout& layout, QObject* parent) :
        QAbstractTableModel(parent),
        m_Layout(layout)
    {
    }

    int DisassemblyModel::rowCount(const QModelIndex& parent) const
    {
        return static_cast<int>(m_Layout.GetInstructions().size());
    }

    int DisassemblyModel::columnCount(const QModelIndex&) const
    {
        return 3;
    }

    QVariant DisassemblyModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
            return QVariant();

        static const char* headers[] = { "Address", "Bytes", "Instruction" };
        return section < 3 ? headers[section] : QVariant();
    }

    QVariant DisassemblyModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || role != Qt::DisplayRole)
            return QVariant();

        const auto& entry = m_Layout.GetInstructions()[index.row()];
        const auto& code = m_Layout.GetCode();

        int funcIndex = entry.FuncIndex;
        if (code[entry.Pc] == rage::scrOpcode::CALL)
        {
            uint32_t targetPc = ScriptDisassembler::ReadU24(code, entry.Pc + 1);

            int targetFuncIndex = m_Layout.GetFunctionIndexForPc(targetPc);
            if (targetFuncIndex != -1)
                funcIndex = targetFuncIndex;
        }

        auto insn = ScriptDisassembler::DecodeInstruction(m_Layout.GetProgram(), code, entry.Pc, entry.StringIndex, funcIndex);

        switch (index.column())
        {
        case 0: return QString::fromStdString(insn.Address);
        case 1: return QString::fromStdString(insn.Bytes);
        case 2: return QString::fromStdString(insn.Instruction);
        }

        return QVariant();
    }
}