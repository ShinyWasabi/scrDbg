#pragma once
#include <QDialog>
#include "game/rage/scrThread.hpp"
#include "script/ScriptLayout.hpp"

class QTableWidget;

namespace scrDbg
{
    class StackDialog : public QDialog
    {
        Q_OBJECT

    public:
        StackDialog(rage::scrThread thread, ScriptLayout& layout, QWidget* parent = nullptr);

    private slots:
        void OnFrameSelected(int row, int column);
        void OnEditStack(int row, int column);

    private:
        void PopulateCallstack();
        void PopulateFrameDetails(int frameIndex);

        rage::scrThread m_Thread;
        ScriptLayout& m_Layout;
        std::vector<uint32_t> m_FramePointers;

        QTableWidget* m_CallStack;
        QTableWidget* m_StackFrame;
    };
}