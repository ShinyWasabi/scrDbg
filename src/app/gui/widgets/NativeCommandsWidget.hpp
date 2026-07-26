#pragma once
#include "pipe/PipeCommands.hpp"
#include <QWidget>
#include <QtCore>

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QLabel;
class QFormLayout;
class QPushButton;

namespace scrDbgApp
{
    class NativeCommandsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit NativeCommandsWidget(QWidget* parent = nullptr);

    private slots:
        void OnSearchTextChanged(const QString& text);
        void OnResultActivated(QListWidgetItem* item);
        void OnCallClicked();

    private:
        static QString GetTypeName(NativeDB::Types type);

        void SelectNative(uint64_t hash, const QString& name);
        void RebuildArgumentInputs();
        void ClearArgumentInputs();
        void ClearReturnOutputs();
        void ClearOutValueOutputs();
        void ShowReturnValues(const std::vector<PipeCommands::PipeNativeReturn>& values);
        void ShowOutValues(const std::vector<PipeCommands::PipeNativeOutValue>& values, const std::vector<size_t>& argIndices);
        QWidget* CreateInputForType(NativeDB::Types type);
        QWidget* CreateOutputForType(const PipeCommands::PipeNativeReturn& value);
        QWidget* CreateOutValue(const PipeCommands::PipeNativeOutValue& value);

        QLineEdit* m_SearchEdit;
        QListWidget* m_ResultsList;
        QLabel* m_SelectedNativeLabel;
        QLineEdit* m_ScriptNameEdit;
        QFormLayout* m_ArgsLayout;
        QFormLayout* m_ReturnsLayout;
        QFormLayout* m_OutValuesLayout;
        QPushButton* m_CallButton;

        uint64_t m_SelectedHash = 0;
        std::vector<NativeDB::Types> m_ArgTypes;
        std::vector<QWidget*> m_ArgWidgets;
        std::vector<QWidget*> m_ReturnWidgets;
        std::vector<QWidget*> m_OutValueWidgets;
    };
}