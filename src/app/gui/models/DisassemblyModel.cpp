#include "DisassemblyModel.hpp"
#include "disasm/Disassembler.hpp"
#include "gui/style/BreakpointStyle.hpp"
#include "pipe/PipeCommands.hpp"
#include <QBrush>

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

    void DisassemblyModel::RefreshBreakpointsIfStale() const
    {
        if (m_BreakpointsValid && m_BreakpointsAge.isValid() && m_BreakpointsAge.elapsed() < 250)
            return;

        const uint32_t hash = m_Disassembler->GetHash();

        m_Breakpoints.clear();
        for (const auto& [script, pc] : PipeCommands::GetAllBreakpoints())
        {
            if (script == hash)
                m_Breakpoints.insert(pc);
        }

        m_ActivePc.reset();
        auto active = PipeCommands::GetActiveBreakpoint();
        if (active && active->first == hash)
            m_ActivePc = active->second;

        m_BreakpointsValid = true;
        m_BreakpointsAge.start();
    }

    void DisassemblyModel::InvalidateBreakpoints()
    {
        m_BreakpointsValid = false;

        if (rowCount() > 0)
            emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount({}) - 1), {Qt::BackgroundRole, Qt::DecorationRole});
    }

    QVariant DisassemblyModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (role == Qt::BackgroundRole || (role == Qt::DecorationRole && index.column() == 0))
        {
            RefreshBreakpointsIfStale();

            uint32_t pc = m_Disassembler->GetInstruction(index.row());
            bool execution = m_ActivePc && *m_ActivePc == pc;
            bool breakpoint = m_Breakpoints.count(pc) != 0;

            if (role == Qt::DecorationRole)
                return BreakpointStyle::MarkerPixmap(16, breakpoint, execution);

            if (execution)
                return QBrush(BreakpointStyle::EXECUTION_LINE);
            if (breakpoint)
                return QBrush(BreakpointStyle::BREAKPOINT_LINE);
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