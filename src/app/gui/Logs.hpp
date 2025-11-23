#pragma once
#include <QWidget>
#include <QtCore>

class QPlainTextEdit;
class QLabel;
class QTimer;

namespace scrDbgApp
{
    class LogsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit LogsWidget(QWidget* parent = nullptr);

    private:
        enum LogType
        {
            LOG_TYPE_NONE,
            LOG_TYPE_FRAME_TIME,
            LOG_TYPE_NATIVE_CALLS,
            LOG_TYPE_STATIC_WRITES,
            LOG_TYPE_GLOBAL_WRITES
        };

        void UpdateLog();

        QPlainTextEdit* m_TextEdit;
        QLabel* m_CurrentScript;
        QTimer* m_Timer;

        QStringList m_LogBuffer;
        qint64 m_LastPos;
    };
}