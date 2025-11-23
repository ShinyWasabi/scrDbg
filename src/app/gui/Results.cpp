#include "Results.hpp"
#include <QHeaderView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

namespace scrDbgApp
{
    ResultsDialog::ResultsDialog(const QString& title, const std::vector<Entry>& entries, QWidget* parent)
        : QDialog(parent)
    {
        setWindowTitle(title);
        resize(600, 400);
        setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint);

        m_Model = new QStandardItemModel(this);
        m_Model->setHorizontalHeaderLabels({"Address", "Function", "Instruction"});

        for (const auto& e : entries)
        {
            QList<QStandardItem*> row;
            row << new QStandardItem(QString("0x%1").arg(QString::number(e.Address, 16).toUpper()))
                << new QStandardItem(QString::fromStdString(e.FunctionName))
                << new QStandardItem(QString::fromStdString(e.Instruction));
            m_Model->appendRow(row);
        }

        m_Filter = new QSortFilterProxyModel(this);
        m_Filter->setSourceModel(m_Model);
        m_Filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_Filter->setFilterKeyColumn(-1); // Search in all columns

        m_Table = new QTableView(this);
        m_Table->setModel(m_Filter);
        m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Table->setSelectionMode(QAbstractItemView::SingleSelection);
        m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_Table->setAlternatingRowColors(true);
        m_Table->horizontalHeader()->setStretchLastSection(true);
        m_Table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        m_Table->verticalHeader()->setVisible(false);
        m_Table->setStyleSheet("QTableView { font-family: Consolas; font-size: 11pt; }");
        connect(m_Table, &QTableView::doubleClicked, this, &ResultsDialog::OnItemDoubleClicked);

        m_SearchBox = new QLineEdit(this);
        m_SearchBox->setPlaceholderText("Search...");
        connect(m_SearchBox, &QLineEdit::textChanged, this, [this](const QString& text) {
            if (m_Filter)
                m_Filter->setFilterFixedString(text);
        });

        auto layout = new QVBoxLayout(this);
        layout->addWidget(m_Table);
        layout->addWidget(m_SearchBox);
        setLayout(layout);
    }

    void ResultsDialog::OnItemDoubleClicked(const QModelIndex& index)
    {
        QModelIndex srcIndex = m_Filter->mapToSource(index);
        QString addrText = m_Model->item(srcIndex.row(), 0)->text();

        bool ok = false;
        uint32_t address = addrText.toUInt(&ok, 16);
        if (ok)
            emit EntryDoubleClicked(address);
    }
}