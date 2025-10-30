#include "FunctionList.hpp"
#include "Disassembler.hpp"
#include "game/rage/Opcode.hpp"

namespace scrDbg
{
    FunctionListModel::FunctionListModel(const ScriptLayout& layout, QObject* parent) :
        QAbstractTableModel(parent),
        m_Layout(layout)
    {
    }

    int FunctionListModel::rowCount(const QModelIndex&) const
    {
        return static_cast<int>(m_Layout.GetFunctions().size());
    }

    int FunctionListModel::columnCount(const QModelIndex&) const
    {
        return 7;
    }

    QVariant FunctionListModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
            return QVariant();

        static const char* headers[] = { "Name", "Start", "End", "Length", "Args", "Locals", "Rets" };
        return section < 7 ? headers[section] : QVariant();
    }

    QVariant FunctionListModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || role != Qt::DisplayRole)
            return QVariant();

        const auto& func = m_Layout.GetFunctions()[index.row()];

        switch (index.column())
        {
        case 0: return QString::fromStdString(func.Name);
        case 1: return QString("0x%1").arg(QString::number(func.Start, 16).toUpper());
        case 2: return QString("0x%1").arg(QString::number(func.End, 16).toUpper());
        case 3: return func.Length;
        case 4: return func.ArgCount;
        case 5: return func.LocalCount;
        case 6: return func.RetCount;
        }

        return QVariant();
    }
}