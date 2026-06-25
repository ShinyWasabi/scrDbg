#include "ScriptGlobalsWidget.hpp"
#include "BitfieldWidget.hpp"
#include "gui/dialogs/EditValueDialog.hpp"
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace scrDbgApp
{
    ScriptGlobalsWidget::ScriptGlobalsWidget(QWidget* parent)
        : QWidget(parent)
    {
        QFormLayout* formLayout = new QFormLayout();
        formLayout->setLabelAlignment(Qt::AlignRight);

        m_GlobalIndex = new QSpinBox(this);
        m_GlobalIndex->setRange(0, INT_MAX);
        m_GlobalIndex->setValue(0);
        m_GlobalIndex->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Base Address", m_GlobalIndex);

        for (int i = 0; i < 5; i++)
        {
            QSpinBox* offsetInput = new QSpinBox(this);
            offsetInput->setRange(0, INT_MAX);
            offsetInput->setValue(0);
            offsetInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
            offsetInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            m_GlobalOffsets.push_back(offsetInput);

            QSpinBox* sizeInput = new QSpinBox(this);
            sizeInput->setRange(0, INT_MAX);
            sizeInput->setValue(0);
            sizeInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
            sizeInput->setFixedWidth(60);
            m_GlobalSizes.push_back(sizeInput);

            QWidget* row = new QWidget(this);
            QHBoxLayout* hl = new QHBoxLayout(row);
            hl->setContentsMargins(0, 0, 0, 0);
            hl->addWidget(offsetInput, 1);
            hl->addWidget(new QLabel("Size", row));
            hl->addWidget(sizeInput, 0);
            formLayout->addRow(QString("Offset %1").arg(i + 1), row);
        }

        m_ValueType = new QComboBox(this);
        m_ValueType->addItems({"INT", "BITFIELD", "FLOAT", "TEXT_LABEL"}); // Globals cannot be declared as STRING
        connect(m_ValueType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            m_CurrentValueStack->setCurrentIndex(index);
            if (index == 1)
                m_CurrentValueStack->setMaximumHeight(QWIDGETSIZE_MAX);
            else
                m_CurrentValueStack->setFixedHeight(m_ValueType->sizeHint().height());
        });
        formLayout->addRow("Type", m_ValueType);

        m_CurrentValueStack = new QStackedWidget(this);

        m_CurrentValueInt = new QSpinBox(this);
        m_CurrentValueInt->setRange(INT_MIN, INT_MAX);
        m_CurrentValueInt->setButtonSymbols(QAbstractSpinBox::NoButtons);
        m_CurrentValueInt->setReadOnly(true);

        m_CurrentValueBitfield = new BitfieldWidget(this);
        m_CurrentValueBitfield->SetInteractive(false);

        m_CurrentValueFloat = new QDoubleSpinBox(this);
        m_CurrentValueFloat->setRange(-FLT_MAX, FLT_MAX);
        m_CurrentValueFloat->setDecimals(6);
        m_CurrentValueFloat->setButtonSymbols(QAbstractSpinBox::NoButtons);
        m_CurrentValueFloat->setReadOnly(true);

        m_CurrentValueTextLabel = new QLineEdit(this);
        m_CurrentValueTextLabel->setReadOnly(true);
        m_CurrentValueTextLabel->setMaxLength(64);

        m_CurrentValueStack->addWidget(m_CurrentValueInt);
        m_CurrentValueStack->addWidget(m_CurrentValueBitfield);
        m_CurrentValueStack->addWidget(m_CurrentValueFloat);
        m_CurrentValueStack->addWidget(m_CurrentValueTextLabel);
        m_CurrentValueStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_CurrentValueStack->setFixedHeight(m_ValueType->sizeHint().height());

        formLayout->addRow("Value", m_CurrentValueStack);

        m_EditValueButton = new QPushButton("Edit Value", this);
        connect(m_EditValueButton, &QPushButton::clicked, this, &ScriptGlobalsWidget::OnEditValue);

        m_ResetButton = new QPushButton("Reset", this);
        connect(m_ResetButton, &QPushButton::clicked, this, &ScriptGlobalsWidget::OnReset);

        QHBoxLayout* buttonRow = new QHBoxLayout();
        buttonRow->addWidget(m_EditValueButton);
        buttonRow->addWidget(m_ResetButton);

        m_UpdateTimer = new QTimer(this);
        m_UpdateTimer->setInterval(100);
        connect(m_UpdateTimer, &QTimer::timeout, this, &ScriptGlobalsWidget::OnUpdateCurrentGlobalValue);
        m_UpdateTimer->start();

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addLayout(formLayout);
        layout->addLayout(buttonRow);
        layout->addStretch();
        setLayout(layout);
    }

    bool ScriptGlobalsWidget::IsGlobalAddressValid(int address)
    {
        if (address < 0)
            return false;

        if (g_Game->Is64Bit())
        {
            int blockIndex = (address >> 0x12) & 0x3F;
            if (blockIndex >= 64)
                return false;

            int blockCount = g_Game->GetGlobalBlockCount(blockIndex);
            int offset = address & 0x3FFFF;
            if (offset >= blockCount)
                return false;

            return true;
        }

        return address < g_Game->GetGlobalCount();
    }

    int ScriptGlobalsWidget::ComputeGlobalAddress()
    {
        int base = m_GlobalIndex->value();

        for (int i = 0; i < 5; i++)
        {
            int offset = m_GlobalOffsets[i]->value();

            int size = m_GlobalSizes[i]->value();
            if (size > 0)
                base += (1 + (offset * size));
            else
                base += offset;
        }

        return base;
    }

    void ScriptGlobalsWidget::OnUpdateCurrentGlobalValue()
    {
        int address = ComputeGlobalAddress();
        if (!IsGlobalAddressValid(address))
        {
            m_CurrentValueInt->setValue(0);
            m_CurrentValueBitfield->SetValue(0);
            m_CurrentValueFloat->setValue(0.0);
            m_CurrentValueTextLabel->setText("");
            return;
        }

        auto global = g_Game->GetGlobal(address);

        switch (m_CurrentValueStack->currentIndex())
        {
        case ScriptValueType::INT:
            m_CurrentValueInt->setValue(global.Get<int32_t>());
            break;
        case ScriptValueType::BIT_FIELD:
            m_CurrentValueBitfield->SetValue(global.Get<int32_t>());
            break;
        case ScriptValueType::FLOAT:
            m_CurrentValueFloat->setValue(global.Get<float>());
            break;
        case ScriptValueType::TEXT_LABEL:
            m_CurrentValueTextLabel->setText(QString::fromStdString(global.GetString(64)));
            break;
        }
    }

    void ScriptGlobalsWidget::OnEditValue()
    {
        int address = ComputeGlobalAddress();
        if (!IsGlobalAddressValid(address))
        {
            QMessageBox::warning(this, "Invalid Address", "Invalid global address!");
            return;
        }

        auto global = g_Game->GetGlobal(address);
        auto type = static_cast<ScriptValueType>(m_ValueType->currentIndex());

        EditValueDialog dlg(global, type, this);
        if (dlg.exec() == QDialog::Accepted)
            dlg.Apply();
    }

    void ScriptGlobalsWidget::OnReset()
    {
        m_GlobalIndex->setValue(0);
        for (int i = 0; i < 5; i++)
        {
            m_GlobalOffsets[i]->setValue(0);
            m_GlobalSizes[i]->setValue(0);
        }
        m_ValueType->setCurrentIndex(0);
    }
}