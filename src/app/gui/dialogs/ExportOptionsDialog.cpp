#include "ExportOptionsDialog.hpp"
#include <QCheckBox>
#include <QCoreApplication>
#include <QFileDialog>
#include <QGridLayout>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QTableView>
#include <algorithm>

namespace scrDbgApp
{
    ExportOptionsDialog::ExportOptionsDialog(uint32_t scriptHash, QTableView* disassembly, QWidget* parent)
        : QDialog(parent)
    {
        setWindowTitle("Export Options");

        m_ExportDisassembly = new QPushButton("Export Disassembly");
        m_ExportDisassembly->setToolTip("Export the disassembly of this script program.");

        m_ExportStatics = new QPushButton("Export Statics");
        m_ExportStatics->setToolTip("Export the static variables of this script program.");

        m_ExportGlobals = new QPushButton("Export Globals");
        m_ExportGlobals->setToolTip("Export the global variables of this script program.");

        m_ExportNatives = new QPushButton("Export Natives");
        m_ExportNatives->setToolTip("Export the native commands of this script program.");

        m_ExportStrings = new QPushButton("Export Strings");
        m_ExportStrings->setToolTip("Export the string literals of this script program.");

        int maxButtonWidth = std::max({m_ExportDisassembly->sizeHint().width(),
            m_ExportStatics->sizeHint().width(),
            m_ExportGlobals->sizeHint().width(),
            m_ExportNatives->sizeHint().width(),
            m_ExportStrings->sizeHint().width()});

        m_ExportDisassembly->setMinimumWidth(maxButtonWidth);
        m_ExportStatics->setMinimumWidth(maxButtonWidth);
        m_ExportGlobals->setMinimumWidth(maxButtonWidth);
        m_ExportNatives->setMinimumWidth(maxButtonWidth);
        m_ExportStrings->setMinimumWidth(maxButtonWidth);

        m_ExportAllGlobals = new QCheckBox("Export all");
        m_ExportAllGlobals->setToolTip("Export all the global blocks.");

        m_ExportAllNatives = new QCheckBox("Export all");
        m_ExportAllNatives->setToolTip("Export all the native commands in the game.");

        m_OnlyTextLabels = new QCheckBox("Only text labels");
        m_OnlyTextLabels->setToolTip("Export only text labels with their translations.");

        QGridLayout* grid = new QGridLayout(this);

        int row = 0;
        grid->addWidget(m_ExportDisassembly, row++, 0);
        grid->addWidget(m_ExportStatics, row++, 0);
        grid->addWidget(m_ExportGlobals, row, 0);
        grid->addWidget(m_ExportAllGlobals, row++, 1);
        grid->addWidget(m_ExportNatives, row, 0);
        grid->addWidget(m_ExportAllNatives, row++, 1);
        grid->addWidget(m_ExportStrings, row, 0);
        grid->addWidget(m_OnlyTextLabels, row++, 1);

        connect(m_ExportDisassembly, &QPushButton::clicked, this, [disassembly]() {
            ExportDisassembly(disassembly);
        });

        connect(m_ExportStatics, &QPushButton::clicked, this, [scriptHash]() {
            ExportStatics(scriptHash);
        });

        connect(m_ExportGlobals, &QPushButton::clicked, this, [this, scriptHash]() {
            ExportGlobals(scriptHash, m_ExportAllGlobals->isChecked());
        });

        connect(m_ExportNatives, &QPushButton::clicked, this, [this, scriptHash]() {
            ExportNatives(scriptHash, m_ExportAllNatives->isChecked());
        });

        connect(m_ExportStrings, &QPushButton::clicked, this, [this, scriptHash]() {
            ExportStrings(scriptHash, m_OnlyTextLabels->isChecked());
        });
    }

    void ExportOptionsDialog::ExportToFile(const QString& title, const QString& filename, int count, std::function<void(QTextStream&, QProgressDialog&)> cb)
    {
        QString name = QFileDialog::getSaveFileName(nullptr, title, filename, "Text Files (*.txt);;All Files (*)");
        if (name.isEmpty())
            return;

        QFile file(name);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(nullptr, title, "Failed to open file for writing.");
            return;
        }

        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);

        QProgressDialog progress(QString("Exporting %1...").arg(title), "Cancel", 0, count, nullptr);
        progress.setWindowModality(Qt::ApplicationModal);
        progress.setMinimumDuration(200);
        progress.setValue(0);

        cb(out, progress);
        file.close();
    }

    void ExportOptionsDialog::ExportDisassembly(QTableView* view)
    {
        const int count = view->model()->rowCount();

        auto oldModel = view->model();

        ExportToFile("Disassembly", "disassembly.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
            for (int row = 0; row < count; row++)
            {
                if (progress.wasCanceled())
                    return;

                auto newModel = view->model();
                if (oldModel != newModel)
                {
                    QMessageBox::critical(nullptr, "Error", "Disassembly changed during export.");
                    return;
                }

                const QString addr = newModel->data(newModel->index(row, 0)).toString();
                const QString bytes = newModel->data(newModel->index(row, 1)).toString();
                const QString instr = newModel->data(newModel->index(row, 2)).toString();

                out << QString("%1  %2  %3\n").arg(addr, -10).arg(bytes, -25).arg(instr);

                if (row % 50 == 0)
                {
                    progress.setValue(row);
                    QCoreApplication::processEvents();
                }
            }

            progress.setValue(count);
            QMessageBox::information(nullptr, "Export Disassembly", QString("Exported %1 instructions.").arg(count));
        });
    }

    void ExportOptionsDialog::ExportStatics(uint32_t scriptHash)
    {
        auto thread = g_Game->GetThread(scriptHash);
        auto program = g_Game->GetProgram(scriptHash);
        if (!thread || !program)
            return;

        const uint32_t count = program->GetStaticCount();
        if (count == 0)
        {
            QMessageBox::warning(nullptr, "No Statics", "This script has no statics.");
            return;
        }

        ExportToFile("Statics", "statics.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
            for (uint32_t i = 0; i < count; i++)
            {
                if (progress.wasCanceled())
                    return;

                const int currentVal = thread->GetStack(i).Get<int32_t>();
                const int defaultVal = program->GetStatic(i).Get<int32_t>();
                out << "Static_" << i << " = " << currentVal << " // Default: " << defaultVal << "\n";

                if (i % 50 == 0)
                {
                    progress.setValue(i);
                    QCoreApplication::processEvents();
                }
            }

            progress.setValue(count);
            QMessageBox::information(nullptr, "Export Statics", QString("Exported %1 statics.").arg(count));
        });
    }

    void ExportOptionsDialog::ExportGlobals(uint32_t scriptHash, bool exportAll)
    {
        if (exportAll)
        {
            int lastValidBlock = -1;
            int totalGlobalCount = 0;
            for (int i = 0; i < 64; i++)
            {
                int blockCount = g_Game->GetGlobalBlockCount(i);
                if (blockCount > 0)
                {
                    lastValidBlock = i;
                    totalGlobalCount += blockCount;
                }
            }

            if (lastValidBlock == -1)
            {
                QMessageBox::warning(nullptr, "No Blocks", "No valid global blocks found.");
                return;
            }

            ExportToFile("All Globals", "all_globals.txt", totalGlobalCount, [&](QTextStream& out, QProgressDialog& progress) {
                for (int block = 0; block <= lastValidBlock; block++)
                {
                    int blockCount = g_Game->GetGlobalBlockCount(block);
                    if (blockCount == 0)
                        continue;

                    out << "// Block " << block << " (Count " << blockCount << ")\n";

                    for (int i = 0; i < blockCount; i++)
                    {
                        if (progress.wasCanceled())
                            return;

                        int globalIndex = (block << 18) + i;
                        int value = g_Game->GetGlobal(globalIndex).Get<int32_t>();
                        out << "Global_" << globalIndex << " = " << value << "\n";

                        if (globalIndex % 50 == 0)
                        {
                            progress.setValue(globalIndex);
                            QCoreApplication::processEvents();
                        }
                    }
                }

                progress.setValue(totalGlobalCount);
                QMessageBox::information(nullptr, "Export Globals", QString("Exported %1 blocks (%2 globals total).").arg(lastValidBlock).arg(totalGlobalCount));
            });
        }
        else
        {
            auto program = g_Game->GetProgram(scriptHash);
            if (!program)
                return;

            const uint32_t block = program->GetGlobalBlock();
            const uint32_t count = program->GetGlobalCount();
            if (count == 0)
            {
                QMessageBox::warning(nullptr, "No Globals", "This script has no globals.");
                return;
            }

            ExportToFile("Globals", "globals.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
                for (uint32_t i = 0; i < count; i++)
                {
                    if (progress.wasCanceled())
                        return;

                    int globalIndex = (block << 0x12) + i;
                    int currentVal = g_Game->GetGlobal(globalIndex).Get<int32_t>();
                    int defaultVal = program->GetProgramGlobal(i).Get<int32_t>();
                    out << "Global_" << globalIndex << " = " << currentVal << " // Default: " << defaultVal << "\n";

                    if (i % 50 == 0)
                    {
                        progress.setValue(i);
                        QCoreApplication::processEvents();
                    }
                }

                progress.setValue(count);
                QMessageBox::information(nullptr, "Export Globals", QString("Exported %1 globals.").arg(count));
            });
        }
    }

    void ExportOptionsDialog::ExportNatives(uint32_t scriptHash, bool exportAll)
    {
        if (exportAll)
        {
            auto allNatives = g_Game->GetAllNatives();

            const uint32_t count = static_cast<uint32_t>(allNatives.size());
            if (count == 0)
            {
                QMessageBox::warning(nullptr, "No Natives", "No natives found.");
                return;
            }

            ExportToFile("Natives", "natives.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
                uint32_t index = 0;
                for (auto& [hash, handler] : allNatives)
                {
                    if (progress.wasCanceled())
                        return;

                    out << "0x" << QString::number(hash, 16).toUpper();
                    out << ":" << QString::fromStdString(Process::GetName()) << "+0x" << QString::number(handler - Process::GetBaseAddress(), 16).toUpper();

                    auto name = std::string(g_Game->GetNativeNameByHash(hash));
                    out << " // " << (name.empty() ? "UNKNOWN_NATIVE" : QString::fromStdString(name)) << "\n";

                    if (index % 50 == 0)
                    {
                        progress.setValue(index);
                        QCoreApplication::processEvents();
                    }
                    index++;
                }

                progress.setValue(count);
                QMessageBox::information(nullptr, "Export Natives", QString("Exported %1 natives.").arg(count));
            });
        }
        else
        {
            auto program = g_Game->GetProgram(scriptHash);
            if (!program)
                return;

            const uint32_t count = program->GetNativeCount();
            if (count == 0)
            {
                QMessageBox::warning(nullptr, "No Natives", "This script has no natives.");
                return;
            }

            ExportToFile("Natives", "natives.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
                for (uint32_t i = 0; i < count; i++)
                {
                    if (progress.wasCanceled())
                        return;

                    uint64_t handler = program->GetNative(i);
                    uint64_t hash = g_Game->GetNativeHashByHandler(handler);
                    out << "0x" << QString::number(hash, 16).toUpper();
                    out << ":" << QString::fromStdString(Process::GetName()) << "+0x" << QString::number(handler - Process::GetBaseAddress(), 16).toUpper();

                    auto name = std::string(g_Game->GetNativeNameByHash(hash));
                    out << " // " << (name.empty() ? "UNKNOWN_NATIVE" : QString::fromStdString(name)) << "\n";

                    if (i % 50 == 0)
                    {
                        progress.setValue(i);
                        QCoreApplication::processEvents();
                    }
                }

                progress.setValue(count);
                QMessageBox::information(nullptr, "Export Natives", QString("Exported %1 natives.").arg(count));
            });
        }
    }

    void ExportOptionsDialog::ExportStrings(uint32_t scriptHash, bool onlyTextLabels)
    {
        auto program = g_Game->GetProgram(scriptHash);
        if (!program)
            return;

        const auto strings = program->GetStrings();
        const int count = static_cast<int>(strings.size());
        if (count == 0)
        {
            QMessageBox::warning(nullptr, "No Strings", "This script has no strings.");
            return;
        }

        int exportedCount = 0;
        ExportToFile(onlyTextLabels ? "Text Labels" : "Strings", onlyTextLabels ? "text_labels.txt" : "strings.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
            for (int i = 0; i < count; i++)
            {
                if (progress.wasCanceled())
                    return;

                const std::string& s = strings[i];

                if (onlyTextLabels)
                {
                    const uint32_t hash = JOAAT(s);
                    const std::string label = g_Game->GetTextLabel(hash);
                    if (!label.empty())
                    {
                        out << QString("%1 (0x%2): %3\n").arg(QString::fromStdString(s)).arg(QString::number(hash, 16).toUpper()).arg(QString::fromStdString(label));
                        ++exportedCount;
                    }
                }
                else
                {
                    out << QString::fromStdString(s) << '\n';
                    ++exportedCount;
                }

                if (i % 50 == 0)
                {
                    progress.setValue(i);
                    QCoreApplication::processEvents();
                }
            }

            progress.setValue(count);
            QMessageBox::information(nullptr, "Export Strings", QString("Exported %1 strings.").arg(exportedCount));
        });
    }
}