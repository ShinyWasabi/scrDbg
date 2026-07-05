#include "BinarySearchDialog.hpp"
#include "gui/dialogs/ResultsDialog.hpp"
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>
#include <QVBoxLayout>

namespace scrDbgApp
{
    BinarySearchDialog::BinarySearchDialog(QWidget* parent)
        : QDialog(parent)
    {
        setWindowTitle("Binary Search");

        QVBoxLayout* layout = new QVBoxLayout(this);

        m_Input = new QLineEdit();
        layout->addWidget(m_Input);

        m_PatternBtn = new QRadioButton("Pattern");
        m_HexadecimalBtn = new QRadioButton("Hexadecimal");
        m_DecimalBtn = new QRadioButton("Decimal");
        m_FloatBtn = new QRadioButton("Float");
        m_StringBtn = new QRadioButton("String");

        m_PatternBtn->setChecked(true);
        m_PatternBtn->setToolTip("Search for an IDA-style pattern (e.g., 61 ? ? ? 41 16 56).");
        m_HexadecimalBtn->setToolTip("Search for a hexadecimal value (e.g., 0x99B507EA).");
        m_DecimalBtn->setToolTip("Search for a decimal value (e.g., 2578778090).");
        m_FloatBtn->setToolTip("Search for a float value (e.g., 3.14).");
        m_StringBtn->setToolTip("Search for a string inside the script.");

        layout->addWidget(m_PatternBtn);
        layout->addWidget(m_HexadecimalBtn);
        layout->addWidget(m_DecimalBtn);
        layout->addWidget(m_FloatBtn);
        layout->addWidget(m_StringBtn);

        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    QString BinarySearchDialog::SearchText() const
    {
        return m_Input->text();
    }

    BinarySearchDialog::SearchType BinarySearchDialog::GetSelectedType() const
    {
        if (m_HexadecimalBtn->isChecked())
            return SearchType::HEXADECIMAL;

        if (m_DecimalBtn->isChecked())
            return SearchType::DECIMAL;

        if (m_FloatBtn->isChecked())
            return SearchType::FLOAT;

        if (m_StringBtn->isChecked())
            return SearchType::STRING;

        return SearchType::PATTERN;
    }

    void BinarySearchDialog::Execute(Disassembler* disassembler, ScrollFn scrollFn)
    {
        if (exec() != QDialog::Accepted || SearchText().isEmpty())
            return;

        SearchType type = GetSelectedType();
        bool isStringSearch = type == SearchType::STRING;

        Disassembler::BinarySearchPattern patterns;

        if (isStringSearch)
        {
            patterns = ParseBinarySearchString(SearchText(), disassembler);
            if (patterns.empty())
            {
                QMessageBox::warning(this, "Binary Search", "No string matches found.");
                return;
            }
        }
        else
        {
            auto pattern = ParseBinarySearch(type, SearchText());
            if (pattern.empty())
            {
                QMessageBox::warning(this, "Binary Search", "Could not parse the input.");
                return;
            }
            patterns.push_back(std::move(pattern));
        }

        std::vector<uint32_t> allMatches;
        for (auto& pat : patterns)
        {
            auto addrs = disassembler->ScanPattern(pat);
            allMatches.insert(allMatches.end(), addrs.begin(), addrs.end());
        }

        if (allMatches.empty())
        {
            QMessageBox::warning(this, "Binary Search", "No matches found.");
            return;
        }

        bool isGta5OrRdr3 = g_Game->GetType() == GameType::GTA5_GEN8 || g_Game->GetType() == GameType::GTA5_GEN9 || g_Game->GetType() == GameType::RDR3;

        std::vector<ResultsDialog::Entry> results;
        for (int i = 0; i < disassembler->GetInstructionCount(); i++)
        {
            uint32_t pc = disassembler->GetInstruction(i);
            uint32_t size = disassembler->GetInstructionSize(pc);

            for (uint32_t addr : allMatches)
            {
                if (addr < pc || addr >= pc + size)
                    continue;

                // For string searches on GTA5 and RDR2, show the STRING opcode that follows PUSH_CONST
                int decodeIndex = (isGta5OrRdr3 && isStringSearch && i + 1 < disassembler->GetInstructionCount()) ? i + 1 : i;
                auto func = disassembler->GetFunctionForPc(pc);
                auto decoded = disassembler->DecodeInstruction(decodeIndex);

                results.push_back({disassembler->GetInstruction(decodeIndex), func ? func->Name : std::string{}, decoded.Instruction});
            }
        }

        ResultsDialog resDlg("Binary Search", results, this);
        connect(&resDlg, &ResultsDialog::EntryDoubleClicked, this, [&scrollFn](uint32_t addr) {
            scrollFn(addr);
        });
        resDlg.exec();
    }

    std::vector<std::optional<uint8_t>> BinarySearchDialog::ParseBinarySearch(SearchType type, const QString& input)
    {
        std::vector<std::optional<uint8_t>> result;

        QString str = input.trimmed();
        if (str.isEmpty())
            return {};

        switch (type)
        {
        case SearchType::PATTERN:
        {
            QStringList parts = str.split(' ', Qt::SkipEmptyParts);

            // Reject patterns that are only wildcards
            bool allWildcards = true;
            for (const QString& part : parts)
            {
                if (part.trimmed() != "?")
                {
                    allWildcards = false;
                    break;
                }
            }
            if (allWildcards)
                return {};

            for (const QString& part : parts)
            {
                QString token = part.trimmed();
                if (token == "?")
                {
                    result.push_back(std::nullopt);
                }
                else
                {
                    bool ok = false;
                    uint byte = token.toUInt(&ok, 16);
                    if (!ok || byte > 0xFF)
                        return {}; // invalid

                    result.push_back(static_cast<uint8_t>(byte));
                }
            }
            break;
        }
        case SearchType::HEXADECIMAL:
        {
            bool ok = false;
            uint32_t value = 0;

            if (str.startsWith("0x") || str.startsWith("0X"))
                value = str.mid(2).toUInt(&ok, 16);
            else
                value = str.toUInt(&ok, 16);

            if (!ok)
                return {}; // invalid

            for (int i = 0; i < 4; ++i)
                result.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xFF));

            while (result.size() > 1 && result.back() == 0)
                result.pop_back();

            break;
        }
        case SearchType::DECIMAL:
        {
            bool ok = false;
            uint32_t value = str.toUInt(&ok, 10);
            if (!ok)
                return {}; // invalid

            for (int i = 0; i < 4; ++i)
                result.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xFF));

            while (result.size() > 1 && result.back() == 0)
                result.pop_back();

            break;
        }
        case SearchType::FLOAT:
        {
            bool ok = false;
            float value = str.toFloat(&ok);
            if (!ok)
                return {}; // invalid

            uint8_t* p = reinterpret_cast<uint8_t*>(&value);
            for (int i = 0; i < 4; ++i)
                result.push_back(p[i]);

            while (result.size() > 1 && result.back() == 0)
                result.pop_back();

            break;
        }
        }

        return result;
    }

    Disassembler::BinarySearchPattern BinarySearchDialog::ParseBinarySearchString(const QString& input, const Disassembler* disassembler)
    {
        if (!disassembler)
            return {};

        return disassembler->MakeStringSearchPatterns(input.toLower().toStdString());
    }
}