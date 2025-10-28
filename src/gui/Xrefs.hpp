#pragma once
#include <QDialog>

class QTableWidget;

namespace scrDbg
{
    class XrefDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit XrefDialog(const std::vector<std::pair<uint32_t, std::string>>& xrefs, QWidget* parent = nullptr);

    signals:
        void XrefDoubleClicked(uint32_t address);

    private slots:
        void OnItemDoubleClicked(int row, int column);

    private:
        QTableWidget* m_Table;
    };
}