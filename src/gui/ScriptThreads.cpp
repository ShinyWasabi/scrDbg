#include "ScriptThreads.hpp"
#include "FunctionList.hpp"
#include "Disassembly.hpp"
#include "Breakpoints.hpp"
#include "Xrefs.hpp"
#include "Disassembler.hpp"
#include "PipeCommands.hpp"
#include "Pointers.hpp"
#include "game/gta/Natives.hpp"
#include "game/gta/TextLabels.hpp"
#include "game/rage/scrThread.hpp"
#include "game/rage/Joaat.hpp"
#include "game/rage/Opcode.hpp"
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QScrollBar>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTimer>
#include <QFileDialog>
#include <QProgressDialog>
#include <QCheckBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QGuiApplication>
#include <QClipboard>

namespace scrDbg
{
    ScriptThreadsWidget::ScriptThreadsWidget(QWidget* parent) :
        QWidget(parent),
        m_LastScriptHash(0),
        m_Layout(nullptr),
        m_FunctionFilter(nullptr)
    {
        m_ScriptNames = new QComboBox(this);
        m_ScriptNames->setEditable(false);

        m_State = new QLabel("State: RUNNING");
        m_Priority = new QLabel("Priority: HIGHEST");
        m_Program = new QLabel("Program: 0");
        m_ThreadId = new QLabel("Thread ID: 0");
        m_ProgramCounter = new QLabel("Program Counter: 0x0000");
        m_FramePointer = new QLabel("Frame Pointer: 0x0000");
        m_StackPointer = new QLabel("Stack Pointer: 0x0000");
        m_StackSize = new QLabel("Stack Size: 0");

        QVector<QLabel*> leftLabels = { m_State, m_Priority, m_Program, m_ThreadId, m_ProgramCounter, m_FramePointer, m_StackPointer, m_StackSize };
        QVBoxLayout* leftLayout = new QVBoxLayout();
        for (auto* lbl : leftLabels)
            leftLayout->addWidget(lbl);

        m_GlobalVersion = new QLabel("Global Version: 0");
        m_CodeSize = new QLabel("Code Size: 0");
        m_ArgCount = new QLabel("Arg Count: 0");
        m_StaticCount = new QLabel("Static Count: 0");
        m_GlobalCount = new QLabel("Global Count: 0");
        m_GlobalBlock = new QLabel("Global Block: 0");
        m_NativeCount = new QLabel("Native Count: 0");
        m_StringCount = new QLabel("String Count: 0");

        QVector<QLabel*> rightLabels = { m_GlobalVersion, m_CodeSize, m_ArgCount, m_StaticCount, m_GlobalCount, m_GlobalBlock, m_NativeCount, m_StringCount };
        QVBoxLayout* rightLayout = new QVBoxLayout();
        for (auto* lbl : rightLabels)
            rightLayout->addWidget(lbl);

        QHBoxLayout* columnsLayout = new QHBoxLayout();
        columnsLayout->addLayout(leftLayout);
        columnsLayout->addSpacing(50);
        columnsLayout->addLayout(rightLayout);

        m_TogglePauseScript = new QPushButton("Pause Script");
        connect(m_TogglePauseScript, &QPushButton::clicked, this, &ScriptThreadsWidget::OnTogglePauseScript);
        m_KillScript = new QPushButton("Kill Script");
        connect(m_KillScript, &QPushButton::clicked, this, &ScriptThreadsWidget::OnKillScript);
        m_ExportOptions = new QPushButton("Export Options");
        connect(m_ExportOptions, &QPushButton::clicked, this, &ScriptThreadsWidget::OnExportOptionsDialog);
        m_JumpToAddress = new QPushButton("Jump to Address");
        connect(m_JumpToAddress, &QPushButton::clicked, this, &ScriptThreadsWidget::OnJumpToAddress);
        m_BinarySearch = new QPushButton("Binary Search");
        connect(m_BinarySearch, &QPushButton::clicked, this, &ScriptThreadsWidget::OnBinarySearch);
        m_ViewBreakpoints = new QPushButton("View Breakpoints");
        m_ViewBreakpoints->setEnabled(g_BreakpointsSupported);
        if (g_BreakpointsSupported)
            connect(m_ViewBreakpoints, &QPushButton::clicked, this, &ScriptThreadsWidget::OnBreakpointsDialog);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(m_TogglePauseScript);
        buttonLayout->addWidget(m_KillScript);
        buttonLayout->addWidget(m_ExportOptions);
        buttonLayout->addWidget(m_JumpToAddress);
        buttonLayout->addWidget(m_BinarySearch);
        buttonLayout->addWidget(m_ViewBreakpoints);
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
            uint32_t pc = m_Layout->GetFunctionStart(idx.row());
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
            uint32_t pc = m_Layout->GetInstructionPc(i);
            uint32_t size = ScriptDisassembler::GetInstructionSize(m_Layout->GetCode(), pc);

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

    void ScriptThreadsWidget::UpdateCurrentScript()
    {
        uint32_t hash = GetCurrentScriptHash();

        auto thread = rage::scrThread::GetThread(hash);
        if (!thread)
            return;

        if (thread.GetState() == rage::scrThreadState::PAUSED)
            m_TogglePauseScript->setText("Resume Script");
        else
            m_TogglePauseScript->setText("Pause Script");

        m_State->setText("State: " +
            QString((thread.GetState() == rage::scrThreadState::RUNNING) ? "RUNNING" :
                (thread.GetState() == rage::scrThreadState::IDLE) ? "IDLE" :
                (thread.GetState() == rage::scrThreadState::PAUSED) ? "PAUSED" : "KILLED"
        ));

        m_Priority->setText("Priority: " +
            QString((thread.GetPriority() == rage::scrThreadPriority::HIGHEST) ? "HIGHEST" :
                (thread.GetPriority() == rage::scrThreadPriority::NORMAL) ? "NORMAL" :
                (thread.GetPriority() == rage::scrThreadPriority::LOWEST) ? "LOWEST" : "MANUAL_UPDATE"
        ));

        m_Program->setText(QString("Program: %1").arg(thread.GetProgram()));
        m_ThreadId->setText(QString("Thread ID: %1").arg(thread.GetId()));
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

        if (hash != m_LastScriptHash)
        {
            m_LastScriptHash = hash;
            OnRefreshDisassembly(program, true);
        }
    }

    void ScriptThreadsWidget::UpdateDisassemblyInfo(int row, bool includeDesc)
    {
        auto& code = m_Layout->GetCode();
        uint32_t pc = m_Layout->GetInstructionPc(row);
        int index = m_Layout->GetFunctionIndexForPc(pc);

        const auto& funcs = m_Layout->GetFunctions();

        QString name = QString::fromStdString(funcs[index].Name);
        uint32_t offset = pc - funcs[index].Start;

        QString desc;
        if (includeDesc)
            desc = QString::fromStdString(ScriptDisassembler::GetInstructionDesc(code[pc]));

        QString text = QString("%1+%2").arg(name).arg(offset);
        if (!desc.isEmpty())
            text += QString(" | %1").arg(desc);

        m_DisassemblyInfo->setText(text);
    }

    void ScriptThreadsWidget::ClearViews()
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
            ClearViews();
            return;
        }

        UpdateCurrentScript();
    }

    void ScriptThreadsWidget::OnRefreshDisassembly(const rage::scrProgram& program, bool resetScroll)
    {
        ClearViews();

        m_Layout = std::make_unique<ScriptLayout>(program);

        auto disasmModel = new DisassemblyModel(*m_Layout, m_Disassembly);
        m_Disassembly->setModel(disasmModel);
        m_Disassembly->setColumnWidth(0, 100);
        m_Disassembly->setColumnWidth(1, 150);
        m_Disassembly->setColumnWidth(2, 400);
        if (resetScroll)
            m_Disassembly->scrollToTop();

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
        if (resetScroll)
            m_FunctionList->scrollToTop();
    }

    void ScriptThreadsWidget::OnTogglePauseScript()
    {
        if (auto thread = rage::scrThread::GetThread(GetCurrentScriptHash()))
        {
            if (thread.GetState() == rage::scrThreadState::PAUSED)
            {
                if (PipeCommands::BreakpointActive())
                    PipeCommands::ResumeBreakpoint();
                else
                    thread.SetState(rage::scrThreadState::RUNNING);
            }
            else
            {
                thread.SetState(rage::scrThreadState::PAUSED);
            }
        }
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

        int maxButtonWidth = std::max({
            exportDisassembly->sizeHint().width(),
            exportStatics->sizeHint().width(),
            exportGlobals->sizeHint().width(),
            exportNatives->sizeHint().width(),
            exportStrings->sizeHint().width()
        });

        exportDisassembly->setMinimumWidth(maxButtonWidth);
        exportStatics->setMinimumWidth(maxButtonWidth);
        exportGlobals->setMinimumWidth(maxButtonWidth);
        exportNatives->setMinimumWidth(maxButtonWidth);
        exportStrings->setMinimumWidth(maxButtonWidth);

        QCheckBox* exportAllGlobals = new QCheckBox("Export all");
        QCheckBox* exportAllNatives = new QCheckBox("Export all");
        QCheckBox* onlyTextLabels = new QCheckBox("Only text labels");

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
            QMessageBox::warning(this, "Export Statics", "This script has no statics.");
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
                QMessageBox::warning(this, "Export Globals", "No valid global blocks found.");
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
                QMessageBox::warning(this, "Export Globals", "This script has no globals.");
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
                QMessageBox::warning(this, "Export Natives", "No natives found.");
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
                QMessageBox::warning(this, "Export Natives", "This script has no natives.");
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
            QMessageBox::warning(this, "Export Strings", "This script has no strings.");
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
        QString input = QInputDialog::getText(this, "Binary Search", "Enter byte pattern:", QLineEdit::Normal, "", &ok);
        if (!ok || input.isEmpty())
            return;

        QStringList parts = input.split(QRegularExpression("[,\\s]+"), Qt::SkipEmptyParts);

        std::vector<std::optional<uint8_t>> pattern;
        for (const QString& part : parts)
        {
            QString token = part.trimmed();
            if (token == "?" || token == "??")
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

        auto& program = m_Layout->GetProgram();
        auto& code = m_Layout->GetCode();

        const uint32_t codeSize = program.GetCodeSize();
        const size_t patSize = pattern.size();

        std::vector<uint32_t> results;
        for (uint32_t i = 0; i + patSize <= codeSize; ++i)
        {
            bool match = true;
            for (uint32_t j = 0; j < patSize; ++j)
            {
                const auto& p = pattern[j];
                if (p && *p != code[i + j])
                {
                    match = false;
                    break;
                }
            }

            if (match)
                results.push_back(i);
        }

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

    void ScriptThreadsWidget::OnBreakpointsDialog()
    {
        auto* dlg = new BreakpointsDialog(this);
        connect(dlg, &BreakpointsDialog::BreakpointDoubleClicked, this, [this, dlg](uint32_t script, uint32_t pc) {
            int idx = m_ScriptNames->findData(script);
            if (idx != -1)
                m_ScriptNames->setCurrentIndex(idx);

            // wait a bit
            QTimer::singleShot(100, this, [this, pc]() {
                ScrollToAddress(pc);
            });
            dlg->close();
        });
        dlg->show();
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
        uint32_t pc = m_Layout->GetInstructionPc(index.row());

        if (ScriptDisassembler::IsJumpInstruction(code[pc]) || code[pc] == rage::scrOpcode::CALL)
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
        uint32_t pc = m_Layout->GetInstructionPc(index.row());
        uint32_t size = ScriptDisassembler::GetInstructionSize(m_Layout->GetCode(), pc);
        for (uint32_t i = 0; i < size; ++i)
            m_Layout->GetProgram().SetCode(pc + i, rage::scrOpcode::NOP);

        OnRefreshDisassembly(m_Layout->GetProgram(), false);
    }

    void ScriptThreadsWidget::OnPatchInstruction(const QModelIndex& index)
    {
        uint32_t pc = m_Layout->GetInstructionPc(index.row());
        uint32_t instrSize = ScriptDisassembler::GetInstructionSize(m_Layout->GetCode(), pc);

        bool ok = false;
        QString input = QInputDialog::getText(this, "Custom Patch", QString("Enter %1 bytes in hex (space or comma separated):").arg(instrSize), QLineEdit::Normal, "", &ok);
        if (!ok || input.isEmpty())
            return;

        QByteArray newBytes;
        QStringList parts = input.split(QRegularExpression("[,\\s]+"), Qt::SkipEmptyParts);
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

        // Fill the rest with nops?
        while (newBytes.size() < instrSize)
            newBytes.append(rage::scrOpcode::NOP);

        for (int i = 0; i < newBytes.size(); ++i)
            m_Layout->GetProgram().SetCode(pc + i, static_cast<uint8_t>(newBytes[i]));

        OnRefreshDisassembly(m_Layout->GetProgram(), false);
    }

    // TO-DO: Refactor this shit
    void ScriptThreadsWidget::OnGeneratePattern(const QModelIndex& index)
    {
        if (!index.isValid())
            return;

        auto& code = m_Layout->GetCode();
        uint32_t pc = m_Layout->GetInstructionPc(index.row());

        if (pc >= code.size())
            return;

        auto GetWildcardSize = [](uint8_t opcode) -> size_t {
            switch (opcode)
            {
            case rage::scrOpcode::CALL:
                return 3;
            case rage::scrOpcode::J:
            case rage::scrOpcode::JZ:
            case rage::scrOpcode::IEQ_JZ:
            case rage::scrOpcode::INE_JZ:
            case rage::scrOpcode::IGT_JZ:
            case rage::scrOpcode::IGE_JZ:
            case rage::scrOpcode::ILT_JZ:
            case rage::scrOpcode::ILE_JZ:
                return 2;
            case rage::scrOpcode::STATIC_U8:
            case rage::scrOpcode::STATIC_U8_LOAD:
            case rage::scrOpcode::STATIC_U8_STORE:
                return 1;
            case rage::scrOpcode::STATIC_U16:
            case rage::scrOpcode::STATIC_U16_LOAD:
            case rage::scrOpcode::STATIC_U16_STORE:
            case rage::scrOpcode::GLOBAL_U16:
            case rage::scrOpcode::GLOBAL_U16_LOAD:
            case rage::scrOpcode::GLOBAL_U16_STORE:
                return 2;
            case rage::scrOpcode::STATIC_U24:
            case rage::scrOpcode::STATIC_U24_LOAD:
            case rage::scrOpcode::STATIC_U24_STORE:
            case rage::scrOpcode::GLOBAL_U24:
            case rage::scrOpcode::GLOBAL_U24_LOAD:
            case rage::scrOpcode::GLOBAL_U24_STORE:
                return 3;
            default:
                return 0;
            }
        };

        auto MakePattern = [&](size_t start, size_t len) -> std::string {
            std::ostringstream ss;
            ss << std::uppercase << std::hex << std::setfill('0');

            size_t i = 0;
            bool first = true;
            while (i < len && (start + i) < code.size())
            {
                if (!first)
                    ss << ' ';
                first = false;

                uint8_t opcode = code[start + i];
                ss << std::setw(2) << static_cast<int>(opcode);
                ++i;

                size_t operandSize = GetWildcardSize(opcode);
                for (size_t j = 0; j < operandSize && i < len && (start + i) < code.size(); ++j, ++i)
                {
                    ss << ' ' << '?';
                }
            }

            return ss.str();
        };

        constexpr size_t maxPatternLength = 32;
        size_t patternLength = 4;
        std::string uniquePattern;

        for (; patternLength <= maxPatternLength; ++patternLength)
        {
            std::string patternStr = MakePattern(pc, patternLength);

            size_t count = 0;
            for (size_t i = 0; i + patternLength <= code.size(); ++i)
            {
                bool match = true;
                size_t j = 0;

                while (j < patternLength)
                {
                    uint8_t a = code[pc + j];
                    uint8_t b = code[i + j];

                    size_t operandSize = GetWildcardSize(a);

                    if (operandSize > 0 && j < patternLength - operandSize)
                    {
                        ++j;
                        j += operandSize;
                        continue;
                    }

                    if (a != b)
                    {
                        match = false;
                        break;
                    }

                    ++j;
                }

                if (match)
                    ++count;

                if (count > 1)
                    break;
            }

            if (count == 1)
            {
                uniquePattern = patternStr;
                break;
            }
        }

        if (uniquePattern.empty())
        {
            QMessageBox::warning(this, "Generate Pattern", "Failed to generate pattern.");
            return;
        }

        QGuiApplication::clipboard()->setText(QString::fromStdString(uniquePattern));
        QMessageBox::information(this, "Generate Pattern", QString("Pattern copied to clipboard:\n%2").arg(QString::fromStdString(uniquePattern)));
    }

    void ScriptThreadsWidget::OnViewXrefsDialog(const QModelIndex& index)
    {
        auto& code = m_Layout->GetCode();
        uint32_t targetPc = m_Layout->GetInstructionPc(index.row());

        std::vector<std::pair<uint32_t, std::string>> xrefs;

        uint32_t pc = 0;
        while (pc < code.size())
        {
            bool isXref = false;
            if (ScriptDisassembler::IsJumpInstruction(code[pc]))
            {
                if ((pc + 2 + ScriptDisassembler::ReadS16(code, pc + 1) + 1) == targetPc)
                    isXref = true;
            }
            else if (code[pc] == rage::scrOpcode::CALL || code[pc] == rage::scrOpcode::PUSH_CONST_U24) // check for function pointers
            {
                if (ScriptDisassembler::ReadU24(code, pc + 1) == targetPc)
                    isXref = true;
            }

            if (isXref)
            {
                int funcIndex = m_Layout->GetFunctionIndexForPc(targetPc);
                auto insn = ScriptDisassembler::DecodeInstruction(code, pc, rage::scrProgram(), -1, funcIndex);
                xrefs.emplace_back(pc, insn.Instruction);
            }

            pc += ScriptDisassembler::GetInstructionSize(code, pc);
        }

        if (xrefs.empty())
        {
            QMessageBox::warning(this, "No Xrefs", "No xrefs found for this address.");
            return;
        }

        auto* dlg = new XrefDialog(xrefs, this);
        connect(dlg, &XrefDialog::XrefDoubleClicked, this, [this, dlg](uint32_t addr) {
            ScrollToAddress(addr);
            dlg->close();
        });
        dlg->show();
    }

    void ScriptThreadsWidget::OnJumpToInstructionAddress(const QModelIndex& index)
    {
        int32_t targetAddress = -1;

        auto& code = m_Layout->GetCode();
        uint32_t pc = m_Layout->GetInstructionPc(index.row());

        if (code[pc] == rage::scrOpcode::CALL)
            targetAddress = ScriptDisassembler::ReadU24(code, pc + 1);
        else
            targetAddress = pc + 2 + ScriptDisassembler::ReadS16(code, pc + 1) + 1;

        if (targetAddress == -1)
            return;

        ScrollToAddress(targetAddress);
    }

    void ScriptThreadsWidget::OnSetBreakpoint(const QModelIndex& index, bool set)
    {
        uint32_t script = GetCurrentScriptHash();
        uint32_t pc = m_Layout->GetInstructionPc(index.row());
        PipeCommands::SetBreakpoint(script, pc, set);
    }
}