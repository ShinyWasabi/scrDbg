#pragma once
#include <QDialog>

class QLineEdit;
class QRadioButton;

namespace scrDbgApp
{
    class Disassembler;

    class BinarySearchDialog : public QDialog
    {
        Q_OBJECT

    public:
        using ScrollFn = std::function<void(uint32_t)>;

        explicit BinarySearchDialog(QWidget* parent = nullptr);

        void Execute(Disassembler* disassembler, ScrollFn scrollFn);

    private:
        enum class SearchType
        {
            PATTERN,
            HEXADECIMAL,
            DECIMAL,
            FLOAT,
            STRING,
        };

        static std::vector<std::optional<uint8_t>> ParseBinarySearch(SearchType type, const QString& input);
        static Disassembler::BinarySearchPattern ParseBinarySearchString(const QString& input, const Disassembler* disassembler);

        QString SearchText() const;
        SearchType GetSelectedType() const;

        QLineEdit* m_Input;
        QRadioButton* m_PatternBtn;
        QRadioButton* m_HexadecimalBtn;
        QRadioButton* m_DecimalBtn;
        QRadioButton* m_FloatBtn;
        QRadioButton* m_StringBtn;
    };
}