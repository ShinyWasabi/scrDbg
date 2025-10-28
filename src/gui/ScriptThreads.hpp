#pragma once
#include <QWidget>
#include <QtCore>

class QLabel;
class QComboBox;
class QPushButton;
class QTableView;
namespace rage
{
    class scrProgram;
}

namespace scrDbg
{
    class DisassemblyModel;

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
        void OnExportDisassembly();
        void OnExportStrings();
        void OnJumpToAddress();
        void OnBinarySearch();
        void OnBreakpointsDialog();
        void OnDisassemblyContextMenu(const QPoint& pos);
        void OnCopyInstruction(const QModelIndex& index);
        void OnNopInstruction(const QModelIndex& index);
        void OnPatchInstruction(const QModelIndex& index);
        void OnGeneratePattern(const QModelIndex& index);
        void OnViewXrefs(const QModelIndex& index);
        void OnJumpToInstructionAddress(const QModelIndex& index);
        void OnSetBreakpoint(const QModelIndex& index, bool set);

	private:
        uint32_t GetCurrentScriptHash();
        DisassemblyModel* GetDisassembly();
        bool ScrollToAddress(uint32_t address);
        void UpdateCurrentScript();

        uint32_t m_LastScriptHash;

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
        QPushButton* m_ExportDisassembly;
        QPushButton* m_ExportStrings;
        QPushButton* m_JumpToAddress;
        QPushButton* m_BinarySearch;
        QPushButton* m_ViewBreakpoints;
        QTableView* m_Disassembly;

        QTimer* m_UpdateTimer;
	};
}