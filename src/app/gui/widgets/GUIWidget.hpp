#pragma once
#include <QApplication>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTabWidget>
#include <QWidget>

namespace scrDbgApp
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