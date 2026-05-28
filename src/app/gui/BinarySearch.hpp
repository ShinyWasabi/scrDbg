#pragma once

class QString;

namespace scrDbgApp::BinarySearch
{
    enum class SearchType
    {
        PATTERN,
        HEXADECIMAL,
        DECIMAL,
        FLOAT,
        STRING // handled in ParseBinarySearchString
    };

    std::vector<std::optional<uint8_t>> ParseBinarySearch(const QString& input, SearchType type);
    Disassembler::StringSeachPattern ParseBinarySearchString(const QString& input, const Disassembler* disassembler);
}