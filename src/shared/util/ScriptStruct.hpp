#pragma once

namespace scrDbg::ScriptStruct
{
    enum class FieldType : uint8_t
    {
        INT = 0,
        BOOL,
        FLOAT,
        STRING,
        TEXT_LABEL_7,
        TEXT_LABEL_15,
        TEXT_LABEL_23,
        TEXT_LABEL_31,
        TEXT_LABEL_63
    };

    static constexpr size_t GetDataSize(FieldType type)
    {
        switch (type)
        {
        case FieldType::TEXT_LABEL_7:
            return 8;
        case FieldType::TEXT_LABEL_15:
            return 16;
        case FieldType::TEXT_LABEL_23:
            return 24;
        case FieldType::TEXT_LABEL_31:
            return 32;
        case FieldType::TEXT_LABEL_63:
            return 64;
        }

        return 0;
    }

    static constexpr size_t GetMaxTextLength(FieldType type)
    {
        size_t dataSize = GetDataSize(type);
        return dataSize > 0 ? dataSize - 1 : 0;
    }

    static constexpr size_t GetSlotCount(FieldType type)
    {
        size_t dataSize = GetDataSize(type);
        if (dataSize > 0)
            return dataSize / 4;

        return 1; // INT, BOOL, FLOAT, STRING
    }

    static const char* GetTypeName(FieldType type)
    {
        switch (type)
        {
        case FieldType::INT:
            return "INT";
        case FieldType::BOOL:
            return "BOOL";
        case FieldType::FLOAT:
            return "FLOAT";
        case FieldType::STRING:
            return "STRING";
        case FieldType::TEXT_LABEL_7:
            return "TEXT_LABEL_7";
        case FieldType::TEXT_LABEL_15:
            return "TEXT_LABEL_15";
        case FieldType::TEXT_LABEL_23:
            return "TEXT_LABEL_23";
        case FieldType::TEXT_LABEL_31:
            return "TEXT_LABEL_31";
        case FieldType::TEXT_LABEL_63:
            return "TEXT_LABEL_63";
        }

        return "NONE";
    }
}