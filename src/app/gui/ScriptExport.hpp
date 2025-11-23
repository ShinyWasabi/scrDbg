#pragma once

class QTableView;

namespace scrDbgApp::ScriptExport
{
    extern void ExportDisassembly(QTableView* view);
    extern void ExportStatics(uint32_t scriptHash);
    extern void ExportGlobals(uint32_t scriptHash, bool exportAll);
    extern void ExportNatives(uint32_t scriptHash, bool exportAll);
    extern void ExportStrings(uint32_t scriptHash, bool onlyTextLabels);
}