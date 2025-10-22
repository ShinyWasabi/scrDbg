#pragma once
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include <QWidget>
#include <QTabWidget>

namespace scrDbg
{
    class GUIWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit GUIWidget(QWidget* parent = nullptr);

    private:
        QTabWidget* m_MainWidget;

        QTimer* m_UpdateTimer;
    };
}