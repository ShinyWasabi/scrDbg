#include "DisassemblyModel.hpp"
#include "disasm/Disassembler.hpp"
#include "pipe/PipeCommands.hpp"

namespace scrDbgApp
{
    DisassemblyModel::DisassemblyModel(const Disassembler* disassembler, QObject* parent)
        : QAbstractTableModel(parent),
          m_Disassembler(disassembler)
    {
    }

    int DisassemblyModel::rowCount(const QModelIndex& parent) const
    {
        return m_Disassembler->GetInstructionCount();
    }

    int DisassemblyModel::columnCount(const QModelIndex&) const
    {
        return 3;
    }

    QVariant DisassemblyModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
            return QVariant();

        static const char* headers[] = {"Address", "Bytes", "Instruction"};
        return section < 3 ? headers[section] : QVariant();
    }

    QVariant DisassemblyModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (role == Qt::BackgroundRole)
        {
            auto pc = m_Disassembler->GetInstruction(index.row());
            auto hash = m_Disassembler->GetHash();

            auto active = PipeCommands::GetActiveBreakpoint();
            if (active && active->first == hash && active->second == pc)
                return QBrush(Qt::green);

            if (PipeCommands::BreakpointExists(hash, pc))
                return QBrush(Qt::red);

            return QVariant();
        }

        if (role != Qt::DisplayRole)
            return QVariant();

        // DecodeInstruction builds Address/Bytes/Instruction together in one
        // pass. data() gets called once per column per row by Qt, so without
        // this cache we would redo the full decode 3x per row on every repaint.
        // Qt queries columns for a row consecutively, so a single-row memo
        // collapses that back down to one decode per row.
        if (index.row() != m_CachedRow)
        {
            m_CachedInsn = m_Disassembler->DecodeInstruction(index.row());
            m_CachedRow = index.row();
        }

        switch (index.column())
        {
        case 0:
            return QString::fromStdString(m_CachedInsn.Address);
        case 1:
            return QString::fromStdString(m_CachedInsn.Bytes);
        case 2:
            return QString::fromStdString(m_CachedInsn.Instruction);
        }

        return QVariant();
    }
}