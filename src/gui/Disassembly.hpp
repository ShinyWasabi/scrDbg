#pragma once
#include "rage/scrProgram.hpp"
#include <QWidget>
#include <QtCore>

namespace scrDbg
{
    class DisassemblyModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit DisassemblyModel(const rage::scrProgram& program, QObject* parent = nullptr);

        int rowCount(const QModelIndex&) const override;
        int columnCount(const QModelIndex&) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        const rage::scrProgram& GetProgram() const;
        uint32_t GetInstructionPC(int row) const;
        std::vector<uint8_t>& GetCode();
        QModelIndex GetJumpIndex(uint32_t address);

    private:
        rage::scrProgram m_Program;
        std::vector<std::pair<uint32_t, int>> m_Instructions;
        std::vector<uint8_t> m_Code;
    };
}