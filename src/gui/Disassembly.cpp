#include "Disassembly.hpp"
#include "Disassembler.hpp"

namespace scrDbg
{
    DisassemblyModel::DisassemblyModel(const rage::scrProgram& program, QObject* parent) :
        QAbstractTableModel(parent),
        m_Program(program)
    {
        m_Code = m_Program.GetFullCode();

        uint32_t pc = 0;
        int strIndex = -1;
        while (pc < m_Code.size())
        {
            strIndex = ScriptDisassembler::GetNextStringIndex(m_Code, pc, strIndex);

            m_Instructions.emplace_back(pc, strIndex);

            pc += ScriptDisassembler::GetInstructionSize(m_Code, pc);
        }
    }

    int DisassemblyModel::rowCount(const QModelIndex& parent) const
    {
        return static_cast<int>(m_Instructions.size());
    }

    int DisassemblyModel::columnCount(const QModelIndex&) const
    {
        return 3;
    }

    QVariant DisassemblyModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
            return QVariant();

        static const char* headers[] = { "Address", "Bytes", "Instruction" };
        return section < 3 ? headers[section] : QVariant();
    }

    QVariant DisassemblyModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || role != Qt::DisplayRole)
            return QVariant();

        const auto& [offset, strIndex] = m_Instructions[index.row()];
        auto insn = ScriptDisassembler::DecodeInstruction(m_Program, m_Code, offset, strIndex);

        switch (index.column())
        {
        case 0: return QString::fromStdString(insn.Address);
        case 1: return QString::fromStdString(insn.Bytes);
        case 2: return QString::fromStdString(insn.Instruction);
        }

        return QVariant();
    }

    const rage::scrProgram& DisassemblyModel::GetProgram() const
    {
        return m_Program;
    }

    uint32_t DisassemblyModel::GetInstructionPC(int row) const
    {
        if (row < 0 || row >= static_cast<int>(m_Instructions.size()))
            return 0;

        return m_Instructions[row].first;
    }

    std::vector<uint8_t>& DisassemblyModel::GetCode()
    {
        return m_Code;
    }
}