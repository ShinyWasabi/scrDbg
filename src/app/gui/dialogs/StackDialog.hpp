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
        // for GTA 4, MP3, and RDR1, which doesn't natively have call stack and depth in their scrThreads
        struct CallStack
        {
            uint8_t Depth;
            uint32_t Frames[16];
        };

        CallStack GetCallStack() const;

        void PopulateCallstack();
        void PopulateFrameDetails(int frameIndex);
        void UpdateStackValue(int row, Pointer pointer, int type);

        std::unique_ptr<ScriptThread> m_Thread;
        Disassembler* m_Disassembler;
        std::vector<uint32_t> m_FramePointers;

        QTableWidget* m_CallStack;
        QTableWidget* m_StackFrame;
    };
}