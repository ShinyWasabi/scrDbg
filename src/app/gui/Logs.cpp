#include "Logs.hpp"
#include "game/rage/Joaat.hpp"
#include "pipe/PipeCommands.hpp"
#include <QButtonGroup>
#include <QCheckBox>
#include <QDesktopServices>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

namespace scrDbgApp
{
    LogsWidget::LogsWidget(QWidget* parent)
        : QWidget(parent),
          m_LastPos(0)
    {
        m_CurrentScript = new QLabel("Current Script: ALL", this);

        auto scriptInput = new QLineEdit(this);
        scriptInput->setFixedWidth(200);
        scriptInput->setEnabled(false);
        scriptInput->setPlaceholderText("Enter script name to filter by");

        auto setScriptBtn = new QPushButton("Set Script", this);
        setScriptBtn->setFixedWidth(70);
        connect(setScriptBtn, &QPushButton::clicked, [this, scriptInput]() {
            QString name = scriptInput->text().trimmed();
            if (!name.isEmpty())
            {
                uint32_t hash = RAGE_JOAAT(name.toStdString().c_str());
                PipeCommands::SetLoggerScript(hash);

                m_CurrentScript->setText("Current Script: " + name);
            }
        });

        auto allScriptsCb = new QCheckBox("All Scripts", this);
        allScriptsCb->setChecked(true);
        connect(allScriptsCb, &QCheckBox::toggled, [this, scriptInput](bool checked) {
            if (checked)
            {
                PipeCommands::SetLoggerScript(0);
                m_CurrentScript->setText("Current Script: ALL");
                scriptInput->clear();
                scriptInput->setEnabled(false);
            }
            else
            {
                scriptInput->setEnabled(true);
            }
        });

        auto scriptLayout = new QHBoxLayout();
        scriptLayout->setSpacing(10);
        scriptLayout->addWidget(scriptInput);
        scriptLayout->addWidget(setScriptBtn);
        scriptLayout->addWidget(allScriptsCb);
        scriptLayout->addStretch();

        auto noneRb = new QRadioButton("None", this);
        noneRb->setChecked(true);
        auto frameTimeRb = new QRadioButton("Frame Time", this);
        auto functionCallsRb = new QRadioButton("Function Calls", this);
        auto nativeCallsRb = new QRadioButton("Native Calls", this);
        auto staticWritesRb = new QRadioButton("Static Writes", this);
        auto globalWritesRb = new QRadioButton("Global Writes", this);

        auto logTypeGroup = new QButtonGroup(this);
        logTypeGroup->addButton(noneRb, LOG_TYPE_NONE);
        logTypeGroup->addButton(frameTimeRb, LOG_TYPE_FRAME_TIME);
        logTypeGroup->addButton(functionCallsRb, LOG_TYPE_FUNCTION_CALLS);
        logTypeGroup->addButton(nativeCallsRb, LOG_TYPE_NATIVE_CALLS);
        logTypeGroup->addButton(staticWritesRb, LOG_TYPE_STATIC_WRITES);
        logTypeGroup->addButton(globalWritesRb, LOG_TYPE_GLOBAL_WRITES);
        connect(logTypeGroup, QOverload<int>::of(&QButtonGroup::idClicked), [this](int id) {
            PipeCommands::SetLoggerType(id);
        });

        auto typeLayout = new QHBoxLayout();
        typeLayout->setSpacing(10);
        typeLayout->addWidget(new QLabel("Type:", this));
        typeLayout->addWidget(noneRb);
        typeLayout->addWidget(frameTimeRb);
        typeLayout->addWidget(functionCallsRb);
        typeLayout->addWidget(nativeCallsRb);
        typeLayout->addWidget(staticWritesRb);
        typeLayout->addWidget(globalWritesRb);
        typeLayout->addStretch();

        auto refreshBtn = new QPushButton("Refresh", this);
        connect(refreshBtn, &QPushButton::clicked, this, &LogsWidget::UpdateLog);

        auto clearBtn = new QPushButton("Clear", this);
        connect(clearBtn, &QPushButton::clicked, [this]() {
            m_LogBuffer.clear();
            m_TextEdit->clear();
            m_LastPos = 0;
            PipeCommands::ClearLoggerFile();
        });

        auto openFolderBtn = new QPushButton("Open Log Folder", this);
        connect(openFolderBtn, &QPushButton::clicked, []() {
            QDir dir(QDir::currentPath()); // it'll be at the current path
            QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absolutePath()));
        });

        auto autoUpdateCb = new QCheckBox("Auto Update", this);
        autoUpdateCb->setChecked(true);
        connect(autoUpdateCb, &QCheckBox::toggled, [this](bool checked) {
            checked ? m_Timer->start(250) : m_Timer->stop();
        });

        auto buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(refreshBtn);
        buttonLayout->addWidget(clearBtn);
        buttonLayout->addWidget(openFolderBtn);
        buttonLayout->addWidget(autoUpdateCb);
        buttonLayout->addStretch();

        m_TextEdit = new QPlainTextEdit(this);
        m_TextEdit->setReadOnly(true);
        m_TextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
        m_TextEdit->setMinimumHeight(350);

        m_Timer = new QTimer(this);
        connect(m_Timer, &QTimer::timeout, this, &LogsWidget::UpdateLog);
        m_Timer->start(250);

        auto layout = new QVBoxLayout(this);
        layout->addWidget(m_CurrentScript);
        layout->addLayout(scriptLayout);
        layout->addLayout(typeLayout);
        layout->addLayout(buttonLayout);
        layout->addWidget(m_TextEdit);
        setLayout(layout);
    }

    void LogsWidget::UpdateLog()
    {
        QFile file("scrDbg.log");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        file.seek(m_LastPos);
        QTextStream in(&file);

        QStringList newLines;
        while (!in.atEnd())
            newLines.append(in.readLine());

        m_LastPos = file.pos();
        if (newLines.isEmpty())
            return;

        m_LogBuffer.append(newLines);

        // Keep max 1000 lines
        if (m_LogBuffer.size() > 1000)
            m_LogBuffer = m_LogBuffer.mid(m_LogBuffer.size() - 1000);

        m_TextEdit->setPlainText(m_LogBuffer.join("\n"));
        m_TextEdit->verticalScrollBar()->setValue(m_TextEdit->verticalScrollBar()->maximum());
    }
}