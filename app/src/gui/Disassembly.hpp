#pragma once
#include "script/ScriptLayout.hpp"
#include <QWidget>
#include <QtCore>

namespace scrDbg
{
    class DisassemblyModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit DisassemblyModel(ScriptLayout& layout, QObject* parent = nullptr);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex&) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    private:
        ScriptLayout& m_Layout;
    };
}