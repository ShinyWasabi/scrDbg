#include "BreakpointsDialog.hpp"
#include "pipe/PipeCommands.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QTableWidget>
#include <QVBoxLayout>

namespace scrDbgApp
{
    static std::unordered_map<uint32_t, std::string> g_KnownNames;

    static std::unordered_map<uint32_t, std::string> GetRunningScripts()
    {
        std::unordered_map<uint32_t, std::string> scripts;
        for (const auto& thread : g_Game->GetThreads())
        {
            if (thread->GetState() == ScriptThread::KILLED || thread->GetStackSize() == 0)
                continue;

            std::string name = thread->GetScriptName();
            if (!name.empty())
                scripts.emplace(thread->GetScriptHash(), std::move(name));
        }

        return scripts;
    }

    static QString FormatHash(uint32_t hash)
    {
        return QString("0x%1").arg(QString::number(hash, 16).toUpper(), 8, QLatin1Char('0'));
    }

    uint32_t BreakpointsDialog::ResolveScriptHash(const QString& text)
    {
        QString name = text.trimmed();
        if (name.isEmpty())
            return 0;

        if (name.startsWith("0x", Qt::CaseInsensitive))
        {
            bool ok = false;
            uint32_t hash = name.mid(2).toUInt(&ok, 16);
            return ok ? hash : 0;
        }

        std::string str = name.toStdString();
        for (const auto& [hash, running] : GetRunningScripts())
        {
            if (QString::fromStdString(running).compare(name, Qt::CaseInsensitive) == 0)
                return hash;
        }

        return Joaat(str);
    }

    BreakpointsDialog::BreakpointsDialog(bool pauseGame, bool canChangePauseGame, QWidget* parent)
        : QDialog(parent)
    {
        setWindowTitle("Breakpoints");
        resize(520, 420);
        setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint);

        m_Table = new QTableWidget(this);
        m_Table->setColumnCount(3);
        m_Table->setHorizontalHeaderLabels({"Script", "Hash", "Address"});
        m_Table->horizontalHeader()->setStretchLastSection(true);
        m_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        m_Table->verticalHeader()->setVisible(false);
        m_Table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Table->setSelectionMode(QAbstractItemView::SingleSelection);
        m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_Table->setToolTip("Double-click to show a breakpoint in its script's disassembly.");
        connect(m_Table, &QTableWidget::cellDoubleClicked, this, &BreakpointsDialog::OnItemDoubleClicked);
        connect(m_Table, &QTableWidget::itemSelectionChanged, this, [this]() {
            m_Remove->setEnabled(!m_Table->selectedItems().isEmpty());
        });

        m_Remove = new QPushButton("Remove", this);
        m_Remove->setEnabled(false);
        connect(m_Remove, &QPushButton::clicked, this, &BreakpointsDialog::OnRemoveClicked);

        m_RemoveAll = new QPushButton("Remove All", this);
        connect(m_RemoveAll, &QPushButton::clicked, this, &BreakpointsDialog::OnRemoveAllClicked);

        m_PauseGame = new QCheckBox("Breakpoints pause game", this);
        m_PauseGame->setChecked(pauseGame);
        m_PauseGame->setEnabled(canChangePauseGame);
        m_PauseGame->setToolTip(canChangePauseGame ? "Choose whether a breakpoint should pause the entire game or only its script thread." : "Can't be changed while a breakpoint is active.");
        connect(m_PauseGame, &QCheckBox::toggled, this, &BreakpointsDialog::PauseGameChanged);

        auto listButtons = new QHBoxLayout();
        listButtons->addWidget(m_Remove);
        listButtons->addWidget(m_RemoveAll);
        listButtons->addStretch();
        listButtons->addWidget(m_PauseGame);

        m_Script = new QComboBox(this);
        m_Script->setEditable(true);
        m_Script->setInsertPolicy(QComboBox::NoInsert);
        m_Script->lineEdit()->setPlaceholderText("Script name, or hex hash");
        m_Script->setToolTip("Name of the script. It doesn't have to be running yet; the breakpoint\ntriggers once a script with this name executes the address.");
        connect(m_Script, &QComboBox::editTextChanged, this, &BreakpointsDialog::OnScriptTextChanged);

        m_Pc = new QLineEdit(this);
        m_Pc->setPlaceholderText("Address (hex)");
        m_Pc->setValidator(new QRegularExpressionValidator(QRegularExpression("(0[xX])?[0-9A-Fa-f]{1,8}"), m_Pc));
        connect(m_Pc, &QLineEdit::returnPressed, this, &BreakpointsDialog::OnAddClicked);

        m_ResolvedHash = new QLabel(this);
        m_ResolvedHash->setStyleSheet("color: gray;");

        m_Add = new QPushButton("Add Breakpoint", this);
        connect(m_Add, &QPushButton::clicked, this, &BreakpointsDialog::OnAddClicked);

        auto form = new QFormLayout();
        form->addRow("Script:", m_Script);
        form->addRow("", m_ResolvedHash);
        form->addRow("Address:", m_Pc);

        auto addLayout = new QVBoxLayout();
        addLayout->addLayout(form);
        addLayout->addWidget(m_Add, 0, Qt::AlignRight);

        auto addGroup = new QGroupBox("Add Breakpoint", this);
        addGroup->setLayout(addLayout);

        auto layout = new QVBoxLayout(this);
        layout->addWidget(m_Table, 1);
        layout->addLayout(listButtons);
        layout->addWidget(addGroup);
        setLayout(layout);

        RefreshScriptNames();
        RefreshBreakpoints();
        OnScriptTextChanged(m_Script->currentText());
    }

    void BreakpointsDialog::RefreshScriptNames()
    {
        QStringList names;
        for (const auto& [hash, name] : GetRunningScripts())
            names << QString::fromStdString(name);
        names.sort(Qt::CaseInsensitive);

        m_Script->blockSignals(true);
        m_Script->clear();
        m_Script->addItems(names);
        m_Script->setCurrentIndex(-1);
        m_Script->setEditText({});
        m_Script->blockSignals(false);
    }

    void BreakpointsDialog::OnScriptTextChanged(const QString& text)
    {
        uint32_t hash = ResolveScriptHash(text);
        if (!hash)
        {
            m_ResolvedHash->setText("Hash: -");
            return;
        }

        auto running = GetRunningScripts();
        bool isRunning = running.count(hash) != 0;
        m_ResolvedHash->setText(QString("Hash: %1%2").arg(FormatHash(hash), isRunning ? " (running)" : " (not running)"));
    }

    void BreakpointsDialog::RefreshBreakpoints()
    {
        m_Table->setRowCount(0);

        auto running = GetRunningScripts();
        auto breakpoints = PipeCommands::GetAllBreakpoints();
        auto active = PipeCommands::GetActiveBreakpoint();

        for (const auto& [script, pc] : breakpoints)
        {
            int row = m_Table->rowCount();
            m_Table->insertRow(row);

            QString name;
            if (auto it = running.find(script); it != running.end())
                name = QString::fromStdString(it->second);
            else if (auto known = g_KnownNames.find(script); known != g_KnownNames.end())
                name = QString::fromStdString(known->second) + " (not running)";
            else
                name = "<unknown>";

            auto nameItem = new QTableWidgetItem(name);
            auto hashItem = new QTableWidgetItem(FormatHash(script));
            auto pcItem = new QTableWidgetItem(QString::number(pc, 16).toUpper());
            nameItem->setData(Qt::UserRole, script);
            pcItem->setData(Qt::UserRole, pc);

            if (active.has_value() && active->first == script && active->second == pc)
            {
                QBrush brush(QColor(0xC8, 0xF0, 0xC8));
                nameItem->setBackground(brush);
                hashItem->setBackground(brush);
                pcItem->setBackground(brush);
            }

            m_Table->setItem(row, 0, nameItem);
            m_Table->setItem(row, 1, hashItem);
            m_Table->setItem(row, 2, pcItem);
        }

        m_RemoveAll->setEnabled(m_Table->rowCount() > 0);
    }

    void BreakpointsDialog::OnAddClicked()
    {
        const QString scriptText = m_Script->currentText().trimmed();
        const uint32_t hash = ResolveScriptHash(scriptText);
        if (!hash)
        {
            QMessageBox::warning(this, "Add Breakpoint", "Enter a script name or a hex hash.");
            return;
        }

        QString pcText = m_Pc->text().trimmed();
        if (pcText.startsWith("0x", Qt::CaseInsensitive))
            pcText = pcText.mid(2);

        bool ok = false;
        uint32_t pc = pcText.toUInt(&ok, 16);
        if (!ok || pcText.isEmpty())
        {
            QMessageBox::warning(this, "Add Breakpoint", "Enter a valid hex address.");
            return;
        }

        if (!scriptText.startsWith("0x", Qt::CaseInsensitive))
            g_KnownNames[hash] = scriptText.toStdString();

        PipeCommands::SetBreakpoint(hash, pc, true);
        RefreshBreakpoints();
        m_Pc->clear();
    }

    void BreakpointsDialog::OnRemoveClicked()
    {
        auto selected = m_Table->selectedItems();
        if (selected.isEmpty())
            return;

        int row = selected.first()->row();
        uint32_t script = m_Table->item(row, 0)->data(Qt::UserRole).toUInt();
        uint32_t pc = m_Table->item(row, 2)->data(Qt::UserRole).toUInt();

        PipeCommands::SetBreakpoint(script, pc, false);
        RefreshBreakpoints();
    }

    void BreakpointsDialog::OnItemDoubleClicked(int row, int column)
    {
        Q_UNUSED(column);

        uint32_t script = m_Table->item(row, 0)->data(Qt::UserRole).toUInt();
        uint32_t pc = m_Table->item(row, 2)->data(Qt::UserRole).toUInt();
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