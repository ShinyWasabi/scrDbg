#include "GUI.hpp"
#include "ScriptThreads.hpp"
#include "ScriptStatics.hpp"
#include "ScriptGlobals.hpp"
#include "core/Process.hpp"
#include "util/Misc.hpp"
#include "resource/scrDbg.h"
#include <QVBoxLayout>

namespace scrDbg
{
    GUIWidget::GUIWidget(QWidget* parent)
        : QWidget(parent)
    {
        setWindowTitle(QString("scrDbg %1 - %2 %3").arg(SCRDBG_FILE_VERSION).arg(Process::GetName()).arg(Misc::GetGameBuild()));
        resize(900, 780);

        m_MainWidget = new QTabWidget(this);

        m_MainWidget->addTab(new ScriptThreadsWidget(this), "Script Threads");
        m_MainWidget->addTab(new ScriptStaticsWidget(this), "Script Statics");
        m_MainWidget->addTab(new ScriptGlobalsWidget(this), "Script Globals");

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(m_MainWidget);
        setLayout(mainLayout);

        m_UpdateTimer = new QTimer(this);
        m_UpdateTimer->setInterval(500);
        connect(m_UpdateTimer, &QTimer::timeout, [this]() {
            if (!Process::IsRunning())
            {
                QMessageBox::information(this, "Game Closed", "The game process has exited. scrDbg will now close.");
                QTimer::singleShot(0, qApp, &QApplication::quit);
            }
        });
        m_UpdateTimer->start();
    }
}