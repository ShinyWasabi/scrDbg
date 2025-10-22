#pragma once
#include <QWidget>
#include <QtCore>

class QPushButton;
class QLineEdit;

namespace scrDbg
{
	class ScriptGlobalsWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit ScriptGlobalsWidget(QWidget* parent = nullptr);

	private slots:
		void OnWriteNewGlobalValue();
		void OnUpdateCurrentGlobalValue();

	private:
		int ComputeGlobalAddress();

		QLineEdit* m_GlobalIndex;
		QVector<QLineEdit*> m_GlobalOffsets;
		QVector<QLineEdit*> m_GlobalSizes;
		QLineEdit* m_GlobalNewValue;
		QLineEdit* m_GlobalCurrentValue;
		QPushButton* m_WriteGlobal;

		QTimer* m_UpdateTimer;
	};
}