#pragma once
#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

namespace scrDbgApp
{
    class BreakpointsDialog : public QDialog
    {
        Q_OBJECT

    public:
        BreakpointsDialog(bool pauseGame, bool canChangePauseGame, QWidget* parent = nullptr);

        static uint32_t ResolveScriptHash(const QString& text);

    signals:
        void BreakpointDoubleClicked(uint32_t script, uint32_t pc);
        void PauseGameChanged(bool pause);

    private slots:
        void OnItemDoubleClicked(int row, int column);
        void OnAddClicked();
        void OnRemoveClicked();
        void OnRemoveAllClicked();
        void OnScriptTextChanged(const QString& text);

    private:
        void RefreshBreakpoints();
        void RefreshScriptNames();

        QTableWidget* m_Table;
        QPushButton* m_Remove;
        QPushButton* m_RemoveAll;
        QCheckBox* m_PauseGame;
        QComboBox* m_Script;
        QLineEdit* m_Pc;
        QLabel* m_ResolvedHash;
        QPushButton* m_Add;
    };
}