#include "ScriptExport.hpp"
#include "core/Process.hpp"
#include "util/GUIHelpers.hpp"
#include "game/gta/Natives.hpp"
#include "game/gta/TextLabels.hpp"
#include "game/rage/scrThread.hpp"
#include "game/rage/scrProgram.hpp"
#include "game/rage/Joaat.hpp"
#include <QTableView>
#include <QTextStream>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QMessageBox>

// TO-DO: a lot of code repetition here, consider refactoring

namespace scrDbg::ScriptExport
{
    void ExportDisassembly(QTableView* view)
    {
        const int count = view->model()->rowCount();

        auto oldModel = view->model();

        GUIHelpers::ExportToFile("Disassembly", "disassembly.txt", count, [&](QTextStream & out, QProgressDialog & progress) {
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

    void ExportStatics(uint32_t scriptHash)
    {
        auto thread = rage::scrThread::GetThread(scriptHash);
        auto program = rage::scrProgram::GetProgram(scriptHash);
        if (!thread || !program)
            return;

        const uint32_t count = program.GetStaticCount();
        if (count == 0)
        {
            QMessageBox::warning(nullptr, "No Statics", "This script has no statics.");
            return;
        }

        GUIHelpers::ExportToFile("Statics", "statics.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
            for (uint32_t i = 0; i < count; i++)
            {
                if (progress.wasCanceled())
                    return;

                const int currentVal = static_cast<int>(thread.GetStack(i));
                const int defaultVal = static_cast<int>(program.GetStatic(i));
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

    void ExportGlobals(uint32_t scriptHash, bool exportAll)
    {
        if (exportAll)
        {
            int lastValidBlock = -1;
            int totalGlobalCount = 0;
            for (int i = 0; i < 64; i++)
            {
                int blockCount = rage::scrProgram::GetGlobalBlockCount(i);
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

            GUIHelpers::ExportToFile("All Globals", "all_globals.txt", totalGlobalCount, [&](QTextStream& out, QProgressDialog& progress) {
                for (int block = 0; block <= lastValidBlock; block++)
                {
                    int blockCount = rage::scrProgram::GetGlobalBlockCount(block);
                    if (blockCount == 0)
                        continue;

                    out << "// Block " << block << " (Count " << blockCount << ")\n";

                    for (int i = 0; i < blockCount; i++)
                    {
                        if (progress.wasCanceled())
                            return;

                        int globalIndex = (block << 18) + i;
                        int value = static_cast<int>(rage::scrProgram::GetGlobal(globalIndex));
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
            auto program = rage::scrProgram::GetProgram(scriptHash);
            if (!program)
                return;

            const uint32_t block = program.GetGlobalBlockIndex();
            const uint32_t count = program.GetGlobalCount();
            if (count == 0)
            {
                QMessageBox::warning(nullptr, "No Globals", "This script has no globals.");
                return;
            }

            GUIHelpers::ExportToFile("Globals", "globals.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
                for (uint32_t i = 0; i < count; i++)
                {
                    if (progress.wasCanceled())
                        return;

                    int globalIndex = (block << 0x12) + i;
                    int currentVal = static_cast<int>(rage::scrProgram::GetGlobal(globalIndex));
                    int defaultVal = static_cast<int>(program.GetProgramGlobal(i));
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

    void ExportNatives(uint32_t scriptHash, bool exportAll)
    {
        if (exportAll)
        {
            auto allNatives = gta::Natives::GetAll();

            const uint32_t count = static_cast<uint32_t>(allNatives.size());
            if (count == 0)
            {
                QMessageBox::warning(nullptr, "No Natives", "No natives found.");
                return;
            }

            GUIHelpers::ExportToFile("Natives", "natives.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
                uint32_t index = 0;
                for (auto& [hash, handler] : allNatives)
                {
                    if (progress.wasCanceled())
                        return;

                    out << "0x" << QString::number(hash, 16).toUpper();
                    out << ":" << QString::fromStdString(Process::GetName()) << "+0x" << QString::number(handler - Process::GetBaseAddress(), 16).toUpper();

                    auto name = std::string(gta::Natives::GetNameByHash(hash));
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
            auto program = rage::scrProgram::GetProgram(scriptHash);
            if (!program)
                return;

            const uint32_t count = program.GetNativeCount();
            if (count == 0)
            {
                QMessageBox::warning(nullptr, "No Natives", "This script has no natives.");
                return;
            }

            GUIHelpers::ExportToFile("Natives", "natives.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
                for (uint32_t i = 0; i < count; i++)
                {
                    if (progress.wasCanceled())
                        return;

                    uint64_t handler = program.GetNative(i);
                    uint64_t hash = gta::Natives::GetHashByHandler(handler);
                    out << "0x" << QString::number(hash, 16).toUpper();
                    out << ":" << QString::fromStdString(Process::GetName()) << "+0x" << QString::number(handler - Process::GetBaseAddress(), 16).toUpper();

                    auto name = std::string(gta::Natives::GetNameByHash(hash));
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

    void ExportStrings(uint32_t scriptHash, bool onlyTextLabels)
    {
        auto program = rage::scrProgram::GetProgram(scriptHash);
        if (!program)
            return;

        const auto strings = program.GetAllStrings();
        const int count = static_cast<int>(strings.size());
        if (count == 0)
        {
            QMessageBox::warning(nullptr, "No Strings", "This script has no strings.");
            return;
        }

        int exportedCount = 0;
        GUIHelpers::ExportToFile(onlyTextLabels ? "Text Labels" : "Strings", onlyTextLabels ? "text_labels.txt" : "strings.txt", count, [&](QTextStream& out, QProgressDialog& progress) {
            for (int i = 0; i < count; i++)
            {
                if (progress.wasCanceled())
                    return;

                const std::string& s = strings[i];

                if (onlyTextLabels)
                {
                    const uint32_t hash = RAGE_JOAAT(s);
                    const std::string label = gta::TextLabels::GetTextLabel(hash);
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