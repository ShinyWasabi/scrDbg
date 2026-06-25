#include "BitfieldWidget.hpp"
#include <QFont>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

namespace scrDbgApp
{
    BitfieldWidget::BitfieldWidget(QWidget* parent)
        : QWidget(parent)
    {
        QVBoxLayout* root = new QVBoxLayout(this);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(4);

        m_HexEdit = new QLineEdit(this);
        m_HexEdit->setMaxLength(8);
        m_HexEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{0,8}"), this));
        m_HexEdit->setPlaceholderText("00000000");
        m_HexEdit->setFont(QFont("Courier", m_HexEdit->font().pointSize()));
        connect(m_HexEdit, &QLineEdit::textEdited, this, &BitfieldWidget::OnHexEdited);
        root->addWidget(m_HexEdit);

        QGridLayout* grid = new QGridLayout();
        grid->setSpacing(2);
        grid->setContentsMargins(0, 0, 0, 0);

        for (int group = 0; group < 8; group++)
        {
            int baseBit = 31 - group * 4; // 31, 27, 23, 19, 15, 11, 7, 3

            for (int offset = 0; offset < 4; offset++)
            {
                int bit = baseBit - offset;
                int col = group * 4 + offset;

                QPushButton* button = new QPushButton("0", this);
                button->setFixedWidth(25);
                button->setFocusPolicy(Qt::NoFocus);
                button->setProperty("bit", bit);
                connect(button, &QPushButton::clicked, this, [this, bit] {
                    OnBitClicked(bit);
                });

                m_Bits[31 - bit] = button;
                grid->addWidget(button, 0, col);

                QLabel* label = new QLabel(QString::number(bit), this);
                label->setAlignment(Qt::AlignHCenter);
                label->setFixedWidth(25);
                grid->addWidget(label, 1, col);
            }
        }

        root->addLayout(grid);
        SetValue(0);
    }

    void BitfieldWidget::SetValue(int value)
    {
        if (m_Value == value)
            return;

        m_Value = value;
        UpdateBitButtons();

        const QString hex = QString("%1").arg(static_cast<uint>(value), 8, 16, QChar('0')).toUpper();
        if (m_HexEdit->text().toUpper() != hex)
        {
            const bool blocked = m_HexEdit->blockSignals(true);
            m_HexEdit->setText(hex);
            m_HexEdit->blockSignals(blocked);
        }

        emit ValueChanged(m_Value);
    }

    void BitfieldWidget::SetInteractive(bool interactive)
    {
        m_HexEdit->setReadOnly(!interactive);
        for (auto button : m_Bits)
        {
            if (button)
                button->setEnabled(interactive);
        }
    }

    void BitfieldWidget::OnHexEdited(const QString& text)
    {
        bool ok = false;
        const uint parsed = text.toUInt(&ok, 16);
        if (ok)
        {
            m_Value = static_cast<int>(parsed);
            UpdateBitButtons();
            emit ValueChanged(m_Value);
        }
    }

    void BitfieldWidget::OnBitClicked(int bit)
    {
        m_Value ^= (1 << bit);
        UpdateBitButtons();

        const QString hex = QString("%1").arg(static_cast<uint>(m_Value), 8, 16, QChar('0')).toUpper();
        const bool blocked = m_HexEdit->blockSignals(true);
        m_HexEdit->setText(hex);
        m_HexEdit->blockSignals(blocked);

        emit ValueChanged(m_Value);
    }

    void BitfieldWidget::UpdateBitButtons()
    {
        for (int bit = 0; bit < 32; bit++)
        {
            QPushButton* button = m_Bits[31 - bit];
            if (!button)
                continue;

            const bool set = (m_Value >> bit) & 1;
            button->setText(set ? "1" : "0");

            if (set)
            {
                button->setStyleSheet("QPushButton { background: transparent; color: #2986CC; border: 1px solid #555; }"
                                      "QPushButton:hover { background: rgba(41,134,204,30); }");
            }
            else
            {
                button->setStyleSheet("QPushButton { background: transparent; border: 1px solid #555; }"
                                      "QPushButton:hover { background: rgba(255,255,255,15); }");
            }
        }
    }
}