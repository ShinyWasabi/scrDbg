#pragma once
#include <QDialog>

class QTableWidget;

namespace scrDbgApp
{
    class StackDialog : public QDialog
    {
        Q_OBJECT

    public:
        StackDialog(std::unique_ptr<ScriptThread> thread, Disassembler* disassembler, QWidget* parent = nullptr);

    private slots:
        void OnFrameSelected(int row, int column);
        void OnEditStack(int row, int column);

    private:
        void PopulateCallstack();
        void PopulateFrameDetails(int frameIndex);

        std::unique_ptr<ScriptThread> m_Thread;
        Disassembler* m_Disassembler;
        std::vector<uint32_t> m_FramePointers;

        QTableWidget* m_CallStack;
        QTableWidget* m_StackFrame;
    };
}