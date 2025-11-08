#include "Xrefs.hpp"
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHeaderView>

namespace scrDbg
{
    XrefsDialog::XrefsDialog(const std::vector<std::pair<uint32_t, std::string>>& xrefs, QWidget* parent) :
        QDialog(parent)
    {
        setWindowTitle("Xrefs");
        resize(500, 350);
        setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint);

        m_Table = new QTableWidget(this);
        m_Table->setColumnCount(2);
        m_Table->setHorizontalHeaderLabels({ "Address", "Instruction" });
        m_Table->horizontalHeader()->setStretchLastSection(true);
        m_Table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Table->setSelectionMode(QAbstractItemView::SingleSelection);
        m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        connect(m_Table, &QTableWidget::cellDoubleClicked, this, &XrefsDialog::OnItemDoubleClicked);

        for (const auto& [addr, insn] : xrefs)
        {
            int row = m_Table->rowCount();
            m_Table->insertRow(row);
            m_Table->setItem(row, 0, new QTableWidgetItem(QString("0x%1").arg(QString::number(addr, 16).toUpper())));
            m_Table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(insn)));
        }

        auto layout = new QVBoxLayout(this);
        layout->addWidget(m_Table);
        setLayout(layout);
    }

    void XrefsDialog::OnItemDoubleClicked(int row, int column)
    {
        Q_UNUSED(column);
        if (row < 0)
            return;

        bool ok = false;
        uint32_t address = m_Table->item(row, 0)->text().toUInt(&ok, 16);
        if (ok)
            emit XrefDoubleClicked(address);
    }
}