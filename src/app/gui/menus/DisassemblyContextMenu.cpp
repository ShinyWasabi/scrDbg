#include "DisassemblyContextMenu.hpp"
#include "gui/dialogs/ResultsDialog.hpp"
#include "gui/models/DisassemblyModel.hpp"
#include "pipe/PipeCommands.hpp"
#include <QClipboard>
#include <QGuiApplication>
#include <QInputDialog>
#include <QMessageBox>

namespace scrDbgApp
{
    DisassemblyContextMenu::DisassemblyContextMenu(const QModelIndex& index, Disassembler* disassembler, uint32_t scriptHash, QWidget* parent)
        : QMenu(parent),
          m_Disassembler(disassembler),
          m_ScriptHash(scriptHash)
    {
        auto& code = disassembler->GetCode();
        uint32_t pc = disassembler->GetInstruction(index.row());
        uint32_t size = disassembler->GetInstructionSize(pc);

        QAction* copyAction = addAction("Copy");
        connect(copyAction, &QAction::triggered, [this, index]() {
            OnCopy(index);
        });

        QAction* nopAction = addAction("NOP Instruction");
        connect(nopAction, &QAction::triggered, [this, index]() {
            OnNopInstruction(index);
        });

        QAction* patchAction = addAction("Custom Patch");
        connect(patchAction, &QAction::triggered, [this, index]() {
            OnCustomPatch(index);
        });

        QAction* patternAction = addAction("Generate Pattern");
        connect(patternAction, &QAction::triggered, [this, index]() {
            OnGeneratePattern(index);
        });

        QAction* xrefAction = addAction("View Xrefs");
        connect(xrefAction, &QAction::triggered, [this, index]() {
            OnViewXrefsDialog(index);
        });

        if (disassembler->IsJumpOrCall(code[pc]))
        {
            QAction* jumpAction = addAction("Jump to Address");
            connect(jumpAction, &QAction::triggered, [this, index]() {
                OnJumpToInstructionAddress(index);
            });
        }

        bool exists = PipeCommands::BreakpointExists(scriptHash, pc);
        QAction* breakpointAction = exists ? addAction("Remove Breakpoint") : addAction("Set Breakpoint");
        connect(breakpointAction, &QAction::triggered, [this, index, exists]() {
            OnSetBreakpoint(index, !exists);
        });
    }

    void DisassemblyContextMenu::OnCopy(const QModelIndex& index)
    {
        auto* model = index.model();
        int row = index.row();
        QStringList parts;
        for (int col = 0; col < model->columnCount(); ++col)
            parts << model->data(model->index(row, col), Qt::DisplayRole).toString();
        QGuiApplication::clipboard()->setText(parts.join('\t'));
    }

    void DisassemblyContextMenu::OnNopInstruction(const QModelIndex& index)
    {
        uint32_t pc = m_Disassembler->GetInstruction(index.row());
        uint32_t size = m_Disassembler->GetInstructionSize(pc);

        std::vector<uint8_t> patch(size, 0);
        m_Disassembler->GetProgram()->SetCode(pc, patch);
        m_Disassembler->Refresh();
        static_cast<DisassemblyModel*>(const_cast<QAbstractItemModel*>(index.model()))->layoutChanged();
    }

    void DisassemblyContextMenu::OnCustomPatch(const QModelIndex& index)
    {
        uint32_t pc = m_Disassembler->GetInstruction(index.row());
        uint32_t size = m_Disassembler->GetInstructionSize(pc);

        bool ok = false;
        QString input = QInputDialog::getText(parentWidget(), "Custom Patch", QString("Enter up to %1 bytes in hex (space separated):").arg(size), QLineEdit::Normal, "", &ok);
        if (!ok || input.isEmpty())
            return;

        QStringList parts = input.split(' ', Qt::SkipEmptyParts);
        if (static_cast<uint32_t>(parts.size()) > size)
        {
            QMessageBox::warning(parentWidget(), "Too Long", "Too many bytes entered!");
            return;
        }

        QByteArray newBytes;
        for (const QString& part : parts)
        {
            bool okByte;
            uint8_t byte = static_cast<uint8_t>(part.toUInt(&okByte, 16));
            if (!okByte)
            {
                QMessageBox::warning(parentWidget(), "Invalid Byte", "Invalid hex byte entered: " + part);
                return;
            }
            newBytes.append(byte);
        }

        bool fillWithNops = false;
        if (newBytes.size() < static_cast<int>(size))
            fillWithNops = QMessageBox::question(parentWidget(), "Fill Remaining?", QString("You entered %1 of %2 bytes.\nFill remaining %3 bytes with NOPs?").arg(newBytes.size()).arg(size).arg(size - newBytes.size()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

        std::vector<uint8_t> patch;
        for (uint32_t i = 0; i < size; i++)
        {
            if (i < static_cast<uint32_t>(newBytes.size()))
                patch.push_back(static_cast<uint8_t>(newBytes[i]));
            else if (fillWithNops)
                patch.push_back(0);
            else
                break;
        }

        m_Disassembler->GetProgram()->SetCode(pc, patch);
        m_Disassembler->Refresh();
        static_cast<DisassemblyModel*>(const_cast<QAbstractItemModel*>(index.model()))->layoutChanged();
    }

    void DisassemblyContextMenu::OnGeneratePattern(const QModelIndex& index)
    {
        if (!index.isValid())
            return;

        uint32_t pc = m_Disassembler->GetInstruction(index.row());

        std::string uniquePattern;
        for (int len = 4; len <= 32; ++len)
        {
            if (m_Disassembler->IsPatternUnique(pc, len))
            {
                uniquePattern = m_Disassembler->MakePattern(pc, len);
                break;
            }
        }

        if (uniquePattern.empty())
        {
            QMessageBox::warning(parentWidget(), "Failed", "Failed to generate pattern.");
            return;
        }

        QGuiApplication::clipboard()->setText(QString::fromStdString(uniquePattern));
        QMessageBox::information(parentWidget(), "Generate Pattern", QString("Pattern copied to clipboard:\n%1").arg(QString::fromStdString(uniquePattern)));
    }

    void DisassemblyContextMenu::OnViewXrefsDialog(const QModelIndex& index)
    {
        uint32_t targetPc = m_Disassembler->GetInstruction(index.row());

        std::vector<ResultsDialog::Entry> results;
        for (int i = 0; i < m_Disassembler->GetInstructionCount(); i++)
        {
            uint32_t pc = m_Disassembler->GetInstruction(i);
            if (!m_Disassembler->IsXrefToPc(pc, targetPc))
                continue;

            auto func = m_Disassembler->GetFunctionForPc(pc);
            auto decoded = m_Disassembler->DecodeInstruction(i);
            results.push_back({pc, func ? func->Name : std::string{}, decoded.Instruction});
        }

        if (results.empty())
        {
            QMessageBox::warning(parentWidget(), "No Xrefs", "No xrefs found for this address.");
            return;
        }

        ResultsDialog dlg("Xrefs", results, parentWidget());
        connect(&dlg, &ResultsDialog::EntryDoubleClicked, this, [this](uint32_t addr) {
            emit JumpToAddressRequested(addr);
        });
        dlg.exec();
    }

    void DisassemblyContextMenu::OnJumpToInstructionAddress(const QModelIndex& index)
    {
        uint32_t pc = m_Disassembler->GetInstruction(index.row());
        emit JumpToAddressRequested(m_Disassembler->GetJumpTarget(pc));
    }

    void DisassemblyContextMenu::OnSetBreakpoint(const QModelIndex& index, bool set)
    {
        uint32_t pc = m_Disassembler->GetInstruction(index.row());
        PipeCommands::SetBreakpoint(m_ScriptHash, pc, set);
    }
}