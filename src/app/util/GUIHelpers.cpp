#include "GUIHelpers.hpp"
#include "game/rage/scrOpcode.hpp"
#include "game/rage/scrProgram.hpp"
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QRegularExpression>

namespace scrDbgApp::GUIHelpers
{
    void ExportToFile(const QString& title, const QString& filename, int count, std::function<void(QTextStream&, QProgressDialog&)> cb)
    {
        QString name = QFileDialog::getSaveFileName(nullptr, title, filename, "Text Files (*.txt);;All Files (*)");
        if (name.isEmpty())
            return;

        QFile file(name);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(nullptr, title, "Failed to open file for writing.");
            return;
        }

        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);

        QProgressDialog progress(QString("Exporting %1...").arg(title), "Cancel", 0, count, nullptr);
        progress.setWindowModality(Qt::ApplicationModal);
        progress.setMinimumDuration(200);
        progress.setValue(0);

        cb(out, progress);
        file.close();
    }

    std::vector<std::optional<uint8_t>> ParseBinarySearch(const QString& input, BinarySearchType type)
    {
        std::vector<std::optional<uint8_t>> result;
        QString str = input.trimmed();
        if (str.isEmpty())
            return {};

        switch (type)
        {
        case BinarySearchType::PATTERN:
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
        case BinarySearchType::HEXADECIMAL:
        {
            bool ok = false;
            uint32_t value = 0;

            if (str.startsWith("0x") || str.startsWith("0X"))
                value = str.mid(2).toUInt(&ok, 16);
            else
                value = str.toUInt(&ok, 16);

            if (!ok)
                return {}; // invalid

            // Push all bytes (little-endian)
            for (int i = 0; i < 4; ++i)
                result.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xFF));

            // Remove zeros
            while (result.size() > 1 && result.back() == 0)
                result.pop_back();

            break;
        }
        case BinarySearchType::DECIMAL:
        {
            bool ok = false;
            uint32_t value = str.toUInt(&ok, 10);
            if (!ok)
                return {}; // invalid

            // Push all bytes (little-endian)
            for (int i = 0; i < 4; ++i)
                result.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xFF));

            // Remove zeros
            while (result.size() > 1 && result.back() == 0)
                result.pop_back();

            break;
        }
        case BinarySearchType::FLOAT:
        {
            bool ok = false;
            float value = str.toFloat(&ok);
            if (!ok)
                return {}; // invalid

            // Push all bytes (little-endian)
            uint8_t* p = reinterpret_cast<uint8_t*>(&value);
            for (int i = 0; i < 4; ++i)
                result.push_back(p[i]);

            // Remove zeros
            while (result.size() > 1 && result.back() == 0)
                result.pop_back();

            break;
        }
        }

        return result;
    }

    std::vector<std::vector<std::optional<uint8_t>>> ParseBinarySearchString(const QString& input, const rage::scrProgram& program)
    {
        using OptByte = std::optional<uint8_t>;

        std::vector<std::vector<OptByte>> result;

        if (!program)
            return result;

        std::string value = input.toStdString();
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        auto indexes = program.FindScriptIndex(value);
        if (indexes.empty())
            return result;

        auto buildNormal = [&](uint32_t index) {
            std::vector<OptByte> p;

            auto pushLE = [&](uint32_t value, int bytes) {
                for (int i = 0; i < bytes; i++)
                    p.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xFF));
            };

            if (index < 0x08)
            {
                p.push_back(rage::scrOpcode::PUSH_CONST_0 + index);
            }
            else if (index < 0x100)
            {
                p.push_back(rage::scrOpcode::PUSH_CONST_U8);
                pushLE(index, 1);
            }
            else if (index < 0x8000)
            {
                p.push_back(rage::scrOpcode::PUSH_CONST_S16);
                pushLE(index, 2);
            }
            else if (index < 0x1000000)
            {
                p.push_back(rage::scrOpcode::PUSH_CONST_U24);
                pushLE(index, 3);
            }
            else
            {
                p.push_back(rage::scrOpcode::PUSH_CONST_U32);
                pushLE(index, 4);
            }

            p.push_back(rage::scrOpcode::STRING);
            return p;
        };

        auto buildPeephole = [&](uint32_t index) {
            std::vector<std::vector<OptByte>> out;

            if (index > 0xFF) // only U8 constants
                return out;

            uint8_t idx = static_cast<uint8_t>(index);

            // PUSH_CONST_U8_U8 <wild> <index> STRING
            {
                std::vector<OptByte> p;
                p.push_back(rage::scrOpcode::PUSH_CONST_U8_U8);
                p.push_back(std::nullopt); // first U8 (unknown)
                p.push_back(idx);          // second U8 = string index
                p.push_back(rage::scrOpcode::STRING);
                out.push_back(std::move(p));
            }

            // PUSH_CONST_U8_U8_U8 <wild> <wild> <index> STRING
            {
                std::vector<OptByte> p;
                p.push_back(rage::scrOpcode::PUSH_CONST_U8_U8_U8);
                p.push_back(std::nullopt); // first U8 (unknown)
                p.push_back(std::nullopt); // second U8 (unknown)
                p.push_back(idx);          // third U8 = string index
                p.push_back(rage::scrOpcode::STRING);
                out.push_back(std::move(p));
            }

            return out;
        };

        // Build all pattern variants
        for (uint32_t idx : indexes)
        {
            result.push_back(buildNormal(idx));

            // peephole variants (if any)
            auto p = buildPeephole(idx);
            result.insert(result.end(), p.begin(), p.end());
        }

        return result;
    }
}