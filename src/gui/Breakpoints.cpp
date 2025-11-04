#include "Breakpoints.hpp"
#include "pipe/PipeCommands.hpp"
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

namespace scrDbg
{
    BreakpointsDialog::BreakpointsDialog(QWidget* parent) :
        QDialog(parent)
    {
        setWindowTitle("Breakpoints");
        resize(400, 300);
        setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint);

        m_Table = new QTableWidget(this);
        m_Table->setColumnCount(2);
        m_Table->setHorizontalHeaderLabels({ "Script", "Address" });
        m_Table->horizontalHeader()->setStretchLastSection(true);
        m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);

        connect(m_Table, &QTableWidget::cellDoubleClicked, this, &BreakpointsDialog::OnItemDoubleClicked);

        m_RemoveAll = new QPushButton("Remove All", this);
        connect(m_RemoveAll, &QPushButton::clicked, this, &BreakpointsDialog::OnRemoveAllClicked);

        auto layout = new QVBoxLayout(this);
        layout->addWidget(m_Table);
        layout->addWidget(m_RemoveAll);
        setLayout(layout);

        RefreshBreakpoints();
    }

    void BreakpointsDialog::RefreshBreakpoints()
    {
        m_Table->setRowCount(0);

        auto breakpoints = PipeCommands::GetAllBreakpoints();
        for (const auto& [script, pc] : breakpoints)
        {
            int row = m_Table->rowCount();
            m_Table->insertRow(row);
            m_Table->setItem(row, 0, new QTableWidgetItem(QString::number(script, 16).toUpper()));
            m_Table->setItem(row, 1, new QTableWidgetItem(QString::number(pc, 16).toUpper()));
        }
    }

    void BreakpointsDialog::OnItemDoubleClicked(int row, int column)
    {
        Q_UNUSED(column);
        if (row < 0)
            return;

        bool ok1, ok2;
        uint32_t script = m_Table->item(row, 0)->text().toUInt(&ok1, 16);
        uint32_t pc = m_Table->item(row, 1)->text().toUInt(&ok2, 16);

        if (ok1 && ok2)
            emit BreakpointDoubleClicked(script, pc);
    }

    void BreakpointsDialog::OnRemoveAllClicked()
    {
        if (QMessageBox::question(this, "Confirm", "Remove all breakpoints?") != QMessageBox::Yes)
            return;

        PipeCommands::RemoveAllBreakpoints();

        RefreshBreakpoints();
    }
}