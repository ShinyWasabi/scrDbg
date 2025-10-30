#pragma once
#include "ScriptLayout.hpp"
#include <QWidget>
#include <QtCore>

class QLabel;
class QComboBox;
class QPushButton;
class QLineEdit;
class QSortFilterProxyModel;
class QTableView;
class QTextStream;
class QProgressDialog;
namespace rage
{
    class scrProgram;
}

namespace scrDbg
{
	class ScriptThreadsWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit ScriptThreadsWidget(QWidget* parent = nullptr);

    private slots:
        void OnUpdateScripts();
        void OnRefreshDisassembly(const rage::scrProgram& program, bool resetScroll);
        void OnTogglePauseScript();
        void OnKillScript();
        void OnExportOptionsDialog();
        void OnExportDisassembly();
        void OnExportStatics();
        void OnExportGlobals(bool exportAll);
        void OnExportNatives(bool exportAll);
        void OnExportStrings(bool onlyTextLabels);
        void OnJumpToAddress();
        void OnBinarySearch();
        void OnBreakpointsDialog();
        void OnDisassemblyContextMenu(const QPoint& pos);
        void OnCopyInstruction(const QModelIndex& index);
        void OnNopInstruction(const QModelIndex& index);
        void OnPatchInstruction(const QModelIndex& index);
        void OnGeneratePattern(const QModelIndex& index);
        void OnViewXrefsDialog(const QModelIndex& index);
        void OnJumpToInstructionAddress(const QModelIndex& index);
        void OnSetBreakpoint(const QModelIndex& index, bool set);

	private:
        uint32_t GetCurrentScriptHash();
        bool ScrollToAddress(uint32_t address);
        void ExportToFile(const QString& title, const QString& filename, int count, std::function<void(QTextStream&, QProgressDialog&)> cb);
        void UpdateCurrentScript();

        uint32_t m_LastScriptHash;
        std::unique_ptr<ScriptLayout> m_Layout;

        QLabel* m_State;
        QLabel* m_Priority;
        QLabel* m_Program;
        QLabel* m_ThreadId;
        QLabel* m_ProgramCounter;
        QLabel* m_FramePointer;
        QLabel* m_StackPointer;
        QLabel* m_StackSize;
        QLabel* m_GlobalVersion;
        QLabel* m_CodeSize;
        QLabel* m_ArgCount;
        QLabel* m_StaticCount;
        QLabel* m_GlobalCount;
        QLabel* m_GlobalBlock;
        QLabel* m_NativeCount;
        QLabel* m_StringCount;
        QComboBox* m_ScriptNames;
        QPushButton* m_TogglePauseScript;
        QPushButton* m_KillScript;
        QPushButton* m_ExportOptions;
        QPushButton* m_JumpToAddress;
        QPushButton* m_BinarySearch;
        QPushButton* m_ViewBreakpoints;
        QLineEdit* m_FunctionSearch;
        QSortFilterProxyModel* m_FunctionFilter;
        QTableView* m_FunctionList;
        QTableView* m_Disassembly;
        QTimer* m_UpdateTimer;
	};
}