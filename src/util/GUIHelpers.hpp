#pragma once

class QString;
class QTextStream;
class QProgressDialog;

namespace scrDbg::GUIHelpers
{
    enum class BinarySearchType
    {
        PATTERN,
        HEXADECIMAL,
        DECIMAL
        // TO-DO: add float and string
    };

    extern void ExportToFile(const QString& title, const QString& filename, int count, std::function<void(QTextStream&, QProgressDialog&)> cb);
    extern std::vector<std::optional<uint8_t>> ParseBinarySearchStr(const QString& input, BinarySearchType type);
}