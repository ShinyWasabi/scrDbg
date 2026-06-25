#include "StackDialog.hpp"
#include "EditValueDialog.hpp"
#include <QComboBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QSplitter>
#include <QTableWidget>
#include <QVBoxLayout>

namespace scrDbgApp
{
    StackDialog::StackDialog(std::unique_ptr<ScriptThread> thread, Disassembler* disassembler, QWidget* parent)
        : QDialog(parent),
          m_Thread(std::move(thread)),
          m_Disassembler(disassembler)
    {
        setWindowTitle("Stack");
        resize(700, 500);
        setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint);

        auto callStackLabel = new QLabel("Call Stack:");
        m_CallStack = new QTableWidget(this);
        m_CallStack->setColumnCount(2);
        m_CallStack->setHorizontalHeaderLabels({"Address", "Function"});
        m_CallStack->horizontalHeader()->setStretchLastSection(true);
        m_CallStack->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        m_CallStack->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_CallStack->setSelectionMode(QAbstractItemView::SingleSelection);
        m_CallStack->setEditTriggers(QAbstractItemView::NoEditTriggers);
        connect(m_CallStack, &QTableWidget::cellClicked, this, &StackDialog::OnFrameSelected);

        auto callstackLayout = new QVBoxLayout();
        callstackLayout->addWidget(callStackLabel);
        callstackLayout->addWidget(m_CallStack);
        auto callstackWidget = new QWidget();
        callstackWidget->setLayout(callstackLayout);

        auto stackFrameLabel = new QLabel("Stack Frame (double-click to edit):");
        m_StackFrame = new QTableWidget(this);
        m_StackFrame->setColumnCount(4);
        m_StackFrame->setHorizontalHeaderLabels({"Type", "Index", "Value Type", "Value"});
        m_StackFrame->horizontalHeader()->setStretchLastSection(true);
        m_StackFrame->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        m_StackFrame->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_StackFrame->setSelectionMode(QAbstractItemView::SingleSelection);
        m_StackFrame->setEditTriggers(QAbstractItemView::NoEditTriggers);
        connect(m_StackFrame, &QTableWidget::cellDoubleClicked, this, &StackDialog::OnEditStack);

        auto stackframeLayout = new QVBoxLayout();
        stackframeLayout->addWidget(stackFrameLabel);
        stackframeLayout->addWidget(m_StackFrame);
        auto stackframeWidget = new QWidget();
        stackframeWidget->setLayout(stackframeLayout);

        auto splitter = new QSplitter(Qt::Vertical);
        splitter->addWidget(callstackWidget);
        splitter->addWidget(stackframeWidget);

        auto mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(splitter);

        PopulateCallstack();
    }

    void StackDialog::PopulateCallstack()
    {
        uint8_t depth = m_Thread->GetCallDepth();

        m_CallStack->setRowCount(depth);
        m_FramePointers.clear();
        m_FramePointers.resize(depth);

        uint32_t fp = m_Thread->GetFp();

        for (int i = depth - 1; i >= 0; --i)
        {
            uint32_t addr = m_Thread->GetCallStack(i);

            auto func = m_Disassembler->GetFunctionForPc(addr);

            m_FramePointers[i] = fp;

            fp = m_Thread->GetStack(fp + func->ArgCount + 1).Get<int32_t>();

            m_CallStack->setItem(i, 0, new QTableWidgetItem(QString("0x%1").arg(QString::number(addr, 16).toUpper())));
            m_CallStack->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(func->Name)));
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
        uint32_t sp = m_Thread->GetSp();
        uint32_t pc = m_Thread->GetCallStack(frameIndex);

        auto func = m_Disassembler->GetFunctionForPc(pc);

        m_StackFrame->setRowCount(0);

        auto addRow = [&](const char* type, int index, Pointer pointer) {
            int row = m_StackFrame->rowCount();
            m_StackFrame->insertRow(row);
            m_StackFrame->setItem(row, 0, new QTableWidgetItem(type));
            m_StackFrame->setItem(row, 1, new QTableWidgetItem(QString::number(index)));

            QComboBox* combo = new QComboBox();
            combo->addItems({"INT", "BITFIELD", "FLOAT", "TEXT_LABEL", "STRING"});
            combo->setFrame(false);
            connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, row, pointer](int type) {
                UpdateStackValue(row, pointer, type);
            });
            m_StackFrame->setCellWidget(row, 2, combo);

            m_StackFrame->setItem(row, 3, new QTableWidgetItem(QString::number(pointer.Get<int32_t>())));
        };

        for (int i = 0; i < func->ArgCount; i++)
            addRow("Arg", i, m_Thread->GetStack(fp + i));

        int localCount = func->FrameSize - func->ArgCount - 2;
        for (int i = 0; i < localCount; i++)
            addRow("Local", func->ArgCount + 2 + i, m_Thread->GetStack(fp + func->ArgCount + 2 + i));

        int tempCount = sp - (fp + func->FrameSize);
        for (int i = 0; i < tempCount; i++)
            addRow("Temp", func->FrameSize + i, m_Thread->GetStack(fp + func->FrameSize + i));
    }

    void StackDialog::UpdateStackValue(int row, Pointer pointer, int type)
    {
        QString text;

        switch (type)
        {
        case ScriptValueType::INT:
            text = QString::number(pointer.Get<int32_t>());
            break;
        case ScriptValueType::BIT_FIELD:
            text = QString("%1").arg(pointer.Get<uint32_t>(), 8, 16, QChar('0')).toUpper();
            break;
        case ScriptValueType::FLOAT:
            text = QString::number(pointer.Get<float>(), 'f', 6);
            break;
        case ScriptValueType::TEXT_LABEL:
            text = QString::fromStdString(pointer.GetString(64));
            break;
        case ScriptValueType::STRING:
            auto str = pointer.Deref();
            if (str)
                text = QString::fromStdString(pointer.Deref().GetString(256));
            else
                text = "";
            break;
        }

        m_StackFrame->item(row, 3)->setText(text);
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
        Pointer ptr = m_Thread->GetStack(fp + index);

        QComboBox* combo = qobject_cast<QComboBox*>(m_StackFrame->cellWidget(row, 2));
        auto type = static_cast<ScriptValueType>(combo->currentIndex());

        EditValueDialog dlg(ptr, type, this);
        if (dlg.exec() == QDialog::Accepted)
        {
            dlg.Apply();
            UpdateStackValue(row, ptr, static_cast<int>(type));
        }
    }
}