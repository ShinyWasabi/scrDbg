#include "ScriptStatics.hpp"
#include "game/rage/scrThread.hpp"
#include "game/rage/scrProgram.hpp"
#include "game/rage/Joaat.hpp"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include <QMessageBox>

namespace scrDbg
{
    static bool IsStaticAddressValid(uint32_t script, int address)
    {
        if (address < 0)
            return false;

        auto program = rage::scrProgram::GetProgram(script);
        if (!program)
            return false; // or not?

        int count = program.GetStaticCount();
        if (address >= count)
            return false;

        return true;
    }

	ScriptStaticsWidget::ScriptStaticsWidget(QWidget* parent) :
		QWidget(parent)
	{
		m_StaticScriptName = new QLineEdit();
		m_StaticScriptName->setPlaceholderText("Script Name");

        m_StaticIndex = new QLineEdit();
        m_StaticIndex->setValidator(new QIntValidator(0, INT_MAX, this));
        m_StaticIndex->setPlaceholderText("Base Address");

        QGridLayout* offsetsLayout = new QGridLayout();
        for (int i = 0; i < 5; i++)
        {
            QLineEdit* offsetInput = new QLineEdit();
            offsetInput->setValidator(new QIntValidator(0, INT_MAX, this));
            offsetInput->setPlaceholderText(QString("Offset %1").arg(i + 1));
            m_StaticOffsets.push_back(offsetInput);

            QLineEdit* sizeInput = new QLineEdit();
            sizeInput->setValidator(new QIntValidator(1, INT_MAX, this));
            sizeInput->setPlaceholderText("Size");
            sizeInput->setMaximumWidth(60);
            m_StaticSizes.push_back(sizeInput);

            offsetsLayout->addWidget(offsetInput, i, 0);
            offsetsLayout->addWidget(sizeInput, i, 1);
        }

        m_StaticNewValue = new QLineEdit();
        m_StaticNewValue->setPlaceholderText("New Value");
        m_StaticNewValue->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));

        m_StaticCurrentValue = new QLineEdit();
        m_StaticCurrentValue->setPlaceholderText("Current Value");
        m_StaticCurrentValue->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));
        m_StaticCurrentValue->setEnabled(false);

        m_WriteStatic = new QPushButton("Write");
        connect(m_WriteStatic, &QPushButton::clicked, this, &ScriptStaticsWidget::OnWriteNewStaticValue);

        m_UpdateTimer = new QTimer(this);
        m_UpdateTimer->setInterval(100);
        connect(m_UpdateTimer, &QTimer::timeout, this, &ScriptStaticsWidget::OnUpdateCurrentStaticValue);
        m_UpdateTimer->start();

        QVBoxLayout* scriptStaticsLayout = new QVBoxLayout(this);
        scriptStaticsLayout->addWidget(m_StaticScriptName);
        scriptStaticsLayout->addWidget(m_StaticIndex);
        scriptStaticsLayout->addLayout(offsetsLayout);
        scriptStaticsLayout->addWidget(m_StaticNewValue);
        scriptStaticsLayout->addWidget(m_StaticCurrentValue);
        scriptStaticsLayout->addWidget(m_WriteStatic);
        scriptStaticsLayout->addStretch();
        setLayout(scriptStaticsLayout);
	}

    int ScriptStaticsWidget::ComputeStaticAddress()
    {
        int base = -1;
        if (!m_StaticIndex->text().isEmpty())
        {
            bool ok;
            base = m_StaticIndex->text().toInt(&ok);
            if (!ok)
                return INT_MAX;

            for (int i = 0; i < 5; ++i)
            {
                if (!m_StaticOffsets[i]->text().isEmpty())
                {
                    int offset = m_StaticOffsets[i]->text().toInt();
                    if (!m_StaticSizes[i]->text().isEmpty())
                    {
                        int size = m_StaticSizes[i]->text().toInt();
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

    void ScriptStaticsWidget::OnWriteNewStaticValue()
    {
        std::string name = m_StaticScriptName->text().toStdString();
        auto hash = RAGE_JOAAT(name);

        auto thread = rage::scrThread::GetThread(hash);
        if (!thread)
            return;

        int address = ComputeStaticAddress();
        if (!IsStaticAddressValid(hash, address))
        {
            QMessageBox::warning(this, "Invalid Address", "Invalid static address!");
            return;
        }

        auto value = m_StaticNewValue->text().toInt();
        thread.SetStack(address, value);
    }

    void ScriptStaticsWidget::OnUpdateCurrentStaticValue()
    {
        std::string name = m_StaticScriptName->text().toStdString();
        auto hash = RAGE_JOAAT(name);

        auto thread = rage::scrThread::GetThread(hash);
        if (!thread)
        {
            m_StaticCurrentValue->setText("Invalid");
            return;
        }

        int address = ComputeStaticAddress();
        if (!IsStaticAddressValid(hash, address))
        {
            m_StaticCurrentValue->setText("Invalid");
            return;
        }

        int value = static_cast<int>(thread.GetStack(address));
        m_StaticCurrentValue->setText(QString("%1").arg(value));
    }
}