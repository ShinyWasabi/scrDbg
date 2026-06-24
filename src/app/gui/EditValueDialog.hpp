#pragma once
#include <QDialog>

class QSpinBox;
class QDoubleSpinBox;
class QLineEdit;

namespace scrDbgApp
{
    class BitfieldWidget;

    enum ScriptValueType
    {
        INT,
        BIT_FIELD,
        FLOAT,
        TEXT_LABEL,
        STRING
    };

    class EditValueDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit EditValueDialog(Pointer pointer, ScriptValueType type, QWidget* parent = nullptr);

        void Apply();

    private:
        Pointer m_Pointer;
        ScriptValueType m_Type;

        QSpinBox* m_IntEditor = nullptr;
        BitfieldWidget* m_BitFieldEditor = nullptr;
        QDoubleSpinBox* m_FloatEditor = nullptr;
        QLineEdit* m_TextLabelEditor = nullptr;
        QLineEdit* m_StringEditor = nullptr;
    };
}