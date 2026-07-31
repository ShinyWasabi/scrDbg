#pragma once
#include "pipe/PipeCommands.hpp"
#include <QDialog>

class QVBoxLayout;
class QLabel;
class QComboBox;
class QPushButton;

namespace scrDbgApp
{
    class StructBuilderDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit StructBuilderDialog(QWidget* parent = nullptr);

        static bool Edit(QWidget* parent, std::vector<PipeCommands::PipeStructField>& fields); // opens the dialog modally, returns true and writes back into fields if accepted

        void SetFields(const std::vector<PipeCommands::PipeStructField>& fields);
        std::vector<PipeCommands::PipeStructField> GetFields() const;

    private slots:
        void OnAddFieldClicked();

    private:
        struct FieldRow
        {
            QWidget* RowWidget = nullptr;
            QLabel* IndexLabel = nullptr;
            QComboBox* TypeCombo = nullptr;
            QWidget* ValueWidget = nullptr;
            QPushButton* RemoveButton = nullptr;
            ScriptStruct::FieldType Type = ScriptStruct::FieldType::INT;
        };

        QWidget* CreateValueWidget(ScriptStruct::FieldType type, const PipeCommands::PipeStructField* initial);
        void AddFieldRow(const PipeCommands::PipeStructField* initial);
        void RemoveFieldRow(QWidget* rowWidget);
        void RebuildValueWidgetForRow(FieldRow& row, ScriptStruct::FieldType newType);
        void UpdateIndicesAndSizes();
        PipeCommands::PipeStructField ExtractFieldValue(const FieldRow& row) const;

        QVBoxLayout* m_FieldsLayout;
        QLabel* m_SummaryLabel;
        std::vector<FieldRow> m_Rows;
    };
}