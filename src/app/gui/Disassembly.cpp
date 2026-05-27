#include "Disassembly.hpp"
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

        const auto entry = m_Disassembler->GetInstruction(index.row());

        if (role == Qt::BackgroundRole)
        {
            auto hash = m_Disassembler->GetHash();

            auto active = PipeCommands::GetActiveBreakpoint();
            if (active && active->first == hash && active->second == entry.Pc)
                return QBrush(Qt::green);

            if (PipeCommands::BreakpointExists(hash, entry.Pc))
                return QBrush(Qt::red);

            return QVariant();
        }

        if (role != Qt::DisplayRole)
            return QVariant();

        auto insn = m_Disassembler->DecodeInstruction(index.row());

        switch (index.column())
        {
        case 0:
            return QString::fromStdString(insn.Address);
        case 1:
            return QString::fromStdString(insn.Bytes);
        case 2:
            return QString::fromStdString(insn.Instruction);
        }

        return QVariant();
    }
}