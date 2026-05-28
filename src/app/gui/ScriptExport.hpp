#pragma once

class QString;
class QTextStream;
class QProgressDialog;
class QTableView;

namespace scrDbgApp::ScriptExport
{
    void ExportToFile(const QString& title, const QString& filename, int count, std::function<void(QTextStream&, QProgressDialog&)> cb);
    void ExportDisassembly(QTableView* view);
    void ExportStatics(uint32_t scriptHash);
    void ExportGlobals(uint32_t scriptHash, bool exportAll);
    void ExportNatives(uint32_t scriptHash, bool exportAll);
    void ExportStrings(uint32_t scriptHash, bool onlyTextLabels);
}