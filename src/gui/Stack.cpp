#include "Stack.hpp"
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QInputDialog>

namespace scrDbg
{
    StackDialog::StackDialog(rage::scrThread thread, ScriptLayout& layout, QWidget* parent) :
        QDialog(parent),
        m_Thread(thread),
        m_Layout(layout)
    {
        setWindowTitle("Stack");
        resize(700, 500);
        setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint);

        auto* callStackLabel = new QLabel("Call Stack:");
        m_CallStack = new QTableWidget(this);
        m_CallStack->setColumnCount(2);
        m_CallStack->setHorizontalHeaderLabels({ "Address", "Function" });
        m_CallStack->horizontalHeader()->setStretchLastSection(true);
        m_CallStack->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        m_CallStack->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_CallStack->setSelectionMode(QAbstractItemView::SingleSelection);
        m_CallStack->setEditTriggers(QAbstractItemView::NoEditTriggers);
        connect(m_CallStack, &QTableWidget::cellClicked, this, &StackDialog::OnFrameSelected);

        auto* callstackLayout = new QVBoxLayout();
        callstackLayout->addWidget(callStackLabel);
        callstackLayout->addWidget(m_CallStack);
        auto* callstackWidget = new QWidget();
        callstackWidget->setLayout(callstackLayout);

        auto* stackFrameLabel = new QLabel("Stack Frame (double-click to modify):");
        m_StackFrame = new QTableWidget(this);
        m_StackFrame->setColumnCount(3);
        m_StackFrame->setHorizontalHeaderLabels({ "Type", "Index", "Value" });
        m_StackFrame->horizontalHeader()->setStretchLastSection(true);
        m_StackFrame->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        m_StackFrame->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_StackFrame->setSelectionMode(QAbstractItemView::SingleSelection);
        m_StackFrame->setEditTriggers(QAbstractItemView::NoEditTriggers);
        connect(m_StackFrame, &QTableWidget::cellDoubleClicked, this, &StackDialog::OnEditStack);

        auto* stackframeLayout = new QVBoxLayout();
        stackframeLayout->addWidget(stackFrameLabel);
        stackframeLayout->addWidget(m_StackFrame);
        auto* stackframeWidget = new QWidget();
        stackframeWidget->setLayout(stackframeLayout);

        auto* splitter = new QSplitter(Qt::Vertical);
        splitter->addWidget(callstackWidget);
        splitter->addWidget(stackframeWidget);

        auto* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(splitter);

        PopulateCallstack();
    }

    void StackDialog::PopulateCallstack()
    {
        uint8_t depth = m_Thread.GetCallDepth();

        m_CallStack->setRowCount(depth);
        m_FramePointers.clear();
        m_FramePointers.resize(depth);

        uint32_t fp = m_Thread.GetFramePointer();

        for (int i = depth - 1; i >= 0; --i)
        {
            uint32_t addr = m_Thread.GetCallStack(i);

            int index = m_Layout.GetFunctionIndexForPc(addr);
            auto func = m_Layout.GetFunction(index);

            m_FramePointers[i] = fp;

            fp = static_cast<uint32_t>(m_Thread.GetStack(fp + func.ArgCount + 1));

            m_CallStack->setItem(i, 0, new QTableWidgetItem(QString("0x%1").arg(QString::number(addr, 16).toUpper())));
            m_CallStack->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(func.Name)));
        }

        if (depth > 0)
        {
            m_CallStack->selectRow(depth - 1);
            PopulateFrameDetails(depth - 1);
        }
    }

    void StackDialog::PopulateFrameDetails(int frameIndex)
    {
        uint32_t fp = m_FramePointers[frameIndex];
        uint32_t pc = m_Thread.GetCallStack(frameIndex);

        int index = m_Layout.GetFunctionIndexForPc(pc);
        auto func = m_Layout.GetFunction(index);

        m_StackFrame->setRowCount(0);

        int row = 0;

        for (int i = 0; i < func.ArgCount; ++i)
        {
            int value = static_cast<int>(m_Thread.GetStack(fp + i));
            m_StackFrame->insertRow(row);
            m_StackFrame->setItem(row, 0, new QTableWidgetItem("Arg"));
            m_StackFrame->setItem(row, 1, new QTableWidgetItem(QString::number(i)));
            m_StackFrame->setItem(row, 2, new QTableWidgetItem(QString::number(value)));
            ++row;
        }

        int localCount = func.FrameSize - func.ArgCount - 2;
        for (int i = 0; i < localCount; ++i)
        {
            int value = static_cast<int>(m_Thread.GetStack(fp + func.ArgCount + 2 + i));
            m_StackFrame->insertRow(row);
            m_StackFrame->setItem(row, 0, new QTableWidgetItem("Local"));
            m_StackFrame->setItem(row, 1, new QTableWidgetItem(QString::number(func.ArgCount + 2 + i)));
            m_StackFrame->setItem(row, 2, new QTableWidgetItem(QString::number(value)));
            ++row;
        }
    }

    void StackDialog::OnFrameSelected(int row, int column)
    {
        Q_UNUSED(column);

        PopulateFrameDetails(row);
    }

    void StackDialog::OnEditStack(int row, int column)
    {
        Q_UNUSED(column);

        uint32_t fp = m_FramePointers[m_CallStack->currentRow()];

        int index = m_StackFrame->item(row, 1)->text().toInt();

        int current = static_cast<int>(m_Thread.GetStack(fp + index));
        int value = static_cast<int>(QInputDialog::getInt(this, "Edit Stack", "Enter value:", current));

        m_Thread.SetStack(fp + index, value);
        m_StackFrame->item(row, 2)->setText(QString::number(value));
    }
}