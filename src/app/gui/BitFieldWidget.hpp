#pragma once
#include <QWidget>
#include <QtCore>

class QLineEdit;
class QPushButton;

namespace scrDbgApp
{
    class BitfieldWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit BitfieldWidget(QWidget* parent = nullptr);

        void SetValue(int value);
        void SetInteractive(bool interactive);

        int Value() const
        {
            return m_Value;
        }

    signals:
        void ValueChanged(int newValue);

    private:
        void UpdateBitButtons();
        void OnHexEdited(const QString& text);
        void OnBitClicked(int bit);

        int m_Value = 0;
        QLineEdit* m_HexEdit = nullptr;
        QPushButton* m_Bits[32] = {};
    };
}