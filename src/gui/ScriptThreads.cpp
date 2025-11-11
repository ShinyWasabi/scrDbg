#include "ScriptThreads.hpp"
#include "Breakpoints.hpp"
#include "Disassembly.hpp"
#include "FunctionList.hpp"
#include "Pointers.hpp"
#include "Stack.hpp"
#include "Xrefs.hpp"
#include "game/gta/Natives.hpp"
#include "game/gta/TextLabels.hpp"
#include "game/rage/Joaat.hpp"
#include "game/rage/scrOpcode.hpp"
#include "game/rage/scrThread.hpp"
#include "pipe/PipeCommands.hpp"
#include "script/ScriptDisassembler.hpp"
#include "util/ScriptHelpers.hpp"
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QFileDialog>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>

namespace scrDbg
{
    ScriptThreadsWidget::ScriptThreadsWidget(QWidget* parent)
        : QWidget(parent),
          m_LastThreadId(0),
          m_Layout(nullptr),
          m_FunctionFilter(nullptr)
    {
        m_ScriptNames = new QComboBox(this);
        m_ScriptNames->setEditable(false);

        m_State = new QLabel("State: RUNNING");
        m_State->setToolTip("Current state of this script thread.\n(RUNNING, IDLE, KILLED, PAUSED)");
        m_Priority = new QLabel("Priority: HIGHEST");
        m_Priority->setToolTip("Execution priority of this script thread.\n(HIGHEST, NORMAL, LOWEST, MANUAL_UPDATE)");
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
        m_StackSize = new QLabel("Stack Size: 0");
        m_StackSize->setToolTip("Total stack size this script thread needs.");

        QVector<QLabel*> leftLabels = {m_State, m_Priority, m_Program, m_ThreadId, m_ProgramCounter, m_FramePointer, m_StackPointer, m_StackSize};
        QVBoxLayout* leftLayout = new QVBoxLayout();
        for (auto* lbl : leftLabels)
            leftLayout->addWidget(lbl);

        m_GlobalVersion = new QLabel("Global Version: 0");
        m_GlobalVersion->setToolTip("Unused. Checks whether two script programs have incompatible globals variables.");
        m_CodeSize = new QLabel("Code Size: 0");
        m_CodeSize->setToolTip("Total size, in bytes, of the bytecode for this script program.");
        m_ArgCount = new QLabel("Arg Count: 0");
        m_ArgCount->setToolTip("Number of arguments this script program's entry function expects.");
        m_StaticCount = new QLabel("Static Count: 0");
        m_StaticCount->setToolTip("Number of static variables defined in this script program.");
        m_GlobalCount = new QLabel("Global Count: 0");
        m_GlobalCount->setToolTip("Number of global variables defined in this script program.");
        m_GlobalBlock = new QLabel("Global Block: 0");
        m_GlobalCount->setToolTip("The global block index of this script program's global variables.");
        m_NativeCount = new QLabel("Native Count: 0");
        m_NativeCount->setToolTip("Number of native commands that this script program uses.");
        m_StringCount = new QLabel("String Count: 0");
        m_StringCount->setToolTip("Total size, in bytes, of all string literals defined in this script program.");

        QVector<QLabel*> rightLabels = {m_GlobalVersion, m_CodeSize, m_ArgCount, m_StaticCount, m_GlobalCount, m_GlobalBlock, m_NativeCount, m_StringCount};
        QVBoxLayout* rightLayout = new QVBoxLayout();
        for (auto* lbl : rightLabels)
            rightLayout->addWidget(lbl);

        QHBoxLayout* columnsLayout = new QHBoxLayout();
        columnsLayout->addLayout(leftLayout);
        columnsLayout->addLayout(rightLayout);

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
        m_ViewBreakpoints = new QPushButton("View Breakpoints");
        m_ViewBreakpoints->setToolTip("View currently set breakpoints.");
        m_BreakpointsPauseGame = new QCheckBox("Breakpoints pause game");
        m_BreakpointsPauseGame->setToolTip("Choose whether a breakpoint should pause the entire game or only its script thread.");
        m_ViewBreakpoints->setEnabled(g_BreakpointsSupported);
        m_BreakpointsPauseGame->setEnabled(g_BreakpointsSupported);
        if (g_BreakpointsSupported)
        {
            connect(m_ViewBreakpoints, &QPushButton::clicked, this, &ScriptThreadsWidget::OnBreakpointsDialog);
            connect(m_BreakpointsPauseGame, &QCheckBox::toggled, this, [](bool checked) {
                PipeCommands::SetBreakpointPauseGame(checked);
            });
        }

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(m_TogglePauseScript);
        buttonLayout->addWidget(m_KillScript);
        buttonLayout->addWidget(m_ExportOptions);
        buttonLayout->addWidget(m_JumpToAddress);
        buttonLayout->addWidget(m_BinarySearch);
        buttonLayout->addWidget(m_ViewStack);
        buttonLayout->addWidget(m_ViewBreakpoints);
        buttonLayout->addWidget(m_BreakpointsPauseGame);
        buttonLayout->addStretch();

        m_FunctionSearch = new QLineEdit(this);
        m_FunctionSearch->setPlaceholderText("Search function...");
        connect(m_FunctionSearch, &QLineEdit::textChanged, this, [this](const QString& text) {
            if (m_FunctionFilter)
                m_FunctionFilter->setFilterFixedString(text);
        });

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
            uint32_t pc = m_Layout->GetFunction(idx.row()).Start;
            ScrollToAddress(pc);
        });

        QVBoxLayout* functionLayout = new QVBoxLayout();
        functionLayout->addWidget(m_FunctionList);
        functionLayout->addWidget(m_FunctionSearch);

        QWidget* functionWidget = new QWidget(this);
        functionWidget->setLayout(functionLayout);

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
        splitter->addWidget(functionWidget);
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
        scriptThreadsLayout->addLayout(buttonLayout);
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
        for (int i = 0; i < m_Disassembly->model()->rowCount(); ++i)
        {
            uint32_t pc = m_Layout->GetInstruction(i).Pc;
            uint32_t size = ScriptHelpers::GetInstructionSize(m_Layout->GetCode(), pc);

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

    void ScriptThreadsWidget::ExportToFile(const QString& title, const QString& filename, int count, std::function<void(QTextStream&, QProgressDialog&)> cb)
    {
        QString name = QFileDialog::getSaveFileName(this, title, filename, "Text Files (*.txt);;All Files (*)");
        if (name.isEmpty())
            return;

        QFile file(name);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(this, title, "Failed to open file for writing.");
            return;
        }

        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);

        QProgressDialog progress(QString("Exporting %1...").arg(title), "Cancel", 0, count, this);
        progress.setWindowModality(Qt::ApplicationModal);
        progress.setMinimumDuration(200);
        progress.setValue(0);

        cb(out, progress);
        file.close();
    }

    void ScriptThreadsWidget::UpdateDisassemblyInfo(int row, bool includeDesc)
    {
        auto& code = m_Layout->GetCode();
        uint32_t pc = m_Layout->GetInstruction(row).Pc;
        int index = m_Layout->GetFunctionIndexForPc(pc);

        auto func = m_Layout->GetFunction(index);

        QString name = QString::fromStdString(func.Name);
        uint32_t offset = pc - func.Start;

        QString desc;
        if (includeDesc)
            desc = QString::fromStdString(ScriptDisassembler::GetInstructionDescription(code[pc]));

        QString text = QString("%1+%2").arg(name).arg(offset);
        if (!desc.isEmpty())
            text += QString(" | %1").arg(desc);

        m_DisassemblyInfo->setText(text);
    }

    void ScriptThreadsWidget::CleanupDisassembly()
    {
        m_Layout.reset();

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

    void ScriptThreadsWidget::RefreshDisassembly(const rage::scrProgram& program)
    {
        CleanupDisassembly();

        m_Layout = std::make_unique<ScriptLayout>(program);

        auto disasmModel = new DisassemblyModel(*m_Layout, m_Disassembly);
        m_Disassembly->setModel(disasmModel);
        m_Disassembly->setColumnWidth(0, 100);
        m_Disassembly->setColumnWidth(1, 150);
        m_Disassembly->setColumnWidth(2, 400);

        // Fire once to initialize
        OnUpdateDisassemblyInfoByScroll();
        connect(m_Disassembly->verticalScrollBar(), &QScrollBar::valueChanged, this, &ScriptThreadsWidget::OnUpdateDisassemblyInfoByScroll);
        connect(m_Disassembly->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ScriptThreadsWidget::OnUpdateDisassemblyInfoBySelection);

        auto funcModel = new FunctionListModel(*m_Layout, m_FunctionList);
        m_FunctionFilter = new QSortFilterProxyModel(this);
        m_FunctionFilter->setSourceModel(funcModel);
        m_FunctionFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_FunctionFilter->setFilterKeyColumn(0);
        m_FunctionList->setModel(m_FunctionFilter);
    }

    void ScriptThreadsWidget::UpdateCurrentScript()
    {
        uint32_t hash = GetCurrentScriptHash();

        auto thread = rage::scrThread::GetThread(hash);
        if (!thread)
            return;

        auto state = thread.GetState();

        std::optional<std::pair<uint32_t, uint32_t>> activeBp;
        if (g_BreakpointsSupported)
        {
            activeBp = PipeCommands::GetActiveBreakpoint();
            m_BreakpointsPauseGame->setEnabled(!activeBp.has_value());
        }

        bool isGlobalBreakpointPause = activeBp.has_value() && m_BreakpointsPauseGame->isChecked();
        bool isLocalBreakpoint = activeBp.has_value() && activeBp->first == hash;

        if (isGlobalBreakpointPause || isLocalBreakpoint)
        {
            m_TogglePauseScript->setText("Resume Breakpoint");
            m_TogglePauseScript->setToolTip("Resume the active breakpoint.");
        }
        else if (state == rage::scrThreadState::PAUSED)
        {
            m_TogglePauseScript->setText("Resume Script");
            m_TogglePauseScript->setToolTip("Resume the execution of this script thread.");
        }
        else
        {
            m_TogglePauseScript->setText("Pause Script");
            m_TogglePauseScript->setToolTip("Pause the execution of this script thread.");
        }

        QString stateStr = (state == rage::scrThreadState::RUNNING) ? "RUNNING" : (state == rage::scrThreadState::IDLE) ? "IDLE"
                                                                              : (state == rage::scrThreadState::PAUSED) ? "PAUSED"
                                                                                                                        : "KILLED";

        bool showBreakpointActive = isGlobalBreakpointPause || isLocalBreakpoint;
        m_State->setText("State: " + stateStr + (showBreakpointActive ? " (breakpoint active)" : ""));

        auto priority = thread.GetPriority();
        m_Priority->setText("Priority: " +
                            QString((priority == rage::scrThreadPriority::HIGHEST) ? "HIGHEST" : (priority == rage::scrThreadPriority::NORMAL) ? "NORMAL"
                                                                                             : (priority == rage::scrThreadPriority::LOWEST)   ? "LOWEST"
                                                                                                                                               : "MANUAL_UPDATE"));

        uint32_t id = thread.GetId();

        m_Program->setText(QString("Program: %1").arg(thread.GetProgram()));
        m_ThreadId->setText(QString("Thread ID: %1").arg(id));
        m_ProgramCounter->setText(QString("Program Counter: 0x%1").arg(QString::number(thread.GetProgramCounter(), 16).toUpper()));
        m_FramePointer->setText(QString("Frame Pointer: 0x%1").arg(QString::number(thread.GetFramePointer(), 16).toUpper()));
        m_StackPointer->setText(QString("Stack Pointer: 0x%1").arg(QString::number(thread.GetStackPointer(), 16).toUpper()));
        m_StackSize->setText(QString("Stack Size: %1").arg(thread.GetStackSize()));

        auto program = rage::scrProgram::GetProgram(thread.GetProgram());
        if (!program)
            return;

        m_GlobalVersion->setText(QString("Global Version: %1").arg(program.GetGlobalVersion()));
        m_CodeSize->setText(QString("Code Size: %1").arg(program.GetCodeSize()));
        m_ArgCount->setText(QString("Arg Count: %1").arg(program.GetArgCount()));
        m_StaticCount->setText(QString("Static Count: %1").arg(program.GetStaticCount()));
        m_GlobalCount->setText(QString("Global Count: %1").arg(program.GetGlobalCount()));
        m_GlobalBlock->setText(QString("Global Block: %1").arg(program.GetGlobalBlockIndex()));
        m_NativeCount->setText(QString("Native Count: %1").arg(program.GetNativeCount()));
        m_StringCount->setText(QString("String Count: %1").arg(program.GetStringCount()));

        if (m_LastThreadId != id)
        {
            m_LastThreadId = id;
            RefreshDisassembly(program);
        }
    }

    void ScriptThreadsWidget::OnUpdateScripts()
    {
        QString currentScript = m_ScriptNames->currentText();

        std::vector<std::string> aliveScripts;
        for (const auto& t : rage::scrThread::GetThreads())
        {
            if (t.GetState() == rage::scrThreadState::KILLED || t.GetStackSize() == 0)
                continue;

            std::string name = t.GetName();
            if (!name.empty())
                aliveScripts.push_back(name);
        }

        bool changed = false;
        if (m_ScriptNames->count() != aliveScripts.size())
        {
            changed = true;
        }
        else
        {
            for (int i = 0; i < m_ScriptNames->count(); ++i)
            {
                if (m_ScriptNames->itemText(i).toStdString() != aliveScripts[i])
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
            for (auto& name : aliveScripts)
            {
                uint32_t hash = RAGE_JOAAT(name);
                m_ScriptNames->addItem(QString::fromStdString(name), QVariant::fromValue(hash));
            }

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

        auto thread = rage::scrThread::GetThread(GetCurrentScriptHash());
        if (!thread)
            return;

        std::optional<std::pair<uint32_t, uint32_t>> activeBp;
        if (g_BreakpointsSupported)
            activeBp = PipeCommands::GetActiveBreakpoint();

        bool isGlobalBreakpointPause = activeBp.has_value() && m_BreakpointsPauseGame->isChecked();
        bool isLocalBreakpoint = activeBp.has_value() && activeBp->first == hash;

        if (isGlobalBreakpointPause || isLocalBreakpoint)
            PipeCommands::ResumeBreakpoint();
        else if (thread.GetState() == rage::scrThreadState::PAUSED)
            thread.SetState(rage::scrThreadState::RUNNING);
        else
            thread.SetState(rage::scrThreadState::PAUSED);
    }

    void ScriptThreadsWidget::OnKillScript()
    {
        if (auto thread = rage::scrThread::GetThread(GetCurrentScriptHash()))
        {
            thread.SetState(rage::scrThreadState::KILLED);
            QMessageBox::information(this, "Kill Script", QString("Exit Reason: %1").arg(thread.GetErrorMessage().c_str()));
        }
    }

    void ScriptThreadsWidget::OnExportOptionsDialog()
    {
        QDialog dlg(this);
        dlg.setWindowTitle("Export Options");
        dlg.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        dlg.setModal(true);

        QPushButton* exportDisassembly = new QPushButton("Export Disassembly");
        QPushButton* exportStatics = new QPushButton("Export Statics");
        QPushButton* exportGlobals = new QPushButton("Export Globals");
        QPushButton* exportNatives = new QPushButton("Export Natives");
        QPushButton* exportStrings = new QPushButton("Export Strings");

        exportDisassembly->setToolTip("Export the disassembly of this script program.");
        exportStatics->setToolTip("Export the static variables of this script program.");
        exportGlobals->setToolTip("Export the global variables of this script program.");
        exportNatives->setToolTip("Export the native commands of this script program.");
        exportStrings->setToolTip("Export the string literals of this script program.");

        int maxButtonWidth = std::max({exportDisassembly->sizeHint().width(),
            exportStatics->sizeHint().width(),
            exportGlobals->sizeHint().width(),
            exportNatives->sizeHint().width(),
            exportStrings->sizeHint().width()});

        exportDisassembly->setMinimumWidth(maxButtonWidth);
        exportStatics->setMinimumWidth(maxButtonWidth);
        exportGlobals->setMinimumWidth(maxButtonWidth);
        exportNatives->setMinimumWidth(maxButtonWidth);
        exportStrings->setMinimumWidth(maxButtonWidth);

        QCheckBox* exportAllGlobals = new QCheckBox("Export all");
        QCheckBox* exportAllNatives = new QCheckBox("Export all");
        QCheckBox* onlyTextLabels = new QCheckBox("Only text labels");

        exportAllGlobals->setToolTip("Export all the global blocks.");
        exportAllNatives->setToolTip("Export all the native commands in the game.");
        onlyTextLabels->setToolTip("Export only text labels with their translations.");

        QGridLayout* grid = new QGridLayout(&dlg);

        int row = 0;
        grid->addWidget(exportDisassembly, row++, 0);
        grid->addWidget(exportStatics, row++, 0);
        grid->addWidget(exportGlobals, row, 0);
        grid->addWidget(exportAllGlobals, row++, 1);
        grid->addWidget(exportNatives, row, 0);
        grid->addWidget(exportAllNatives, row++, 1);
        grid->addWidget(exportStrings, row, 0);
        grid->addWidget(onlyTextLabels, row++, 1);

        connect(exportDisassembly, &QPushButton::clicked, this, &ScriptThreadsWidget::OnExportDisassembly);
        connect(exportStatics, &QPushButton::clicked, this, &ScriptThreadsWidget::OnExportStatics);

        connect(exportGlobals, &QPushButton::clicked, this, [this, exportAllGlobals]() {
            OnExportGlobals(exportAllGlobals->isChecked());
        });

        connect(exportNatives, &QPushButton::clicked, this, [this, exportAllNatives]() {
            OnExportNatives(exportAllNatives->isChecked());
        });

        connect(exportStrings, &QPushButton::clicked, this, [this, onlyTextLabels]() {
            OnExportStrings(onlyTextLabels->isChecked());
        });

        dlg.setLayout(grid);
        dlg.exec();
    }

    void ScriptThreadsWidget::OnExportDisassembly()
    {
        if (!GetCurrentScriptHash())
            return;

        auto disassembly = m_Disassembly->model();

        const int count = disassembly->rowCount();
        ExportToFile("Disassembly", "disassembly.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
            for (int row = 0; row < count; ++row)
            {
                if (progress.wasCanceled())
                    return;

                if (disassembly != m_Disassembly->model())
                {
                    QMessageBox::critical(this, "Error", "Disassembly changed during export.");
                    return;
                }

                QString addr = disassembly->data(disassembly->index(row, 0), Qt::DisplayRole).toString();
                QString bytes = disassembly->data(disassembly->index(row, 1), Qt::DisplayRole).toString();
                QString instr = disassembly->data(disassembly->index(row, 2), Qt::DisplayRole).toString();

                out << QString("%1  %2  %3\n").arg(addr, -10).arg(bytes, -25).arg(instr);

                if (row % 50 == 0)
                {
                    progress.setValue(row);
                    QCoreApplication::processEvents();
                }
            }

            progress.setValue(count);
            QMessageBox::information(this, "Export Disassembly", QString("Exported %1 instructions.").arg(count));
        });
    }

    void ScriptThreadsWidget::OnExportStatics()
    {
        const uint32_t hash = GetCurrentScriptHash();

        auto thread = rage::scrThread::GetThread(hash);
        auto program = rage::scrProgram::GetProgram(hash);
        if (!thread || !program)
            return;

        const uint32_t count = program.GetStaticCount();
        if (count == 0)
        {
            QMessageBox::warning(this, "No Statics", "This script has no statics.");
            return;
        }

        ExportToFile("Statics", "statics.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
            for (int i = 0; i < count; i++)
            {
                if (progress.wasCanceled())
                    return;

                int currentVal = static_cast<int>(thread.GetStack(i));
                int defaultVal = static_cast<int>(program.GetStatic(i));
                out << "Static_" << i << " = " << currentVal << " // Default: " << defaultVal << "\n";

                if (i % 50 == 0)
                {
                    progress.setValue(i);
                    QCoreApplication::processEvents();
                }
            }

            progress.setValue(count);
            QMessageBox::information(this, "Export Statics", QString("Exported %1 statics.").arg(count));
        });
    }

    void ScriptThreadsWidget::OnExportGlobals(bool exportAll)
    {
        if (exportAll)
        {
            int lastValidBlock = -1;
            int totalGlobalCount = 0;
            for (int i = 0; i < 64; i++)
            {
                int blockCount = rage::scrProgram::GetGlobalBlockCount(i);
                if (blockCount > 0)
                {
                    lastValidBlock = i;
                    totalGlobalCount += blockCount;
                }
            }

            if (lastValidBlock == -1)
            {
                QMessageBox::warning(this, "No Blocks", "No valid global blocks found.");
                return;
            }

            ExportToFile("Globals", "globals.txt", totalGlobalCount, [&](QTextStream& out, QProgressDialog& progress) {
                for (int block = 0; block <= lastValidBlock; block++)
                {
                    int blockCount = rage::scrProgram::GetGlobalBlockCount(block);
                    if (blockCount == 0)
                        continue;

                    out << "// Block " << block << " (Count " << blockCount << ")\n";

                    for (int i = 0; i < blockCount; i++)
                    {
                        if (progress.wasCanceled())
                            return;

                        int globalIndex = (block << 18) + i;
                        int value = static_cast<int>(rage::scrProgram::GetGlobal(globalIndex));
                        out << "Global_" << globalIndex << " = " << value << "\n";

                        if (globalIndex % 50 == 0)
                        {
                            progress.setValue(globalIndex);
                            QCoreApplication::processEvents();
                        }
                    }
                }

                progress.setValue(totalGlobalCount);
                QMessageBox::information(this, "Export Globals", QString("Exported %1 blocks (%2 globals total).").arg(lastValidBlock).arg(totalGlobalCount));
            });
        }
        else
        {
            auto program = rage::scrProgram::GetProgram(GetCurrentScriptHash());
            if (!program)
                return;

            const uint32_t block = program.GetGlobalBlockIndex();
            const uint32_t count = program.GetGlobalCount();
            if (count == 0)
            {
                QMessageBox::warning(this, "No Globals", "This script has no globals.");
                return;
            }

            ExportToFile("Globals", "globals.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
                for (uint32_t i = 0; i < count; i++)
                {
                    if (progress.wasCanceled())
                        return;

                    int globalIndex = (block << 0x12) + i;
                    int currentVal = static_cast<int>(rage::scrProgram::GetGlobal(globalIndex));
                    int defaultVal = static_cast<int>(program.GetProgramGlobal(i));
                    out << "Global_" << globalIndex << " = " << currentVal << " // Default: " << defaultVal << "\n";

                    if (i % 50 == 0)
                    {
                        progress.setValue(i);
                        QCoreApplication::processEvents();
                    }
                }

                progress.setValue(count);
                QMessageBox::information(this, "Export Globals", QString("Exported %1 globals.").arg(count));
            });
        }
    }

    void ScriptThreadsWidget::OnExportNatives(bool exportAll)
    {
        if (exportAll)
        {
            auto allNatives = gta::Natives::GetAll();

            const uint32_t count = static_cast<uint32_t>(allNatives.size());
            if (count == 0)
            {
                QMessageBox::warning(this, "No Natives", "No natives found.");
                return;
            }

            ExportToFile("Natives", "natives.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
                uint32_t index = 0;
                for (auto& [hash, handler] : allNatives)
                {
                    if (progress.wasCanceled())
                        return;

                    out << "0x" << QString::number(hash, 16).toUpper();
                    out << ":" << QString::fromStdString(Process::GetName()) << "+0x" << QString::number(handler - Process::GetBaseAddress(), 16).toUpper();

                    auto name = std::string(gta::Natives::GetNameByHash(hash));
                    out << " // " << (name.empty() ? "UNKNOWN_NATIVE" : QString::fromStdString(name)) << "\n";

                    if (index % 50 == 0)
                    {
                        progress.setValue(index);
                        QCoreApplication::processEvents();
                    }
                    index++;
                }

                progress.setValue(count);
                QMessageBox::information(this, "Export Natives", QString("Exported %1 natives.").arg(count));
            });
        }
        else
        {
            auto program = rage::scrProgram::GetProgram(GetCurrentScriptHash());
            if (!program)
                return;

            const uint32_t count = program.GetNativeCount();
            if (count == 0)
            {
                QMessageBox::warning(this, "No Natives", "This script has no natives.");
                return;
            }

            ExportToFile("Natives", "natives.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
                for (uint32_t i = 0; i < count; i++)
                {
                    if (progress.wasCanceled())
                        return;

                    uint64_t handler = program.GetNative(i);
                    uint64_t hash = gta::Natives::GetHashByHandler(handler);
                    out << "0x" << QString::number(hash, 16).toUpper();
                    out << ":" << QString::fromStdString(Process::GetName()) << "+0x" << QString::number(handler - Process::GetBaseAddress(), 16).toUpper();

                    auto name = std::string(gta::Natives::GetNameByHash(hash));
                    out << " // " << (name.empty() ? "UNKNOWN_NATIVE" : QString::fromStdString(name)) << "\n";

                    if (i % 50 == 0)
                    {
                        progress.setValue(i);
                        QCoreApplication::processEvents();
                    }
                }

                progress.setValue(count);
                QMessageBox::information(this, "Export Natives", QString("Exported %1 natives.").arg(count));
            });
        }
    }

    void ScriptThreadsWidget::OnExportStrings(bool onlyTextLabels)
    {
        auto program = rage::scrProgram::GetProgram(GetCurrentScriptHash());
        if (!program)
            return;

        const auto strings = program.GetAllStrings();
        const int count = static_cast<int>(strings.size());
        if (count == 0)
        {
            QMessageBox::warning(this, "No Strings", "This script has no strings.");
            return;
        }

        int exportedCount = 0;
        ExportToFile("Strings", "strings.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
            for (int i = 0; i < count; ++i)
            {
                if (progress.wasCanceled())
                    return;

                const std::string& s = strings[i];

                if (onlyTextLabels)
                {
                    const uint32_t hash = RAGE_JOAAT(s);
                    const std::string label = gta::TextLabels::GetTextLabel(hash);
                    if (!label.empty())
                    {
                        out << QString("%1 (0x%2): %3\n").arg(QString::fromStdString(s)).arg(QString::number(hash, 16).toUpper()).arg(QString::fromStdString(label));
                        ++exportedCount;
                    }
                }
                else
                {
                    out << QString::fromStdString(s) << '\n';
                    ++exportedCount;
                }

                if (i % 50 == 0)
                {
                    progress.setValue(i);
                    QCoreApplication::processEvents();
                }
            }

            progress.setValue(count);
            QMessageBox::information(this, "Export Strings", QString("Exported %1 strings.").arg(exportedCount));
        });
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

        bool ok = false;
        QString input = QInputDialog::getText(this, "Binary Search", "Enter byte pattern (? for wildcard):", QLineEdit::Normal, "", &ok);
        if (!ok || input.isEmpty())
            return;

        QStringList parts = input.split(QRegularExpression("[,\\s]+"), Qt::SkipEmptyParts);

        std::vector<std::optional<uint8_t>> pattern;
        for (const QString& part : parts)
        {
            QString token = part.trimmed();
            if (token == "?")
            {
                pattern.push_back(std::nullopt);
            }
            else
            {
                bool ok = false;
                uint8_t byte = static_cast<uint8_t>(token.toUInt(&ok, 16));
                if (!ok || token.toUInt() > 0xFF)
                {
                    QMessageBox::warning(this, "Invalid Pattern", "Invalid hex byte: " + token);
                    return;
                }
                pattern.push_back(byte);
            }
        }

        auto results = ScriptHelpers::ScanPattern(m_Layout->GetCode(), pattern);
        if (results.empty())
        {
            QMessageBox::information(this, "Binary Search", "No matches found.");
            return;
        }

        ScrollToAddress(results.front());

        QString msg = QString("Found %1 matches:\n\n").arg(results.size());
        int limit = std::min<int>(results.size(), 10);

        for (int i = 0; i < limit; ++i)
            msg += QString("0x%1\n").arg(QString::number(results[i], 16).toUpper());
        if (results.size() > 10)
            msg += QString("\n...and %1 more").arg(results.size() - 10);

        QMessageBox::information(this, "Binary Search Results", msg);
    }

    void ScriptThreadsWidget::OnViewStack()
    {
        auto thread = rage::scrThread::GetThread(GetCurrentScriptHash());
        if (!thread)
            return;

        StackDialog dlg(thread, *m_Layout, this);
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

        QMenu menu(this);
        QAction* copyAction = menu.addAction("Copy");
        QAction* nopAction = menu.addAction("NOP Instruction");
        QAction* patchAction = menu.addAction("Custom Patch");
        QAction* patternAction = menu.addAction("Generate Pattern");
        QAction* xrefAction = menu.addAction("View Xrefs");

        connect(copyAction, &QAction::triggered, [this, index]() {
            OnCopyInstruction(index);
        });

        connect(nopAction, &QAction::triggered, [this, index]() {
            OnNopInstruction(index);
        });

        connect(patchAction, &QAction::triggered, [this, index]() {
            OnPatchInstruction(index);
        });

        connect(patternAction, &QAction::triggered, [this, index]() {
            OnGeneratePattern(index);
        });

        connect(xrefAction, &QAction::triggered, [this, index]() {
            OnViewXrefsDialog(index);
        });

        auto& code = m_Layout->GetCode();
        uint32_t pc = m_Layout->GetInstruction(index.row()).Pc;

        if (ScriptHelpers::IsJumpInstruction(code[pc]) || code[pc] == rage::scrOpcode::CALL)
        {
            QAction* jumpAction = menu.addAction("Jump to Address");
            connect(jumpAction, &QAction::triggered, [this, index]() {
                OnJumpToInstructionAddress(index);
            });
        }

        QAction* breakpointAction = nullptr;
        if (g_BreakpointsSupported)
        {
            bool exists = PipeCommands::BreakpointExists(GetCurrentScriptHash(), pc);
            breakpointAction = exists ? menu.addAction("Remove Breakpoint") : menu.addAction("Set Breakpoint");
            connect(breakpointAction, &QAction::triggered, [this, index, exists]() {
                OnSetBreakpoint(index, !exists);
            });
        }
        else
        {
            breakpointAction = menu.addAction("Set Breakpoint");
            breakpointAction->setEnabled(false);
        }

        menu.exec(m_Disassembly->viewport()->mapToGlobal(pos));
    }

    void ScriptThreadsWidget::OnCopyInstruction(const QModelIndex& index)
    {
        auto model = index.model();
        int row = index.row();
        int columnCount = model->columnCount();

        QStringList rowData;
        for (int col = 0; col < columnCount; ++col)
        {
            QModelIndex cellIndex = model->index(row, col);
            rowData << cellIndex.data(Qt::DisplayRole).toString();
        }

        QString text = rowData.join("\t");
        QGuiApplication::clipboard()->setText(text);
    }

    void ScriptThreadsWidget::OnNopInstruction(const QModelIndex& index)
    {
        uint32_t pc = m_Layout->GetInstruction(index.row()).Pc;
        uint32_t size = ScriptHelpers::GetInstructionSize(m_Layout->GetCode(), pc);
        for (uint32_t i = 0; i < size; ++i)
            m_Layout->GetProgram().SetCode(pc + i, rage::scrOpcode::NOP);

        m_Layout->Refresh();
        static_cast<DisassemblyModel*>(m_Disassembly->model())->layoutChanged();
    }

    void ScriptThreadsWidget::OnPatchInstruction(const QModelIndex& index)
    {
        uint32_t pc = m_Layout->GetInstruction(index.row()).Pc;
        uint32_t instrSize = ScriptHelpers::GetInstructionSize(m_Layout->GetCode(), pc);

        bool ok = false;
        QString input = QInputDialog::getText(this, "Custom Patch", QString("Enter up to %1 bytes in hex (space separated):").arg(instrSize), QLineEdit::Normal, "", &ok);
        if (!ok || input.isEmpty())
            return;

        QByteArray newBytes;
        QStringList parts = input.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() > instrSize)
        {
            QMessageBox::warning(this, "Too Long", "Too many bytes entered!");
            return;
        }

        for (const QString& part : parts)
        {
            bool okByte;
            uint8_t byte = static_cast<uint8_t>(part.toUInt(&okByte, 16));
            if (!okByte)
            {
                QMessageBox::warning(this, "Invalid Byte", "Invalid hex byte entered: " + part);
                return;
            }
            newBytes.append(byte);
        }

        bool fillWithNops = false;
        if (newBytes.size() < instrSize)
            fillWithNops = QMessageBox::question(this, "Fill Remaining?", QString("You entered %1 of %2 bytes.\nFill remaining %3 bytes with NOPs?").arg(newBytes.size()).arg(instrSize).arg(instrSize - newBytes.size()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

        for (int i = 0; i < instrSize; i++)
        {
            uint8_t value;
            if (i < newBytes.size())
                value = static_cast<uint8_t>(newBytes[i]);
            else if (fillWithNops)
                value = rage::scrOpcode::NOP;
            else
                continue; // Leave as is

            m_Layout->GetProgram().SetCode(pc + i, value);
        }

        m_Layout->Refresh();
        static_cast<DisassemblyModel*>(m_Disassembly->model())->layoutChanged();
    }

    void ScriptThreadsWidget::OnGeneratePattern(const QModelIndex& index)
    {
        if (!index.isValid())
            return;

        auto& code = m_Layout->GetCode();
        uint32_t pc = m_Layout->GetInstruction(index.row()).Pc;

        std::string uniquePattern;

        int patternLength = 4;
        for (; patternLength <= 32; ++patternLength)
        {
            if (ScriptHelpers::IsPatternUnique(code, pc, patternLength))
            {
                uniquePattern = ScriptHelpers::MakePattern(code, pc, patternLength);
                break;
            }
        }

        if (uniquePattern.empty())
        {
            QMessageBox::warning(this, "Failed", "Failed to generate pattern.");
            return;
        }

        QGuiApplication::clipboard()->setText(QString::fromStdString(uniquePattern));
        QMessageBox::information(this, "Generate Pattern", QString("Pattern copied to clipboard:\n%2").arg(QString::fromStdString(uniquePattern)));
    }

    void ScriptThreadsWidget::OnViewXrefsDialog(const QModelIndex& index)
    {
        auto& code = m_Layout->GetCode();
        uint32_t targetPc = m_Layout->GetInstruction(index.row()).Pc;

        std::vector<std::pair<uint32_t, std::string>> xrefs;

        uint32_t pc = 0;
        while (pc < code.size())
        {
            if (ScriptHelpers::IsXrefToPc(code, pc, targetPc))
            {
                int funcIndex = m_Layout->GetFunctionIndexForPc(targetPc);
                auto insn = ScriptDisassembler::DecodeInstruction(code, pc, rage::scrProgram(), -1, funcIndex);
                xrefs.emplace_back(pc, insn.Instruction);
            }

            pc += ScriptHelpers::GetInstructionSize(code, pc);
        }

        if (xrefs.empty())
        {
            QMessageBox::warning(this, "No Xrefs", "No xrefs found for this address.");
            return;
        }

        XrefsDialog dlg(xrefs, this);
        connect(&dlg, &XrefsDialog::XrefDoubleClicked, this, [this, &dlg](uint32_t addr) {
            ScrollToAddress(addr);
            dlg.close();
        });
        dlg.exec();
    }

    void ScriptThreadsWidget::OnJumpToInstructionAddress(const QModelIndex& index)
    {
        uint32_t targetAddress;

        auto& code = m_Layout->GetCode();
        uint32_t pc = m_Layout->GetInstruction(index.row()).Pc;

        if (code[pc] == rage::scrOpcode::CALL)
            targetAddress = ScriptHelpers::ReadU24(code, pc + 1);
        else
            targetAddress = pc + 2 + ScriptHelpers::ReadS16(code, pc + 1) + 1;

        ScrollToAddress(targetAddress);
    }

    void ScriptThreadsWidget::OnSetBreakpoint(const QModelIndex& index, bool set)
    {
        uint32_t script = GetCurrentScriptHash();
        uint32_t pc = m_Layout->GetInstruction(index.row()).Pc;
        PipeCommands::SetBreakpoint(script, pc, set);
    }
}