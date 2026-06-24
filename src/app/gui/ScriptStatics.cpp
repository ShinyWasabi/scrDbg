#include "ScriptStatics.hpp"
#include "BitfieldWidget.hpp"
#include "EditValueDialog.hpp"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
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
    ScriptStaticsWidget::ScriptStaticsWidget(QWidget* parent)
        : QWidget(parent)
    {
        QFormLayout* formLayout = new QFormLayout();
        formLayout->setLabelAlignment(Qt::AlignRight);

        m_ScriptName = new QLineEdit(this);
        formLayout->addRow("Script Name", m_ScriptName);

        m_StaticIndex = new QSpinBox(this);
        m_StaticIndex->setRange(0, INT_MAX);
        m_StaticIndex->setValue(0);
        m_StaticIndex->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Base Address", m_StaticIndex);

        for (int i = 0; i < 5; i++)
        {
            QSpinBox* offsetInput = new QSpinBox(this);
            offsetInput->setRange(0, INT_MAX);
            offsetInput->setValue(0);
            offsetInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
            offsetInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            m_StaticOffsets.push_back(offsetInput);

            QSpinBox* sizeInput = new QSpinBox(this);
            sizeInput->setRange(0, INT_MAX);
            sizeInput->setValue(0);
            sizeInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
            sizeInput->setFixedWidth(60);
            m_StaticSizes.push_back(sizeInput);

            QWidget* row = new QWidget(this);
            QHBoxLayout* hl = new QHBoxLayout(row);
            hl->setContentsMargins(0, 0, 0, 0);
            hl->addWidget(offsetInput, 1);
            hl->addWidget(new QLabel("Size", row));
            hl->addWidget(sizeInput, 0);
            formLayout->addRow(QString("Offset %1").arg(i + 1), row);
        }

        m_ValueType = new QComboBox(this);
        m_ValueType->addItems({"INT", "BITFIELD", "FLOAT", "TEXT_LABEL", "STRING"});
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

        m_CurrentValueString = new QLineEdit(this);
        m_CurrentValueString->setReadOnly(true);
        m_CurrentValueString->setMaxLength(256);

        m_CurrentValueStack->addWidget(m_CurrentValueInt);
        m_CurrentValueStack->addWidget(m_CurrentValueBitfield);
        m_CurrentValueStack->addWidget(m_CurrentValueFloat);
        m_CurrentValueStack->addWidget(m_CurrentValueTextLabel);
        m_CurrentValueStack->addWidget(m_CurrentValueString);
        m_CurrentValueStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_CurrentValueStack->setFixedHeight(m_ValueType->sizeHint().height());

        formLayout->addRow("Value", m_CurrentValueStack);

        m_EditValueButton = new QPushButton("Edit Value", this);
        connect(m_EditValueButton, &QPushButton::clicked, this, &ScriptStaticsWidget::OnEditValue);

        m_ResetButton = new QPushButton("Reset", this);
        connect(m_ResetButton, &QPushButton::clicked, this, &ScriptStaticsWidget::OnReset);

        QHBoxLayout* buttonRow = new QHBoxLayout();
        buttonRow->addWidget(m_EditValueButton);
        buttonRow->addWidget(m_ResetButton);

        m_UpdateTimer = new QTimer(this);
        m_UpdateTimer->setInterval(100);
        connect(m_UpdateTimer, &QTimer::timeout, this, &ScriptStaticsWidget::OnUpdateCurrentStaticValue);
        m_UpdateTimer->start();

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addLayout(formLayout);
        layout->addLayout(buttonRow);
        layout->addStretch();
        setLayout(layout);
    }

    bool ScriptStaticsWidget::IsStaticAddressValid(uint32_t script, int address)
    {
        if (address < 0)
            return false;

        auto program = g_Game->GetProgram(script);
        if (!program)
            return false; // or not?

        int count = program->GetStaticCount();
        if (address >= count)
            return false;

        return true;
    }

    int ScriptStaticsWidget::ComputeStaticAddress()
    {
        int base = m_StaticIndex->value();

        for (int i = 0; i < 5; i++)
        {
            int offset = m_StaticOffsets[i]->value();

            int size = m_StaticSizes[i]->value();
            if (size > 0)
                base += (1 + (offset * size));
            else
                base += offset;
        }

        return base;
    }

    void ScriptStaticsWidget::OnUpdateCurrentStaticValue()
    {
        std::string name = m_ScriptName->text().toStdString();
        uint32_t hash = JOAAT(name);
        auto thread = g_Game->GetThread(hash);

        if (!thread)
        {
            m_CurrentValueInt->setValue(0);
            m_CurrentValueBitfield->SetValue(0);
            m_CurrentValueFloat->setValue(0.0);
            m_CurrentValueTextLabel->setText("");
            m_CurrentValueString->setText("");
            return;
        }

        int address = ComputeStaticAddress();
        if (!IsStaticAddressValid(hash, address))
        {
            m_CurrentValueInt->setValue(0);
            m_CurrentValueBitfield->SetValue(0);
            m_CurrentValueFloat->setValue(0.0);
            m_CurrentValueTextLabel->setText("");
            m_CurrentValueString->setText("");
            return;
        }

        auto _static = thread->GetStack(address);

        switch (m_CurrentValueStack->currentIndex())
        {
        case ScriptValueType::INT:
            m_CurrentValueInt->setValue(_static.Get<int32_t>());
            break;
        case ScriptValueType::BIT_FIELD:
            m_CurrentValueBitfield->SetValue(_static.Get<int32_t>());
            break;
        case ScriptValueType::FLOAT:
            m_CurrentValueFloat->setValue(_static.Get<float>());
            break;
        case ScriptValueType::TEXT_LABEL:
            m_CurrentValueTextLabel->setText(QString::fromStdString(_static.GetString(64)));
            break;
        case ScriptValueType::STRING:
            auto ptr = _static.Deref();
            if (ptr)
                m_CurrentValueString->setText(QString::fromStdString(_static.Deref().GetString(255)));
            else
                m_CurrentValueString->setText("");
            break;
        }
    }

    void ScriptStaticsWidget::OnEditValue()
    {
        std::string name = m_ScriptName->text().toStdString();
        auto hash = JOAAT(name);
        auto thread = g_Game->GetThread(hash);

        if (!thread)
        {
            QMessageBox::warning(this, "Invalid Script", "Script thread not found!");
            return;
        }

        int address = ComputeStaticAddress();
        if (!IsStaticAddressValid(hash, address))
        {
            QMessageBox::warning(this, "Invalid Address", "Invalid static address!");
            return;
        }

        auto _static = thread->GetStack(address);
        auto type = static_cast<ScriptValueType>(m_ValueType->currentIndex());

        EditValueDialog dlg(_static, type, this);
        if (dlg.exec() == QDialog::Accepted)
            dlg.Apply();
    }

    void ScriptStaticsWidget::OnReset()
    {
        m_ScriptName->clear();
        m_StaticIndex->setValue(0);
        for (int i = 0; i < 5; i++)
        {
            m_StaticOffsets[i]->setValue(0);
            m_StaticSizes[i]->setValue(0);
        }
        m_ValueType->setCurrentIndex(0);
    }
}