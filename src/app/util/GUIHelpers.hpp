#pragma once

class QString;
class QTextStream;
class QProgressDialog;

namespace rage
{
    class scrProgram;
}

namespace scrDbgApp::GUIHelpers
{
    enum class BinarySearchType
    {
        PATTERN,
        HEXADECIMAL,
        DECIMAL,
        FLOAT,
        STRING // handled in ParseBinarySearchString
    };

    extern void ExportToFile(const QString& title, const QString& filename, int count, std::function<void(QTextStream&, QProgressDialog&)> cb);
    extern std::vector<std::optional<uint8_t>> ParseBinarySearch(const QString& input, BinarySearchType type);
    extern std::vector<std::vector<std::optional<uint8_t>>> ParseBinarySearchString(const QString& input, const rage::scrProgram& program);
}