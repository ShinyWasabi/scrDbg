#pragma once
#include <QDialog>

class QTableWidget;
class QPushButton;

namespace scrDbg
{
    class BreakpointsDialog : public QDialog
    {
        Q_OBJECT
    
    public:
        explicit BreakpointsDialog(QWidget* parent = nullptr);

    signals:
        void BreakpointDoubleClicked(uint32_t script, uint32_t pc);

    private slots:
        void OnItemDoubleClicked(int row, int column);
        void OnRemoveAllClicked();

    private:
        void RefreshBreakpoints();

        QTableWidget* m_Table;
        QPushButton* m_RemoveAll;
    };
}