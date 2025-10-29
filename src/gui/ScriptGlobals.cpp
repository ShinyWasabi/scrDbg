#include "ScriptGlobals.hpp"
#include "game/rage/scrProgram.hpp"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include <QMessageBox>

namespace scrDbg
{
    static bool IsGlobalAddressValid(int address)
    {
        if (address < 0)
            return false;

        int blockIndex = (address >> 0x12) & 0x3F;
        if (blockIndex >= 64)
            return false;

        int blockCount = rage::scrProgram::GetGlobalBlockCount(blockIndex);
        int offset = address & 0x3FFFF;
        if (offset >= blockCount)
            return false;

        return true;
    }

	ScriptGlobalsWidget::ScriptGlobalsWidget(QWidget* parent) :
		QWidget(parent)
	{
        m_GlobalIndex = new QLineEdit();
        m_GlobalIndex->setValidator(new QIntValidator(0, INT_MAX, this));
        m_GlobalIndex->setPlaceholderText("Base Address");

        QGridLayout* offsetsLayout = new QGridLayout();
        for (int i = 0; i < 5; i++)
        {
            QLineEdit* offsetInput = new QLineEdit();
            offsetInput->setValidator(new QIntValidator(0, INT_MAX, this));
            offsetInput->setPlaceholderText(QString("Offset %1").arg(i + 1));
            m_GlobalOffsets.push_back(offsetInput);

            QLineEdit* sizeInput = new QLineEdit();
            sizeInput->setValidator(new QIntValidator(1, INT_MAX, this));
            sizeInput->setPlaceholderText("Size");
            sizeInput->setMaximumWidth(60);
            m_GlobalSizes.push_back(sizeInput);

            offsetsLayout->addWidget(offsetInput, i, 0);
            offsetsLayout->addWidget(sizeInput, i, 1);
        }

        m_GlobalNewValue = new QLineEdit();
        m_GlobalNewValue->setPlaceholderText("New Value");
        m_GlobalNewValue->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));

        m_GlobalCurrentValue = new QLineEdit();
        m_GlobalCurrentValue->setPlaceholderText("Current Value");
        m_GlobalCurrentValue->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));
        m_GlobalCurrentValue->setEnabled(false);

        m_WriteGlobal = new QPushButton("Write");
        connect(m_WriteGlobal, &QPushButton::clicked, this, &ScriptGlobalsWidget::OnWriteNewGlobalValue);

        m_UpdateTimer = new QTimer(this);
        m_UpdateTimer->setInterval(100);
        connect(m_UpdateTimer, &QTimer::timeout, this, &ScriptGlobalsWidget::OnUpdateCurrentGlobalValue);
        m_UpdateTimer->start();

		QVBoxLayout* scriptGlobalsLayout = new QVBoxLayout(this);
        scriptGlobalsLayout->addWidget(m_GlobalIndex);
        scriptGlobalsLayout->addLayout(offsetsLayout);
        scriptGlobalsLayout->addWidget(m_GlobalNewValue);
        scriptGlobalsLayout->addWidget(m_GlobalCurrentValue);
        scriptGlobalsLayout->addWidget(m_WriteGlobal);
		scriptGlobalsLayout->addStretch();
		setLayout(scriptGlobalsLayout);
	}

    int ScriptGlobalsWidget::ComputeGlobalAddress()
    {
        int base = -1;
        if (!m_GlobalIndex->text().isEmpty())
        {
            bool ok;
            base = m_GlobalIndex->text().toInt(&ok);
            if (!ok)
                return INT_MAX;

            for (int i = 0; i < 5; ++i)
            {
                if (!m_GlobalOffsets[i]->text().isEmpty())
                {
                    int offset = m_GlobalOffsets[i]->text().toInt();
                    if (!m_GlobalSizes[i]->text().isEmpty())
                    {
                        int size = m_GlobalSizes[i]->text().toInt();
                        base += (1 + (offset * size));
                    }
                    else
                    {
                        base += offset;
                    }
                }
            }
        }

        return base;
    }

    void ScriptGlobalsWidget::OnWriteNewGlobalValue()
    {
        int address = ComputeGlobalAddress();
        if (!IsGlobalAddressValid(address))
        {
            QMessageBox::warning(this, "Invalid Address", "Invalid global address!");
            return;
        }

        int value = m_GlobalNewValue->text().toInt();
        rage::scrProgram::SetGlobal(address, value);
    }

    void ScriptGlobalsWidget::OnUpdateCurrentGlobalValue()
    {
        int address = ComputeGlobalAddress();
        if (!IsGlobalAddressValid(address))
        {
            m_GlobalCurrentValue->setText("Invalid");
            return;
        }

        int value = static_cast<int>(rage::scrProgram::GetGlobal(address));
        m_GlobalCurrentValue->setText(QString::number(value));
    }
}