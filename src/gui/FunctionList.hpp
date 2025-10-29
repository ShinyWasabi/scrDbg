#pragma once
#include "ScriptLayout.hpp"
#include <QWidget>
#include <QtCore>

namespace scrDbg
{
    class FunctionListModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit FunctionListModel(const ScriptLayout& layout, QObject* parent = nullptr);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        uint32_t GetFunctionStart(int row) const;

    private:
        const ScriptLayout& m_Layout;
    };
}