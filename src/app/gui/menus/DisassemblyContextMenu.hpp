#pragma once
#include <QMenu>
#include <QModelIndex>

namespace scrDbgApp
{
    class Disassembler;

    class DisassemblyContextMenu : public QMenu
    {
        Q_OBJECT

    public:
        DisassemblyContextMenu(const QModelIndex& index, Disassembler* disassembler, uint32_t scriptHash, QWidget* parent = nullptr);

    signals:
        void JumpToAddressRequested(uint32_t address);

    private:
        void OnCopy(const QModelIndex& index);
        void OnNopInstruction(const QModelIndex& index);
        void OnCustomPatch(const QModelIndex& index);
        void OnGeneratePattern(const QModelIndex& index);
        void OnViewXrefsDialog(const QModelIndex& index);
        void OnJumpToInstructionAddress(const QModelIndex& index);
        void OnSetBreakpoint(const QModelIndex& index, bool set);

        Disassembler* m_Disassembler;
        uint32_t m_ScriptHash;
    };
}