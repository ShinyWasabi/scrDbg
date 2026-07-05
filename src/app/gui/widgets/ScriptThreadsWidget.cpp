#include "ScriptThreadsWidget.hpp"
#include "game/Game.hpp"
#include "gui/dialogs/BinarySearchDialog.hpp"
#include "gui/dialogs/BreakpointsDialog.hpp"
#include "gui/dialogs/ExportOptionsDialog.hpp"
#include "gui/dialogs/StackDialog.hpp"
#include "gui/menus/DisassemblyContextMenu.hpp"
#include "gui/models/DisassemblyModel.hpp"
#include "gui/models/FunctionListModel.hpp"
#include "pipe/PipeCommands.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>

namespace scrDbgApp
{
    ScriptThreadsWidget::ScriptThreadsWidget(QWidget* parent)
        : QWidget(parent),
          m_LastThreadId(0),
          m_Disassembler(nullptr),
          m_FunctionFilter(nullptr)
    {
        bool isRdr2 = g_Game->GetType() == GameType::RDR2;
        bool isPayne = g_Game->GetType() == GameType::PAYNE;
        bool isGta5 = g_Game->GetType() == GameType::GTA5_GEN8 || g_Game->GetType() == GameType::GTA5_GEN9;
        bool isRdr3 = g_Game->GetType() == GameType::RDR3;

        m_ScriptNames = new QComboBox(this);
        m_ScriptNames->setEditable(false);

        m_State = new QLabel("State: RUNNING");
        if (isPayne)
            m_State->setToolTip("Current state of this script thread.\n(RUNNING, IDLE, KILLED, PAUSED, REFRESH, THROW)");
        else if (isGta5)
            m_State->setToolTip("Current state of this script thread.\n(RUNNING, IDLE, KILLED, PAUSED)");
        else
            m_State->setToolTip("Current state of this script thread.\n(RUNNING, IDLE, KILLED, PAUSED, REFRESH)");

        m_Priority = new QLabel("Priority: HIGHEST");
        m_Priority->setToolTip("Execution priority of this script thread.\n(HIGHEST, NORMAL, LOWEST, MANUAL_UPDATE)");
        m_Priority->setVisible(isGta5 || isRdr3);

        m_Program = new QLabel("Program: 0");
        m_Program->setToolTip("JOAAT hash of the name of this script thread's program.");

        m_ThreadId = new QLabel("Thread ID: 0");
        m_ThreadId->setToolTip("Unique identifier for this script thread.");

        m_ProgramCounter = new QLabel("Program Counter: 0x0000");
        m_ProgramCounter->setToolTip("Current program counter of the last called native command in this script thread.");

        m_FramePointer = new QLabel("Frame Pointer: 0x0000");
        m_FramePointer->setToolTip("Base of the current stack frame for this script thread.");

        m_StackPointer = new QLabel("Stack Pointer: 0x0000");
        m_StackPointer->setToolTip("Current top of the stack for this script thread.");

        m_CreateTime = new QLabel("Create Time: 00:00:00");
        m_CreateTime->setToolTip("Time passed since the creation of this script thread.");
        m_CreateTime->setVisible(isGta5);

        m_IsPatched = new QLabel("Is Patched: FALSE");
        m_IsPatched->setToolTip("Flag indicating whether this script thread is using the patch data.");
        m_IsPatched->setVisible(isRdr2);

        m_StackSize = new QLabel("Stack Size: 0");
        m_StackSize->setToolTip("Total stack size this script thread needs.");

        m_TypedFlags = new QLabel("Typed Flags: 0");
        m_TypedFlags->setToolTip("Flags set by START_NEW_SCRIPT_TYPED for this script thread.");
        m_TypedFlags->setVisible(isPayne);

        QVector<QLabel*> leftLabels = {m_State, m_Priority, m_Program, m_ThreadId, m_ProgramCounter, m_FramePointer, m_StackPointer, m_CreateTime, m_StackSize, m_IsPatched, m_TypedFlags};
        QVBoxLayout* leftLayout = new QVBoxLayout();
        for (auto lbl : leftLabels)
            leftLayout->addWidget(lbl);
        leftLayout->addStretch();

        m_FullScriptName = new QLabel("Full Name: ");
        m_FullScriptName->setToolTip("Full name of this script program.");
        m_FullScriptName->setVisible(isRdr2);

        m_GlobalVersion = new QLabel("Global Version: 0");
        m_GlobalVersion->setToolTip("Unused. Checks whether two script programs have incompatible globals variables.");
        m_GlobalVersion->setVisible(isGta5 || isRdr2 || isRdr3);

        m_CodeSize = new QLabel("Code Size: 0");
        m_CodeSize->setToolTip("Total size, in bytes, of the bytecode for this script program.");

        m_ArgCount = new QLabel("Arg Count: 0");
        m_ArgCount->setToolTip("Number of arguments this script program's entry function expects.");

        m_StaticCount = new QLabel("Static Count: 0");
        m_StaticCount->setToolTip("Number of static variables defined in this script program.");

        m_GlobalCount = new QLabel("Global Count: 0");
        m_GlobalCount->setToolTip("Number of global variables defined in this script program.");
        m_GlobalCount->setVisible(isGta5 || isRdr3);

        m_GlobalBlock = new QLabel("Global Block: 0");
        m_GlobalBlock->setToolTip("The global block index of this script program's global variables.");
        m_GlobalBlock->setVisible(isGta5 || isRdr3);

        m_NativeCount = new QLabel("Native Count: 0");
        m_NativeCount->setToolTip("Number of native commands that this script program uses.");
        m_NativeCount->setVisible(isGta5 || isRdr2 || isRdr3);

        m_RefCount = new QLabel("Ref Count: 0");
        m_RefCount->setToolTip("Number of references this script program has.");

        m_StringsSize = new QLabel("Strings Size: 0");
        m_StringsSize->setToolTip("Total size, in bytes, of all string literals defined in this script program.");
        m_StringsSize->setVisible(isGta5 || isRdr3);

        m_IsRsc = new QLabel("Is RSC: FALSE");
        m_IsRsc->setToolTip("Flag indicating whether a script is a RSC (.wsc) or .sco script.");
        m_IsRsc->setVisible(isRdr2);

        m_IsPTScript = new QLabel("Is PT Script: FALSE");
        m_IsPTScript->setToolTip("Flag indicating whether a script is a Payne Thresolds script.");
        m_IsPTScript->setVisible(isPayne);

        QVector<QLabel*> rightLabels = {m_FullScriptName, m_GlobalVersion, m_CodeSize, m_ArgCount, m_StaticCount, m_GlobalCount, m_GlobalBlock, m_NativeCount, m_RefCount, m_StringsSize, m_IsRsc, m_IsPTScript};
        QVBoxLayout* rightLayout = new QVBoxLayout();
        for (auto lbl : rightLabels)
            rightLayout->addWidget(lbl);
        rightLayout->addStretch();

        QHBoxLayout* columnsLayout = new QHBoxLayout();
        columnsLayout->addLayout(leftLayout);
        columnsLayout->addSpacing(48);
        columnsLayout->addLayout(rightLayout);
        columnsLayout->addStretch();

        m_TogglePauseScript = new QPushButton("Pause Script");
        connect(m_TogglePauseScript, &QPushButton::clicked, this, &ScriptThreadsWidget::OnTogglePauseScript);

        m_KillScript = new QPushButton("Kill Script");
        m_KillScript->setToolTip("Terminate this script thread.");
        connect(m_KillScript, &QPushButton::clicked, this, &ScriptThreadsWidget::OnKillScript);

        m_ExportOptions = new QPushButton("Export Options");
        m_ExportOptions->setToolTip("View export options.");
        connect(m_ExportOptions, &QPushButton::clicked, this, &ScriptThreadsWidget::OnExportOptionsDialog);

        m_JumpToAddress = new QPushButton("Jump to Address");
        m_JumpToAddress->setToolTip("Jump to a raw address in the disassembly.");
        connect(m_JumpToAddress, &QPushButton::clicked, this, &ScriptThreadsWidget::OnJumpToAddress);

        m_BinarySearch = new QPushButton("Binary Search");
        m_BinarySearch->setToolTip("Search for a byte sequence in the disassembly.");
        connect(m_BinarySearch, &QPushButton::clicked, this, &ScriptThreadsWidget::OnBinarySearch);

        m_ViewStack = new QPushButton("View Stack");
        m_ViewStack->setToolTip("View the current callstack and stack frame of this script thread.");
        connect(m_ViewStack, &QPushButton::clicked, this, &ScriptThreadsWidget::OnViewStack);
        m_ViewStack->setVisible(isGta5 || isRdr3);

        m_ViewBreakpoints = new QPushButton("View Breakpoints");
        m_ViewBreakpoints->setToolTip("View currently set breakpoints.");
        m_BreakpointsPauseGame = new QCheckBox("Breakpoints pause game");
        m_BreakpointsPauseGame->setToolTip("Choose whether a breakpoint should pause the entire game or only its script thread.");
        connect(m_ViewBreakpoints, &QPushButton::clicked, this, &ScriptThreadsWidget::OnBreakpointsDialog);
        connect(m_BreakpointsPauseGame, &QCheckBox::toggled, this, [](bool checked) {
            PipeCommands::SetBreakpointPauseGame(checked);
        });

        QHBoxLayout* buttonsLayout = new QHBoxLayout();
        buttonsLayout->addWidget(m_TogglePauseScript);
        buttonsLayout->addWidget(m_KillScript);
        buttonsLayout->addWidget(m_ExportOptions);
        buttonsLayout->addWidget(m_JumpToAddress);
        buttonsLayout->addWidget(m_BinarySearch);
        buttonsLayout->addWidget(m_ViewStack);
        buttonsLayout->addWidget(m_ViewBreakpoints);
        buttonsLayout->addWidget(m_BreakpointsPauseGame);
        buttonsLayout->addStretch();

        m_FunctionList = new QTableView(this);
        m_FunctionList->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_FunctionList->setSelectionMode(QAbstractItemView::SingleSelection);
        m_FunctionList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_FunctionList->setAlternatingRowColors(true);
        m_FunctionList->horizontalHeader()->setStretchLastSection(true);
        m_FunctionList->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        m_FunctionList->verticalHeader()->setVisible(false);
        m_FunctionList->setStyleSheet("QTableView { font-family: Consolas; font-size: 11pt; }");
        connect(m_FunctionList, &QTableView::doubleClicked, this, [this](const QModelIndex& index) {
            QModelIndex idx = m_FunctionFilter->mapToSource(index);
            uint32_t pc = m_Disassembler->GetFunction(idx.row()).Start;
            ScrollToAddress(pc);
        });

        m_FunctionSearch = new QLineEdit(this);
        m_FunctionSearch->setPlaceholderText("Search function...");
        connect(m_FunctionSearch, &QLineEdit::textChanged, this, [this](const QString& text) {
            if (m_FunctionFilter)
                m_FunctionFilter->setFilterFixedString(text);
        });

        QVBoxLayout* functionLayout = new QVBoxLayout();
        functionLayout->addWidget(m_FunctionList);
        functionLayout->addWidget(m_FunctionSearch);

        m_FunctionWidget = new QWidget(this);
        m_FunctionWidget->setLayout(functionLayout);

        m_DisassemblyInfo = new QLabel(this);
        m_DisassemblyInfo->setText("- <none>");
        m_DisassemblyInfo->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        m_DisassemblyInfo->setStyleSheet("font-weight: bold; padding: 2px;");

        m_Disassembly = new QTableView(this);
        m_Disassembly->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Disassembly->setSelectionMode(QAbstractItemView::SingleSelection);
        m_Disassembly->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_Disassembly->setShowGrid(false);
        m_Disassembly->horizontalHeader()->setStretchLastSection(true);
        m_Disassembly->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        m_Disassembly->verticalHeader()->setVisible(false);
        m_Disassembly->setStyleSheet("QTableView { font-family: Consolas; font-size: 11pt; }");
        m_Disassembly->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_Disassembly, &QWidget::customContextMenuRequested, this, &ScriptThreadsWidget::OnDisassemblyContextMenu);

        QVBoxLayout* disasmLayout = new QVBoxLayout();
        disasmLayout->addWidget(m_Disassembly);
        disasmLayout->addWidget(m_DisassemblyInfo);

        QWidget* disasmWidget = new QWidget(this);
        disasmWidget->setLayout(disasmLayout);

        QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
        splitter->addWidget(m_FunctionWidget);
        splitter->addWidget(disasmWidget);
        splitter->setChildrenCollapsible(false);
        splitter->setStretchFactor(0, 2);
        splitter->setStretchFactor(1, 4);

        m_UpdateTimer = new QTimer(this);
        m_UpdateTimer->setInterval(50);
        connect(m_UpdateTimer, &QTimer::timeout, this, &ScriptThreadsWidget::OnUpdateScripts);
        m_UpdateTimer->start();

        QVBoxLayout* scriptThreadsLayout = new QVBoxLayout(this);
        scriptThreadsLayout->addWidget(m_ScriptNames);
        scriptThreadsLayout->addLayout(columnsLayout);
        scriptThreadsLayout->addLayout(buttonsLayout);
        scriptThreadsLayout->addWidget(splitter, 1);
        scriptThreadsLayout->addStretch();
        setLayout(scriptThreadsLayout);
    }

    uint32_t ScriptThreadsWidget::GetCurrentScriptHash()
    {
        if (m_ScriptNames->currentIndex() < 0)
            return 0;
        return m_ScriptNames->currentData().toUInt();
    }

    bool ScriptThreadsWidget::ScrollToAddress(uint32_t address)
    {
        QModelIndex idx;
        for (int i = 0; i < m_Disassembly->model()->rowCount(); i++)
        {
            uint32_t pc = m_Disassembler->GetInstruction(i);
            uint32_t size = m_Disassembler->GetInstructionSize(pc);
            if (address >= pc && address < pc + size)
            {
                idx = m_Disassembly->model()->index(i, 0);
                break;
            }
        }

        if (!idx.isValid())
            return false;

        m_Disassembly->scrollTo(idx, QAbstractItemView::PositionAtCenter);
        m_Disassembly->setCurrentIndex(idx);
        m_Disassembly->setFocus(Qt::OtherFocusReason);
        return true;
    }

    void ScriptThreadsWidget::UpdateDisassemblyInfo(int row, bool includeDesc)
    {
        auto& code = m_Disassembler->GetCode();
        uint32_t pc = m_Disassembler->GetInstruction(row);

        QString text;
        if (auto func = m_Disassembler->GetFunctionForPc(pc))
        {
            QString name = QString::fromStdString(func->Name);
            uint32_t offset = pc - func->Start;
            text = QString("%1+0x%2").arg(name).arg(QString::number(offset, 16).toUpper());
        }
        else
        {
            text = QString("+0x%2 (dead)").arg(QString::number(pc, 16).toUpper());
        }

        if (includeDesc)
        {
            QString desc = QString::fromStdString(m_Disassembler->GetInstructionDescription(code[pc]));
            if (!desc.isEmpty())
                text += QString(" | %1").arg(desc);
        }

        m_DisassemblyInfo->setText(text);
    }

    void ScriptThreadsWidget::CleanupDisassembly()
    {
        m_Disassembler.reset();

        if (m_Disassembly->model())
            m_Disassembly->setModel(nullptr);

        if (m_FunctionFilter)
        {
            m_FunctionList->setModel(nullptr);
            delete m_FunctionFilter;
            m_FunctionFilter = nullptr;
        }

        m_DisassemblyInfo->setText("- <none>");
        m_FunctionSearch->clear();
    }

    void ScriptThreadsWidget::RefreshDisassembly(std::unique_ptr<ScriptProgram> program)
    {
        CleanupDisassembly();

        m_Disassembler = g_Game->CreateDisassembly(std::move(program));

        auto disasmModel = new DisassemblyModel(m_Disassembler.get(), m_Disassembly);
        m_Disassembly->setModel(disasmModel);
        m_Disassembly->setColumnWidth(0, 100);
        m_Disassembly->setColumnWidth(1, 150);
        m_Disassembly->setColumnWidth(2, 400);

        // Fire once to initialize
        OnUpdateDisassemblyInfoByScroll();
        connect(m_Disassembly->verticalScrollBar(), &QScrollBar::valueChanged, this, &ScriptThreadsWidget::OnUpdateDisassemblyInfoByScroll);
        connect(m_Disassembly->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ScriptThreadsWidget::OnUpdateDisassemblyInfoBySelection);

        auto funcModel = new FunctionListModel(m_Disassembler.get(), m_FunctionList);
        m_FunctionFilter = new QSortFilterProxyModel(this);
        m_FunctionFilter->setSourceModel(funcModel);
        m_FunctionFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_FunctionFilter->setFilterKeyColumn(0);
        m_FunctionList->setModel(m_FunctionFilter);
    }

    void ScriptThreadsWidget::UpdateCurrentScript()
    {
        uint32_t hash = GetCurrentScriptHash();

        auto thread = g_Game->GetThread(hash);
        if (!thread)
            return;

        auto state = thread->GetState();
        auto activeBp = PipeCommands::GetActiveBreakpoint();

        m_BreakpointsPauseGame->setEnabled(!activeBp.has_value());

        bool isGlobalBreakpointPause = activeBp.has_value() && m_BreakpointsPauseGame->isChecked();
        bool isLocalBreakpoint = activeBp.has_value() && activeBp->first == hash;

        if (isGlobalBreakpointPause || isLocalBreakpoint)
        {
            m_TogglePauseScript->setText("Resume Breakpoint");
            m_TogglePauseScript->setToolTip("Resume the active breakpoint.");
        }
        else if (state == ScriptThread::PAUSED)
        {
            m_TogglePauseScript->setText("Resume Script");
            m_TogglePauseScript->setToolTip("Resume the execution of this script thread.");
        }
        else
        {
            m_TogglePauseScript->setText("Pause Script");
            m_TogglePauseScript->setToolTip("Pause the execution of this script thread.");
        }

        // clang-format off
        QString stateStr = (state == ScriptThread::RUNNING) ? "RUNNING" : (state == ScriptThread::WAITING) ? "WAITING" : (state == ScriptThread::PAUSED) ? "PAUSED" : (state == ScriptThread::REFRESH) ? "REFRESH" : (state == ScriptThread::THROW) ? "THROW" : "KILLED";
        bool showBreakpointActive = isGlobalBreakpointPause || isLocalBreakpoint;
        m_State->setText("State: " + stateStr + (showBreakpointActive ? " (breakpoint active)" : ""));

        auto priority = thread->GetPriority();
        QString priorityStr = (priority == ScriptThread::HIGHEST) ? "HIGHEST" : (priority == ScriptThread::NORMAL) ? "NORMAL" : (priority == ScriptThread::LOWEST) ? "LOWEST" : "MANUAL_UPDATE";
        m_Priority->setText("Priority: " + priorityStr);
        // clang-format on

        uint32_t id = thread->GetId();

        m_Program->setText(QString("Program: %1").arg(thread->GetProgramHash()));
        m_ThreadId->setText(QString("Thread ID: %1").arg(id));
        m_ProgramCounter->setText(QString("Program Counter: 0x%1").arg(QString::number(thread->GetPc(), 16).toUpper()));
        m_FramePointer->setText(QString("Frame Pointer: 0x%1").arg(QString::number(thread->GetFp(), 16).toUpper()));
        m_StackPointer->setText(QString("Stack Pointer: 0x%1").arg(QString::number(thread->GetSp(), 16).toUpper()));
        m_StackSize->setText(QString("Stack Size: %1").arg(thread->GetStackSize()));
        m_TypedFlags->setText(QString("Typed Flags: %1").arg(thread->GetTypedFlags()));
        m_CreateTime->setText(QString("Create Time: %1").arg(thread->GetCreateTime()));
        m_CreateTime->setText(QString("Is Patched: %1").arg(thread->IsPatched() ? "TRUE" : "FALSE"));

        auto program = g_Game->GetProgram(thread->GetProgramHash());
        if (!program)
            return;

        m_FullScriptName->setText(QString("Full Name: %1").arg(program->GetName()));
        m_GlobalVersion->setText(QString("Global Version: %1").arg(program->GetGlobalVersion()));
        m_CodeSize->setText(QString("Code Size: %1").arg(program->GetCodeSize()));
        m_ArgCount->setText(QString("Arg Count: %1").arg(program->GetArgCount()));
        m_StaticCount->setText(QString("Static Count: %1").arg(program->GetStaticCount()));
        m_GlobalCount->setText(QString("Global Count: %1").arg(program->GetGlobalCount()));
        m_GlobalBlock->setText(QString("Global Block: %1").arg(program->GetGlobalBlock()));
        m_NativeCount->setText(QString("Native Count: %1").arg(program->GetNativeCount()));
        m_RefCount->setText(QString("Ref Count: %1").arg(program->GetRefCount()));
        m_StringsSize->setText(QString("String Size: %1").arg(program->GetStringsSize()));
        m_IsRsc->setText(QString("Is RSC: %1").arg(program->IsRsc() ? "TRUE" : "FALSE"));
        m_IsPTScript->setText(QString("Is PT Script: %1").arg(program->IsPTScript() ? "TRUE" : "FALSE"));

        if (m_LastThreadId != id)
        {
            m_LastThreadId = id;
            RefreshDisassembly(std::move(program));
        }
    }

    void ScriptThreadsWidget::OnUpdateScripts()
    {
        QString currentScript = m_ScriptNames->currentText();

        std::vector<std::pair<std::string, uint32_t>> aliveScripts;
        for (const auto& t : g_Game->GetThreads())
        {
            if (t->GetState() == ScriptThread::KILLED || t->GetStackSize() == 0)
                continue;

            std::string name = t->GetScriptName();
            if (!name.empty())
                aliveScripts.push_back({name, t->GetScriptHash()});
        }

        bool changed = (m_ScriptNames->count() != static_cast<int>(aliveScripts.size()));
        if (!changed)
        {
            for (int i = 0; i < m_ScriptNames->count(); ++i)
            {
                if (m_ScriptNames->itemText(i).toStdString() != aliveScripts[i].first)
                {
                    changed = true;
                    break;
                }
            }
        }

        if (changed)
        {
            m_ScriptNames->blockSignals(true);
            m_ScriptNames->clear();
            for (auto& script : aliveScripts)
                m_ScriptNames->addItem(QString::fromStdString(script.first), QVariant::fromValue(script.second));
            m_ScriptNames->blockSignals(false);

            int newIndex = m_ScriptNames->findText(currentScript);
            if (newIndex != -1)
                m_ScriptNames->setCurrentIndex(newIndex);
            else if (m_ScriptNames->count() > 0)
                m_ScriptNames->setCurrentIndex(0);
        }

        if (aliveScripts.empty())
        {
            CleanupDisassembly();
            return;
        }

        UpdateCurrentScript();
    }

    void ScriptThreadsWidget::OnTogglePauseScript()
    {
        auto hash = GetCurrentScriptHash();
        auto thread = g_Game->GetThread(hash);
        if (!thread)
            return;

        auto activeBp = PipeCommands::GetActiveBreakpoint();
        bool isGlobalBreakpointPause = activeBp.has_value() && m_BreakpointsPauseGame->isChecked();
        bool isLocalBreakpoint = activeBp.has_value() && activeBp->first == hash;

        if (isGlobalBreakpointPause || isLocalBreakpoint)
            PipeCommands::ResumeBreakpoint();
        else if (thread->GetState() == ScriptThread::PAUSED)
            thread->SetState(ScriptThread::RUNNING);
        else
            thread->SetState(ScriptThread::PAUSED);
    }

    void ScriptThreadsWidget::OnKillScript()
    {
        if (auto thread = g_Game->GetThread(GetCurrentScriptHash()))
        {
            thread->SetState(ScriptThread::KILLED);
            QMessageBox::information(this, "Kill Script", QString("Kill Reason: %1").arg(thread->GetKillReason().c_str()));
        }
    }

    void ScriptThreadsWidget::OnExportOptionsDialog()
    {
        ExportOptionsDialog dlg(GetCurrentScriptHash(), m_Disassembly, this);
        dlg.exec();
    }

    void ScriptThreadsWidget::OnJumpToAddress()
    {
        if (!GetCurrentScriptHash())
            return;

        bool ok = false;
        QString input = QInputDialog::getText(this, "Jump to Address", "Enter address (hex):", QLineEdit::Normal, "", &ok);
        if (!ok || input.isEmpty())
            return;

        bool okHex = false;
        uint32_t addr = input.toUInt(&okHex, 16);
        if (!okHex)
        {
            QMessageBox::warning(this, "Invalid Input", "Please enter a valid hex address.");
            return;
        }

        if (!ScrollToAddress(addr))
            QMessageBox::warning(this, "Not Found", "No instruction at this address.");
    }

    void ScriptThreadsWidget::OnBinarySearch()
    {
        if (!GetCurrentScriptHash())
            return;

        BinarySearchDialog dlg(this);
        dlg.Execute(m_Disassembler.get(), [this](uint32_t addr) {
            ScrollToAddress(addr);
        });
    }

    void ScriptThreadsWidget::OnViewStack()
    {
        auto thread = g_Game->GetThread(GetCurrentScriptHash());
        if (!thread)
            return;

        StackDialog dlg(std::move(thread), m_Disassembler.get(), this);
        dlg.exec();
    }

    void ScriptThreadsWidget::OnBreakpointsDialog()
    {
        BreakpointsDialog dlg(this);
        connect(&dlg, &BreakpointsDialog::BreakpointDoubleClicked, this, [this, &dlg](uint32_t script, uint32_t pc) {
            int idx = m_ScriptNames->findData(script);
            if (idx == -1)
            {
                QMessageBox::warning(this, "Script not found", "Script does not exist.");
                return;
            }

            m_ScriptNames->setCurrentIndex(idx);

            // wait a bit
            QTimer::singleShot(100, this, [this, pc]() {
                ScrollToAddress(pc);
            });
            dlg.close();
        });
        dlg.exec();
    }

    void ScriptThreadsWidget::OnUpdateDisassemblyInfoByScroll()
    {
        int row = m_Disassembly->indexAt(QPoint(0, 0)).row();
        if (row >= 0)
            UpdateDisassemblyInfo(row, false);
    }

    void ScriptThreadsWidget::OnUpdateDisassemblyInfoBySelection()
    {
        auto selected = m_Disassembly->selectionModel()->selectedIndexes();
        if (!selected.isEmpty())
            UpdateDisassemblyInfo(selected.first().row(), true);
    }

    void ScriptThreadsWidget::OnDisassemblyContextMenu(const QPoint& pos)
    {
        QModelIndex index = m_Disassembly->indexAt(pos);
        if (!index.isValid())
            return;

        DisassemblyContextMenu menu(index, m_Disassembler.get(), GetCurrentScriptHash(), this);
        connect(&menu, &DisassemblyContextMenu::JumpToAddressRequested, this, &ScriptThreadsWidget::OnJumpToAddressFromMenu);
        menu.exec(m_Disassembly->viewport()->mapToGlobal(pos));
    }

    void ScriptThreadsWidget::OnJumpToAddressFromMenu(uint32_t address)
    {
        ScrollToAddress(address);
    }
}