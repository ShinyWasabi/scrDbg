#include "StructBuilderDialog.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

namespace scrDbgApp
{
    constexpr ScriptStruct::FieldType g_AllTypes[] = {
        ScriptStruct::FieldType::INT,
        ScriptStruct::FieldType::BOOL,
        ScriptStruct::FieldType::FLOAT,
        ScriptStruct::FieldType::STRING,
        ScriptStruct::FieldType::TEXT_LABEL_7,
        ScriptStruct::FieldType::TEXT_LABEL_15,
        ScriptStruct::FieldType::TEXT_LABEL_23,
        ScriptStruct::FieldType::TEXT_LABEL_31,
        ScriptStruct::FieldType::TEXT_LABEL_63};

    StructBuilderDialog::StructBuilderDialog(QWidget* parent)
        : QDialog(parent)
    {
        setWindowTitle("Struct Builder");
        resize(500, 400);

        auto rootLayout = new QVBoxLayout(this);

        auto scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);

        auto scrollContent = new QWidget(scrollArea);
        m_FieldsLayout = new QVBoxLayout(scrollContent);
        m_FieldsLayout->addStretch();

        scrollArea->setWidget(scrollContent);
        rootLayout->addWidget(scrollArea);

        auto bottomLayout = new QHBoxLayout();
        auto addButton = new QPushButton("Add Field", this);
        connect(addButton, &QPushButton::clicked, this, &StructBuilderDialog::OnAddFieldClicked);
        bottomLayout->addWidget(addButton);

        m_SummaryLabel = new QLabel(this);
        bottomLayout->addWidget(m_SummaryLabel, 1);

        rootLayout->addLayout(bottomLayout);

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        rootLayout->addWidget(buttonBox);

        UpdateIndicesAndSizes();
    }

    QWidget* StructBuilderDialog::CreateValueWidget(ScriptStruct::FieldType type, const PipeCommands::PipeStructField* initial)
    {
        switch (type)
        {
        case ScriptStruct::FieldType::INT:
        {
            auto spin = new QSpinBox(this);
            spin->setRange(INT_MIN, INT_MAX);
            if (initial)
                spin->setValue(initial->IntValue);
            return spin;
        }
        case ScriptStruct::FieldType::BOOL:
        {
            auto check = new QCheckBox(this);
            if (initial)
                check->setChecked(initial->BoolValue);
            return check;
        }
        case ScriptStruct::FieldType::FLOAT:
        {
            auto spin = new QDoubleSpinBox(this);
            spin->setRange(-FLT_MAX, FLT_MAX);
            spin->setDecimals(6);
            if (initial)
                spin->setValue(initial->FloatValue);
            return spin;
        }
        case ScriptStruct::FieldType::STRING:
        {
            auto edit = new QLineEdit(this);
            if (initial)
                edit->setText(QString::fromStdString(initial->StringValue));
            return edit;
        }
        case ScriptStruct::FieldType::TEXT_LABEL_7:
        case ScriptStruct::FieldType::TEXT_LABEL_15:
        case ScriptStruct::FieldType::TEXT_LABEL_23:
        case ScriptStruct::FieldType::TEXT_LABEL_31:
        case ScriptStruct::FieldType::TEXT_LABEL_63:
        {
            auto edit = new QLineEdit(this);
            edit->setMaxLength(static_cast<int>(ScriptStruct::GetMaxTextLength(type)));
            if (initial)
                edit->setText(QString::fromStdString(initial->StringValue));
            return edit;
        }
        }

        auto edit = new QLineEdit(this);
        edit->setReadOnly(true);
        return edit;
    }

    void StructBuilderDialog::AddFieldRow(const PipeCommands::PipeStructField* initial)
    {
        FieldRow row;
        row.Type = initial ? initial->Type : ScriptStruct::FieldType::INT;

        row.RowWidget = new QWidget(this);
        auto rowLayout = new QHBoxLayout(row.RowWidget);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        row.IndexLabel = new QLabel(row.RowWidget);
        row.IndexLabel->setMinimumWidth(30);
        rowLayout->addWidget(row.IndexLabel);

        row.TypeCombo = new QComboBox(row.RowWidget);
        int selectIndex = 0;
        for (int i = 0; i < static_cast<int>(std::size(g_AllTypes)); i++)
        {
            auto type = g_AllTypes[i];
            row.TypeCombo->addItem(QString::fromLatin1(ScriptStruct::GetTypeName(type)), static_cast<int>(type));
            if (type == row.Type)
                selectIndex = i;
        }
        row.TypeCombo->setCurrentIndex(selectIndex);
        rowLayout->addWidget(row.TypeCombo);

        row.ValueWidget = CreateValueWidget(row.Type, initial);
        rowLayout->addWidget(row.ValueWidget, 1);

        row.RemoveButton = new QPushButton("Remove", row.RowWidget);
        rowLayout->addWidget(row.RemoveButton);

        // insert before the trailing stretch
        m_FieldsLayout->insertWidget(m_FieldsLayout->count() - 1, row.RowWidget);
        m_Rows.push_back(row);

        QWidget* rowWidget = row.RowWidget;
        connect(row.RemoveButton, &QPushButton::clicked, this, [this, rowWidget]() {
            RemoveFieldRow(rowWidget);
        });

        connect(row.TypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, rowWidget](int comboIndex) {
            for (auto& r : m_Rows)
            {
                if (r.RowWidget != rowWidget)
                    continue;

                auto newType = static_cast<ScriptStruct::FieldType>(r.TypeCombo->itemData(comboIndex).toInt());
                RebuildValueWidgetForRow(r, newType);
                break;
            }
        });

        UpdateIndicesAndSizes();
    }

    void StructBuilderDialog::RebuildValueWidgetForRow(FieldRow& row, ScriptStruct::FieldType newType)
    {
        auto rowLayout = qobject_cast<QHBoxLayout*>(row.RowWidget->layout());
        int widgetIndex = rowLayout->indexOf(row.ValueWidget);

        rowLayout->removeWidget(row.ValueWidget);
        row.ValueWidget->deleteLater();

        row.Type = newType;
        row.ValueWidget = CreateValueWidget(newType, nullptr);
        rowLayout->insertWidget(widgetIndex, row.ValueWidget, 1);

        UpdateIndicesAndSizes();
    }

    void StructBuilderDialog::RemoveFieldRow(QWidget* rowWidget)
    {
        auto it = std::find_if(m_Rows.begin(), m_Rows.end(), [rowWidget](const FieldRow& r) {
            return r.RowWidget == rowWidget;
        });

        if (it == m_Rows.end())
            return;

        m_FieldsLayout->removeWidget(rowWidget);
        rowWidget->deleteLater();
        m_Rows.erase(it);

        UpdateIndicesAndSizes();
    }

    void StructBuilderDialog::UpdateIndicesAndSizes()
    {
        size_t totalSlots = 0;
        for (size_t i = 0; i < m_Rows.size(); i++)
        {
            m_Rows[i].IndexLabel->setText(QString("[%1]").arg(i));
            totalSlots += ScriptStruct::GetSlotCount(m_Rows[i].Type);
        }

        size_t totalBytes = totalSlots * (g_Game->Is64Bit() ? 8 : 4);
        m_SummaryLabel->setText(QString("Fields: %1  |  Slots: %2  |  Size: %3 bytes").arg(m_Rows.size()).arg(totalSlots).arg(totalBytes));
    }

    PipeCommands::PipeStructField StructBuilderDialog::ExtractFieldValue(const FieldRow& row) const
    {
        PipeCommands::PipeStructField field;
        field.Type = row.Type;

        switch (row.Type)
        {
        case ScriptStruct::FieldType::INT:
            field.IntValue = static_cast<QSpinBox*>(row.ValueWidget)->value();
            break;
        case ScriptStruct::FieldType::BOOL:
            field.BoolValue = static_cast<QCheckBox*>(row.ValueWidget)->isChecked();
            break;
        case ScriptStruct::FieldType::FLOAT:
            field.FloatValue = static_cast<QDoubleSpinBox*>(row.ValueWidget)->value();
            break;
        case ScriptStruct::FieldType::STRING:
        case ScriptStruct::FieldType::TEXT_LABEL_7:
        case ScriptStruct::FieldType::TEXT_LABEL_15:
        case ScriptStruct::FieldType::TEXT_LABEL_23:
        case ScriptStruct::FieldType::TEXT_LABEL_31:
        case ScriptStruct::FieldType::TEXT_LABEL_63:
            field.StringValue = static_cast<QLineEdit*>(row.ValueWidget)->text().toStdString();
            break;
        }

        return field;
    }

    void StructBuilderDialog::OnAddFieldClicked()
    {
        AddFieldRow(nullptr);
    }

    bool StructBuilderDialog::Edit(QWidget* parent, std::vector<PipeCommands::PipeStructField>& fields)
    {
        StructBuilderDialog dialog(parent);
        dialog.SetFields(fields);

        if (dialog.exec() != QDialog::Accepted)
            return false;

        fields = dialog.GetFields();
        return true;
    }

    void StructBuilderDialog::SetFields(const std::vector<PipeCommands::PipeStructField>& fields)
    {
        for (auto& row : m_Rows)
            row.RowWidget->deleteLater();
        m_Rows.clear();

        for (const auto& field : fields)
            AddFieldRow(&field);

        UpdateIndicesAndSizes();
    }

    std::vector<PipeCommands::PipeStructField> StructBuilderDialog::GetFields() const
    {
        std::vector<PipeCommands::PipeStructField> fields;
        fields.reserve(m_Rows.size());

        for (const auto& row : m_Rows)
            fields.push_back(ExtractFieldValue(row));

        return fields;
    }
}