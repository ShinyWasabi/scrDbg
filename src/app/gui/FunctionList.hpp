#pragma once
#include <QWidget>
#include <QtCore>

namespace scrDbgApp
{
    class FunctionListModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit FunctionListModel(const Disassembler* disassembler, QObject* parent = nullptr);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    private:
        const Disassembler* m_Disassembler;
    };
}