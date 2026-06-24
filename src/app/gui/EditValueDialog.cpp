#include "EditValueDialog.hpp"
#include "BitfieldWidget.hpp"
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

namespace scrDbgApp
{
    EditValueDialog::EditValueDialog(Pointer pointer, ScriptValueType type, QWidget* parent)
        : QDialog(parent),
          m_Pointer(pointer),
          m_Type(type)
    {
        setWindowTitle("Edit Value");

        QVBoxLayout* layout = new QVBoxLayout(this);
        QWidget* editor = nullptr;

        switch (m_Type)
        {
        case ScriptValueType::INT:
        {
            m_IntEditor = new QSpinBox(this);
            m_IntEditor->setRange(INT_MIN, INT_MAX);
            m_IntEditor->setButtonSymbols(QAbstractSpinBox::NoButtons);
            m_IntEditor->setValue(m_Pointer.Get<int32_t>());
            editor = m_IntEditor;
            break;
        }
        case ScriptValueType::BIT_FIELD:
        {
            m_BitFieldEditor = new BitfieldWidget(this);
            m_BitFieldEditor->SetValue(m_Pointer.Get<int32_t>());
            editor = m_BitFieldEditor;
            break;
        }
        case ScriptValueType::FLOAT:
        {
            m_FloatEditor = new QDoubleSpinBox(this);
            m_FloatEditor->setRange(-FLT_MAX, FLT_MAX);
            m_FloatEditor->setDecimals(6);
            m_FloatEditor->setButtonSymbols(QAbstractSpinBox::NoButtons);
            m_FloatEditor->setValue(m_Pointer.Get<float>());
            editor = m_FloatEditor;
            break;
        }
        case ScriptValueType::TEXT_LABEL:
        {
            m_TextLabelEditor = new QLineEdit(this);
            m_TextLabelEditor->setMaxLength(64);
            m_TextLabelEditor->setText(QString::fromStdString(m_Pointer.GetString(64)));
            editor = m_TextLabelEditor;
            break;
        }
        case ScriptValueType::STRING:
        {
            auto ptr = m_Pointer.Deref();
            m_StringEditor = new QLineEdit(this);
            m_StringEditor->setMaxLength(255);
            if (ptr)
                m_StringEditor->setText(QString::fromStdString(ptr.GetString(256)));
            else
                m_StringEditor->setText("");
            editor = m_StringEditor;
            break;
        }
        }

        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        layout->addWidget(editor);
        layout->addWidget(buttons);
    }

    void EditValueDialog::Apply()
    {
        switch (m_Type)
        {
        case ScriptValueType::INT:
            m_Pointer.Set<int32_t>(m_IntEditor->value());
            break;
        case ScriptValueType::BIT_FIELD:
            m_Pointer.Set<uint32_t>(static_cast<uint32_t>(m_BitFieldEditor->Value()));
            break;
        case ScriptValueType::FLOAT:
            m_Pointer.Set<float>(static_cast<float>(m_FloatEditor->value()));
            break;
        case ScriptValueType::TEXT_LABEL:
        {
            std::string str = m_TextLabelEditor->text().toStdString();
            m_Pointer.SetBuffer(str.c_str(), str.size() + 1);
            break;
        }
        case ScriptValueType::STRING:
        {
            // Unsupported, we would have to allocate memory
            QMessageBox::critical(this, "Unsupported", "STRING editing is currently not supported.");
            break;
        }
        }
    }
}