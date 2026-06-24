#pragma once
#include <QWidget>
#include <QtCore>

class QSpinBox;
class QComboBox;
class QStackedWidget;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QTimer;

namespace scrDbgApp
{
    class BitfieldWidget;

    class ScriptGlobalsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit ScriptGlobalsWidget(QWidget* parent = nullptr);

    private slots:
        void OnUpdateCurrentGlobalValue();
        void OnEditValue();
        void OnReset();

    private:
        static bool IsGlobalAddressValid(int address);
        int ComputeGlobalAddress();

        QSpinBox* m_GlobalIndex;
        QVector<QSpinBox*> m_GlobalOffsets;
        QVector<QSpinBox*> m_GlobalSizes;

        QComboBox* m_ValueType;

        QStackedWidget* m_CurrentValueStack;
        QSpinBox* m_CurrentValueInt;
        BitfieldWidget* m_CurrentValueBitfield;
        QDoubleSpinBox* m_CurrentValueFloat;
        QLineEdit* m_CurrentValueTextLabel;

        QPushButton* m_EditValueButton;
        QPushButton* m_ResetButton;
        QTimer* m_UpdateTimer;
    };
}