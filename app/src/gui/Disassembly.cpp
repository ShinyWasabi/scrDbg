#include "Disassembly.hpp"
#include "game/rage/scrOpcode.hpp"
#include "pipe/PipeCommands.hpp"
#include "script/ScriptDisassembler.hpp"
#include "util/ScriptHelpers.hpp"

namespace scrDbg
{
    DisassemblyModel::DisassemblyModel(ScriptLayout& layout, QObject* parent)
        : QAbstractTableModel(parent),
          m_Layout(layout)
    {
    }

    int DisassemblyModel::rowCount(const QModelIndex& parent) const
    {
        return m_Layout.GetInstructionCount();
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

        const auto& code = m_Layout.GetCode();
        const auto entry = m_Layout.GetInstruction(index.row());

        if (role == Qt::BackgroundRole)
        {
            auto hash = m_Layout.GetHash();

            auto active = PipeCommands::GetActiveBreakpoint();
            if (active && active->first == hash && active->second == entry.Pc)
                return QBrush(Qt::green);

            if (PipeCommands::BreakpointExists(hash, entry.Pc))
                return QBrush(Qt::red);

            return QVariant();
        }

        if (role != Qt::DisplayRole)
            return QVariant();

        int funcIndex = entry.FuncIndex;
        if (code[entry.Pc] == rage::scrOpcode::CALL)
        {
            uint32_t targetPc = ScriptHelpers::ReadU24(code, entry.Pc + 1);

            int targetFuncIndex = m_Layout.GetFunctionIndexForPc(targetPc);
            if (targetFuncIndex != -1)
                funcIndex = targetFuncIndex;
        }

        auto insn = ScriptDisassembler::DecodeInstruction(code, entry.Pc, m_Layout.GetProgram(), entry.StringIndex, funcIndex);

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