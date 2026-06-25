#pragma once
#include <QDialog>
#include <functional>

class QPushButton;
class QCheckBox;
class QTableView;
class QString;
class QTextStream;
class QProgressDialog;

namespace scrDbgApp
{
    class ExportOptionsDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit ExportOptionsDialog(uint32_t scriptHash, QTableView* disassembly, QWidget* parent = nullptr);

    private:
        static void ExportToFile(const QString& title, const QString& filename, int count, std::function<void(QTextStream&, QProgressDialog&)> cb);
        static void ExportDisassembly(QTableView* view);
        static void ExportStatics(uint32_t scriptHash);
        static void ExportGlobals(uint32_t scriptHash, bool exportAll);
        static void ExportNatives(uint32_t scriptHash, bool exportAll);
        static void ExportStrings(uint32_t scriptHash, bool onlyTextLabels);

        QPushButton* m_ExportDisassembly;
        QPushButton* m_ExportStatics;
        QPushButton* m_ExportGlobals;
        QPushButton* m_ExportNatives;
        QPushButton* m_ExportStrings;
        QCheckBox* m_ExportAllGlobals;
        QCheckBox* m_ExportAllNatives;
        QCheckBox* m_OnlyTextLabels;
    };
}