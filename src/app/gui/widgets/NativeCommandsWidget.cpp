#include "NativeCommandsWidget.hpp"
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace scrDbgApp
{
    NativeCommandsWidget::NativeCommandsWidget(QWidget* parent)
        : QWidget(parent)
    {
        auto rootLayout = new QVBoxLayout(this);

        auto searchGroup = new QGroupBox("Search", this);
        auto searchLayout = new QVBoxLayout(searchGroup);

        m_SearchEdit = new QLineEdit(searchGroup);
        m_SearchEdit->setPlaceholderText("Search native by name");
        searchLayout->addWidget(m_SearchEdit);
        connect(m_SearchEdit, &QLineEdit::textChanged, this, &NativeCommandsWidget::OnSearchTextChanged);

        m_ResultsList = new QListWidget(searchGroup);
        searchLayout->addWidget(m_ResultsList);
        connect(m_ResultsList, &QListWidget::itemClicked, this, &NativeCommandsWidget::OnResultActivated);

        rootLayout->addWidget(searchGroup);

        auto callGroup = new QGroupBox("Call", this);
        auto callLayout = new QVBoxLayout(callGroup);

        m_ScriptNameEdit = new QLineEdit(callGroup);
        m_ScriptNameEdit->setPlaceholderText("Script name to call inside");
        callLayout->addWidget(m_ScriptNameEdit);

        m_SelectedNativeLabel = new QLabel("No native selected", callGroup);
        callLayout->addWidget(m_SelectedNativeLabel);

        m_ArgsLayout = new QFormLayout();
        callLayout->addLayout(m_ArgsLayout);

        auto callButtonLayout = new QHBoxLayout();
        m_CallButton = new QPushButton("Call Native", callGroup);
        m_CallButton->setEnabled(false);
        callButtonLayout->addWidget(m_CallButton);
        callButtonLayout->addStretch();
        callLayout->addLayout(callButtonLayout);
        connect(m_CallButton, &QPushButton::clicked, this, &NativeCommandsWidget::OnCallClicked);

        rootLayout->addWidget(callGroup);

        auto returnsGroup = new QGroupBox("Return Values", this);
        m_ReturnsLayout = new QFormLayout(returnsGroup);
        rootLayout->addWidget(returnsGroup);

        auto outValuesGroup = new QGroupBox("Out Values", this);
        m_OutValuesLayout = new QFormLayout(outValuesGroup);
        rootLayout->addWidget(outValuesGroup);

        rootLayout->addStretch();
    }

    QString NativeCommandsWidget::GetTypeName(NativeDB::Types type)
    {
        switch (type)
        {
        case NativeDB::Types::INT:
            return "INT";
        case NativeDB::Types::BOOL:
            return "BOOL";
        case NativeDB::Types::FLOAT:
            return "FLOAT";
        case NativeDB::Types::STRING:
            return "STRING";
        case NativeDB::Types::REFERENCE:
            return "REFERENCE";
        }

        return "NONE";
    }

    void NativeCommandsWidget::SelectNative(uint64_t hash, const QString& name)
    {
        m_SelectedHash = hash;
        m_SelectedNativeLabel->setText(QString("%1 (0x%2)").arg(name).arg(QString("%1").arg(hash, 16, 16, QChar('0')).toUpper()));

        auto argTypes = NativeDB::GetArgsByHash(hash);
        m_ArgTypes = argTypes ? *argTypes : std::vector<NativeDB::Types>{};

        RebuildArgumentInputs();
        ClearReturnOutputs();
        ClearOutValueOutputs();

        m_CallButton->setEnabled(true);
    }

    void NativeCommandsWidget::ClearArgumentInputs()
    {
        while (m_ArgsLayout->rowCount() > 0)
            m_ArgsLayout->removeRow(0);

        m_ArgWidgets.clear();
    }

    void NativeCommandsWidget::ClearReturnOutputs()
    {
        while (m_ReturnsLayout->rowCount() > 0)
            m_ReturnsLayout->removeRow(0);

        m_ReturnWidgets.clear();
    }

    void NativeCommandsWidget::ClearOutValueOutputs()
    {
        while (m_OutValuesLayout->rowCount() > 0)
            m_OutValuesLayout->removeRow(0);

        m_OutValueWidgets.clear();
    }

    QWidget* NativeCommandsWidget::CreateInputForType(NativeDB::Types type)
    {
        switch (type)
        {
        case NativeDB::Types::INT:
        {
            auto spin = new QSpinBox(this);
            spin->setRange(INT_MIN, INT_MAX);
            return spin;
        }
        case NativeDB::Types::BOOL:
        {
            auto check = new QCheckBox(this);
            return check;
        }
        case NativeDB::Types::FLOAT:
        {
            auto spin = new QDoubleSpinBox(this);
            spin->setRange(-FLT_MAX, FLT_MAX);
            spin->setDecimals(6);
            return spin;
        }
        case NativeDB::Types::STRING:
        {
            auto edit = new QLineEdit(this);
            return edit;
        }
        case NativeDB::Types::REFERENCE:
        {
            auto spin = new QSpinBox(this);
            spin->setRange(INT_MIN, INT_MAX);
            return spin;
        }
        }

        auto edit = new QLineEdit(this);
        edit->setEnabled(false);
        return edit;
    }

    void NativeCommandsWidget::RebuildArgumentInputs()
    {
        ClearArgumentInputs();

        for (size_t i = 0; i < m_ArgTypes.size(); i++)
        {
            NativeDB::Types type = m_ArgTypes[i];
            QWidget* input = CreateInputForType(type);

            m_ArgWidgets.push_back(input);
            m_ArgsLayout->addRow(QString("Arg %1 (%2)").arg(i).arg(GetTypeName(type)), input);
        }

        if (m_ArgTypes.empty())
        {
            auto label = new QLabel("This native takes no arguments.", this);
            m_ArgsLayout->addRow(label);
        }
    }

    QWidget* NativeCommandsWidget::CreateOutputForType(const PipeCommands::PipeNativeReturn& value)
    {
        switch (value.Type)
        {
        case NativeDB::Types::INT:
        {
            auto edit = new QLineEdit(QString::number(value.IntValue), this);
            edit->setReadOnly(true);
            return edit;
        }
        case NativeDB::Types::BOOL:
        {
            auto check = new QCheckBox(this);
            check->setChecked(value.BoolValue);
            check->setEnabled(false);
            return check;
        }
        case NativeDB::Types::FLOAT:
        {
            auto edit = new QLineEdit(QString::number(value.FloatValue), this);
            edit->setReadOnly(true);
            return edit;
        }
        case NativeDB::Types::STRING:
        {
            auto edit = new QLineEdit(QString::fromStdString(value.StringValue), this);
            edit->setReadOnly(true);
            return edit;
        }
        }

        auto edit = new QLineEdit(this);
        edit->setReadOnly(true);
        return edit;
    }

    void NativeCommandsWidget::ShowReturnValues(const std::vector<PipeCommands::PipeNativeReturn>& values)
    {
        ClearReturnOutputs();

        if (values.empty())
        {
            auto label = new QLabel("This native returned no values.", this);
            m_ReturnsLayout->addRow(label);
            return;
        }

        for (size_t i = 0; i < values.size(); i++)
        {
            QWidget* output = CreateOutputForType(values[i]);
            m_ReturnWidgets.push_back(output);
            m_ReturnsLayout->addRow(QString("Return %1 (%2)").arg(i).arg(GetTypeName(values[i].Type)), output);
        }
    }

    QWidget* NativeCommandsWidget::CreateOutValue(const PipeCommands::PipeNativeOutValue& value)
    {
        uint64_t val = g_Game->Is64Bit() ? value.RefValue64 : value.RefValue32;
        auto edit = new QLineEdit(QString::number(val), this);
        edit->setReadOnly(true);
        return edit;
    }

    void NativeCommandsWidget::ShowOutValues(const std::vector<PipeCommands::PipeNativeOutValue>& values, const std::vector<size_t>& argIndices)
    {
        ClearOutValueOutputs();

        if (values.empty())
        {
            auto label = new QLabel("This native produced no out values.", this);
            m_OutValuesLayout->addRow(label);
            return;
        }

        for (size_t i = 0; i < values.size(); i++)
        {
            QWidget* output = CreateOutValue(values[i]);
            m_OutValueWidgets.push_back(output);

            QString label = i < argIndices.size() ? QString("Out (Arg %1)").arg(argIndices[i]) : QString("Out %1").arg(i);
            m_OutValuesLayout->addRow(label, output);
        }
    }

    void NativeCommandsWidget::OnSearchTextChanged(const QString& text)
    {
        m_ResultsList->clear();

        if (text.isEmpty())
            return;

        std::string lowerSearch = text.toStdString();
        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

        for (const auto& [hash, name] : NativeDB::GetNames())
        {
            std::string lowerName = name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

            if (lowerName.find(lowerSearch) == std::string::npos)
                continue;

            auto item = new QListWidgetItem(QString::fromStdString(name), m_ResultsList);
            item->setData(Qt::UserRole, hash);

            if (m_ResultsList->count() >= 100) // display max 100 names
                break;
        }
    }

    void NativeCommandsWidget::OnResultActivated(QListWidgetItem* item)
    {
        if (!item)
            return;

        uint64_t hash = item->data(Qt::UserRole).toULongLong();
        SelectNative(hash, item->text());
    }

    void NativeCommandsWidget::OnCallClicked()
    {
        if (m_SelectedHash == 0)
            return;

        QString scriptName = m_ScriptNameEdit->text().trimmed();
        if (scriptName.isEmpty())
        {
            QMessageBox::warning(this, "No Script Name", "No script name provided.");
            return;
        }

        uint32_t scriptHash = JOAAT(scriptName.toStdString());

        auto thread = g_Game->GetThread(scriptHash);
        if (!thread)
        {
            QMessageBox::warning(this, "Script Not Found", "Script not found.");
            return;
        }

        if (PipeCommands::GetActiveBreakpoint().has_value())
        {
            QMessageBox::warning(this, "Breakpoint Active", "A breakpoint is currently active.");
            return;
        }

        if (thread->GetState() == ScriptThread::State::KILLED || thread->GetState() == ScriptThread::State::PAUSED)
        {
            QMessageBox::warning(this, "Invalid State", "Script is in an invalid state.");
            return;
        }

        std::vector<PipeCommands::PipeNativeArg> args;
        args.reserve(m_ArgTypes.size());

        std::vector<size_t> refArgIndices;

        for (size_t i = 0; i < m_ArgTypes.size(); i++)
        {
            NativeDB::Types type = m_ArgTypes[i];
            QWidget* widget = m_ArgWidgets[i];

            PipeCommands::PipeNativeArg arg;
            arg.Type = type;

            switch (type)
            {
            case NativeDB::Types::INT:
                arg.IntValue = static_cast<QSpinBox*>(widget)->value();
                break;
            case NativeDB::Types::BOOL:
                arg.BoolValue = static_cast<QCheckBox*>(widget)->isChecked();
                break;
            case NativeDB::Types::FLOAT:
                arg.FloatValue = static_cast<QDoubleSpinBox*>(widget)->value();
                break;
            case NativeDB::Types::STRING:
                arg.StringValue = static_cast<QLineEdit*>(widget)->text().toStdString();
                break;
            case NativeDB::Types::REFERENCE:
            {
                int value = static_cast<QSpinBox*>(widget)->value();
                arg.RefValue64 = static_cast<int64_t>(value);
                arg.RefValue32 = static_cast<int32_t>(value);
                refArgIndices.push_back(i);
                break;
            }
            }

            args.push_back(std::move(arg));
        }

        auto results = PipeCommands::InvokeNative(scriptHash, m_SelectedHash, args);
        ShowReturnValues(results.Returns);
        ShowOutValues(results.OutValues, refArgIndices);
    }
}