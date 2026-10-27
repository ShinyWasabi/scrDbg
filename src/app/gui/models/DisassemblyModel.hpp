#pragma once
#include <QWidget>
#include <QtCore>

namespace scrDbgApp
{
    class DisassemblyModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit DisassemblyModel(const Disassembler* disassembler, QObject* parent = nullptr);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex&) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        void InvalidateBreakpoints();

    private:
        void RefreshBreakpointsIfStale() const;

        mutable int m_CachedRow = -1;
        mutable Disassembler::DecodedInstruction m_CachedInsn;

        mutable std::unordered_set<uint32_t> m_Breakpoints;
        mutable std::optional<uint32_t> m_ActivePc;
        mutable QElapsedTimer m_BreakpointsAge;
        mutable bool m_BreakpointsValid = false;

        const Disassembler* m_Disassembler;
    };
}