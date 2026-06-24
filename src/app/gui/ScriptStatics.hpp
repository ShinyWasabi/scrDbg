#pragma once
#include <QWidget>
#include <QtCore>

class QLineEdit;
class QSpinBox;
class QComboBox;
class QStackedWidget;
class QDoubleSpinBox;
class QPushButton;
class QTimer;

namespace scrDbgApp
{
    class BitfieldWidget;

    class ScriptStaticsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit ScriptStaticsWidget(QWidget* parent = nullptr);

    private slots:
        void OnUpdateCurrentStaticValue();
        void OnEditValue();
        void OnReset();

    private:
        static bool IsStaticAddressValid(uint32_t script, int address);
        int ComputeStaticAddress();

        QLineEdit* m_ScriptName;
        QSpinBox* m_StaticIndex;
        QVector<QSpinBox*> m_StaticOffsets;
        QVector<QSpinBox*> m_StaticSizes;

        QComboBox* m_ValueType;

        QStackedWidget* m_CurrentValueStack;
        QSpinBox* m_CurrentValueInt;
        BitfieldWidget* m_CurrentValueBitfield;
        QDoubleSpinBox* m_CurrentValueFloat;
        QLineEdit* m_CurrentValueTextLabel;
        QLineEdit* m_CurrentValueString;

        QPushButton* m_EditValueButton;
        QPushButton* m_ResetButton;
        QTimer* m_UpdateTimer;
    };
}