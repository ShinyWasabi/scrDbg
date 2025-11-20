#pragma once
#include <QDialog>

class QLineEdit;
class QTableView;
class QSortFilterProxyModel;
class QStandardItemModel;

namespace scrDbg
{
    class ResultsDialog : public QDialog
    {
        Q_OBJECT

    public:
        struct Entry
        {
            uint32_t Address;
            std::string FunctionName;
            std::string Instruction;
        };

        explicit ResultsDialog(const QString& title, const std::vector<Entry>& entries, QWidget* parent = nullptr);

    signals:
        void EntryDoubleClicked(uint32_t address);

    private slots:
        void OnItemDoubleClicked(const QModelIndex& index);

    private:
        QStandardItemModel* m_Model;
        QSortFilterProxyModel* m_Filter;
        QTableView* m_Table;
        QLineEdit* m_SearchBox;
    };
}