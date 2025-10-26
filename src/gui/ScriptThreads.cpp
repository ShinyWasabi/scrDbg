#include "ScriptThreads.hpp"
#include "Disassembly.hpp"
#include "Breakpoints.hpp"
#include "Disassembler.hpp"
#include "PipeCommands.hpp"
#include "game/gta/TextLabels.hpp"
#include "game/rage/scrThread.hpp"
#include "game/rage/Joaat.hpp"
#include "game/rage/Opcode.hpp"
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
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
        m_LastScriptHash(0)
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
        m_ExportDisassembly = new QPushButton("Export Disassembly");
        connect(m_ExportDisassembly, &QPushButton::clicked, this, &ScriptThreadsWidget::OnExportDisassembly);
        m_ExportStrings = new QPushButton("Export Strings");
        connect(m_ExportStrings, &QPushButton::clicked, this, &ScriptThreadsWidget::OnExportStrings);
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
        buttonLayout->addWidget(m_ExportDisassembly);
        buttonLayout->addWidget(m_ExportStrings);
        buttonLayout->addWidget(m_JumpToAddress);
        buttonLayout->addWidget(m_BinarySearch);
        buttonLayout->addWidget(m_ViewBreakpoints);
        buttonLayout->addStretch();

        m_Disassembly = new QTableView(this);
        m_Disassembly->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        m_Disassembly->verticalHeader()->setVisible(false);
        m_Disassembly->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Disassembly->setSelectionMode(QAbstractItemView::SingleSelection);
        m_Disassembly->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_Disassembly->setAlternatingRowColors(true);
        m_Disassembly->setStyleSheet("QTableView { font-family: Consolas; font-size: 11pt; }");
        m_Disassembly->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_Disassembly, &QWidget::customContextMenuRequested, this, &ScriptThreadsWidget::OnDisassemblyContextMenu);

        m_UpdateTimer = new QTimer(this);
        m_UpdateTimer->setInterval(50);
        connect(m_UpdateTimer, &QTimer::timeout, this, &ScriptThreadsWidget::OnUpdateScripts);
        m_UpdateTimer->start();

        QVBoxLayout* scriptThreadsLayout = new QVBoxLayout(this);
        scriptThreadsLayout->addWidget(m_ScriptNames);
        scriptThreadsLayout->addLayout(columnsLayout);
        scriptThreadsLayout->addLayout(buttonLayout);
        scriptThreadsLayout->addWidget(m_Disassembly, 1);
        scriptThreadsLayout->addStretch();
        setLayout(scriptThreadsLayout);
    }

    uint32_t ScriptThreadsWidget::GetCurrentScriptHash()
    {
        if (m_ScriptNames->currentIndex() < 0)
            return 0;

        return m_ScriptNames->currentData().toUInt();
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

        UpdateCurrentScript();
    }

    void ScriptThreadsWidget::OnRefreshDisassembly(const rage::scrProgram& program, bool resetScroll)
    {
        if (m_Disassembly->model())
            delete m_Disassembly->model();

        auto model = new DisassemblyModel(program, m_Disassembly);
        m_Disassembly->setModel(model);
        m_Disassembly->setColumnWidth(0, 100);
        m_Disassembly->setColumnWidth(1, 150);
        m_Disassembly->setColumnWidth(2, 400);
        m_Disassembly->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        if (resetScroll)
            m_Disassembly->scrollToTop();
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

    void ScriptThreadsWidget::OnExportDisassembly()
    {
        if (!GetCurrentScriptHash())
            return;

        QString fileName = QFileDialog::getSaveFileName(this, "Export Disassembly", "disassembly.txt", "Text Files (*.txt);;All Files (*)");
        if (fileName.isEmpty())
            return;

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, "Error", "Failed to open file for writing.");
            return;
        }

        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);

        auto model = static_cast<DisassemblyModel*>(m_Disassembly->model());

        const int total = model->rowCount({});
        QProgressDialog progress("Exporting disassembly...", "Cancel", 0, total, this);
        progress.setWindowModality(Qt::ApplicationModal);
        progress.setMinimumDuration(200);
        progress.setValue(0);

        for (int row = 0; row < total; ++row)
        {
            if (progress.wasCanceled())
            {
                QMessageBox::information(this, "Canceled", "Export canceled.");
                file.close();
                QFile::remove(fileName);
                return;
            }

            if (model != m_Disassembly->model())
            {
                QMessageBox::warning(this, "Error", "Disassembly changed during export.");
                file.close();
                QFile::remove(fileName);
                return;
            }

            QString addr = model->data(model->index(row, 0), Qt::DisplayRole).toString();
            QString bytes = model->data(model->index(row, 1), Qt::DisplayRole).toString();
            QString instr = model->data(model->index(row, 2), Qt::DisplayRole).toString();

            out << QString("%1  %2  %3\n").arg(addr, -10).arg(bytes, -25).arg(instr);

            if (row % 50 == 0)
            {
                progress.setValue(row);
                QCoreApplication::processEvents();
            }
        }

        progress.setValue(total);
        file.close();

        QMessageBox::information(this, "Export Disassembly", "Disassembly successfully exported.");
    }

    void ScriptThreadsWidget::OnExportStrings()
    {
        auto program = rage::scrProgram::GetProgram(GetCurrentScriptHash());
        if (!program)
            return;

        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Export Options");
        msgBox.setText("Options:");
        QCheckBox* onlyTextLabels = new QCheckBox("Only text labels", &msgBox);
        msgBox.setCheckBox(onlyTextLabels);
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);

        if (msgBox.exec() != QMessageBox::Ok)
            return;

        bool onlyLabels = onlyTextLabels->isChecked();

        auto strings = program.GetAllStrings();
        if (strings.empty())
        {
            QMessageBox::information(this, "Export Strings", "This script has no strings.");
            return;
        }

        QString fileName = QFileDialog::getSaveFileName(this, "Export Script Strings", "strings.txt", "Text Files (*.txt);;All Files (*)");
        if (fileName.isEmpty())
            return;

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(this, "Export Strings", "Failed to open file for writing.");
            return;
        }

        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);

        int exportedCount = 0;
        for (const auto& s : strings)
        {
            if (onlyLabels)
            {
                auto hash = RAGE_JOAAT(s);
                auto label = gta::TextLabels::GetTextLabel(hash);
                if (!label.empty())
                {
                    out << QString("%1 (0x%2): %3\n").arg(QString::fromStdString(s)).arg(QString::number(hash, 16).toUpper()).arg(QString::fromStdString(label));
                    exportedCount++;
                }
            }
            else
            {
                out << QString::fromStdString(s) << "\n";
                exportedCount++;
            }
        }

        file.close();

        QMessageBox::information(this, "Export Strings", QString("Exported %1 strings to:\n%2").arg(exportedCount).arg(fileName));
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

        auto model = static_cast<DisassemblyModel*>(m_Disassembly->model());

        auto idx = model->GetJumpIndex(addr);
        if (idx.isValid())
        {
            m_Disassembly->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            m_Disassembly->setCurrentIndex(idx);
        }
        else
        {
            QMessageBox::warning(this, "Not Found", "No instruction at this address.");
        }
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

        auto model = static_cast<DisassemblyModel*>(m_Disassembly->model());
        auto& program = model->GetProgram();
        auto& code = model->GetCode();

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

        uint32_t firstAddr = results.front();
        auto idx = model->GetJumpIndex(firstAddr);
        if (idx.isValid())
        {
            m_Disassembly->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            m_Disassembly->setCurrentIndex(idx);
        }

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
            int scIdx = m_ScriptNames->findData(script);
            if (scIdx != -1)
                m_ScriptNames->setCurrentIndex(scIdx);

            // wait a bit
            QTimer::singleShot(100, this, [this, pc]() {
                if (auto model = static_cast<DisassemblyModel*>(m_Disassembly->model()))
                {
                    auto idx = model->GetJumpIndex(pc);
                    if (idx.isValid())
                    {
                        m_Disassembly->scrollTo(idx, QAbstractItemView::PositionAtCenter);
                        m_Disassembly->setCurrentIndex(idx);
                    }
                }
            });

            dlg->close();
        });

        dlg->show();
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

        // TO-DO: Move this somewhere else
        static const std::unordered_set<uint8_t> jmpInsns = {rage::scrOpcode::CALL, rage::scrOpcode::J, rage::scrOpcode::JZ, rage::scrOpcode::IEQ_JZ, rage::scrOpcode::INE_JZ, rage::scrOpcode::IGT_JZ, rage::scrOpcode::IGE_JZ, rage::scrOpcode::ILT_JZ, rage::scrOpcode::ILE_JZ};

        auto model = static_cast<DisassemblyModel*>(m_Disassembly->model());
        auto& code = model->GetCode();
        uint32_t pc = model->GetInstructionPC(index.row());

        if (jmpInsns.contains(code[pc]))
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
        if (!index.isValid())
            return;

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
        if (!index.isValid())
            return;

        auto model = static_cast<DisassemblyModel*>(m_Disassembly->model());

        uint32_t pc = model->GetInstructionPC(index.row());
        uint32_t size = ScriptDisassembler::GetInstructionSize(model->GetCode(), pc);
        for (uint32_t i = 0; i < size; ++i)
            model->GetProgram().SetCode(pc + i, rage::scrOpcode::NOP);

        OnRefreshDisassembly(model->GetProgram(), false);
    }

    void ScriptThreadsWidget::OnPatchInstruction(const QModelIndex& index)
    {
        if (!index.isValid())
            return;

        auto model = static_cast<DisassemblyModel*>(m_Disassembly->model());
        uint32_t pc = model->GetInstructionPC(index.row());
        uint32_t instrSize = ScriptDisassembler::GetInstructionSize(model->GetCode(), pc);

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
            model->GetProgram().SetCode(pc + i, static_cast<uint8_t>(newBytes[i]));

        OnRefreshDisassembly(model->GetProgram(), false);
    }

    // TO-DO: Refactor this shit
    void ScriptThreadsWidget::OnGeneratePattern(const QModelIndex& index)
    {
        if (!index.isValid())
            return;

        auto model = static_cast<DisassemblyModel*>(m_Disassembly->model());
        auto& code = model->GetCode();
        uint32_t pc = model->GetInstructionPC(index.row());

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

    void ScriptThreadsWidget::OnJumpToInstructionAddress(const QModelIndex& index)
    {
        int32_t targetAddress = -1;

        auto model = static_cast<DisassemblyModel*>(m_Disassembly->model());

        auto& code = model->GetCode();
        uint32_t pc = model->GetInstructionPC(index.row());

        if (code[pc] == rage::scrOpcode::CALL)
        {
            targetAddress = ScriptDisassembler::ReadU24(code, pc + 1);
        }
        else
        {
            targetAddress = pc + 2 + ScriptDisassembler::ReadS16(code, pc + 1) + 1;
        }

        if (targetAddress == -1)
            return;

        auto idx = model->GetJumpIndex(targetAddress);
        if (idx.isValid())
        {
            m_Disassembly->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            m_Disassembly->setCurrentIndex(idx);
        }
    }

    void ScriptThreadsWidget::OnSetBreakpoint(const QModelIndex& index, bool set)
    {
        if (!index.isValid())
            return;

        auto model = static_cast<DisassemblyModel*>(m_Disassembly->model());

        uint32_t script = GetCurrentScriptHash();
        uint32_t pc = model->GetInstructionPC(index.row());
        PipeCommands::SetBreakpoint(script, pc, set);
    }
}