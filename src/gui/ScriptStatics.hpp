#pragma once
#include <QWidget>
#include <QtCore>

class QPushButton;
class QLineEdit;

namespace scrDbg
{
	class ScriptStaticsWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit ScriptStaticsWidget(QWidget* parent = nullptr);

	private slots:
		void OnWriteNewStaticValue();
		void OnUpdateCurrentStaticValue();

	private:
		int ComputeStaticAddress();

		QLineEdit* m_StaticScriptName;
		QLineEdit* m_StaticIndex;
		QVector<QLineEdit*> m_StaticOffsets;
		QVector<QLineEdit*> m_StaticSizes;
		QLineEdit* m_StaticNewValue;
		QLineEdit* m_StaticCurrentValue;
		QPushButton* m_WriteStatic;

		QTimer* m_UpdateTimer;
	};
}