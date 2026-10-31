#pragma once
#include "decomp/output/DecompiledFunction.hpp"
#include <QWidget>

class QLabel;

namespace scrDbgApp
{
    class Decompiler;
    class Disassembler;
    class PseudocodeEditor;

    // IDA-style pseudocode view for one script, shown next to the disassembly
    // Tab: jump to the disassembly at the instruction under the cursor
    // Double click on a function name: decompile that function in place
    // Esc: back (previous function, cursor and scroll position)
    // Ctrl+Enter: forward
    // F2: toggle a breakpoint on the current line
    // F5: re-decompile (usually for newly observed indirect-call targets)
    class PseudocodeView : public QWidget
    {
        Q_OBJECT

    public:
        explicit PseudocodeView(QWidget* parent = nullptr);
        ~PseudocodeView() override;

        void SetDisassembler(Disassembler* disassembler, uint32_t scriptHash);
        bool HasScript() const;
        bool ShowAddress(uint32_t address);
        std::optional<uint32_t> GetCurrentAddress() const;
        void SetExecutionAddress(std::optional<uint32_t> address);
        void Invalidate();
        void RefreshBreakpoints();
        void FocusEditor();

        QString GetGutterText(int line) const;
        bool HasBreakpoint(int line) const;
        bool IsExecutionLine(int line) const;
        void ToggleBreakpoint(int line);

    signals:
        void AddressActivated(uint32_t address);
        void BreakpointsChanged();

    protected:
        bool eventFilter(QObject* watched, QEvent* event) override;

    private:
        struct Location
        {
            int FunctionIndex;
            int CursorPosition;
            int ScrollValue;
        };

        const Decomp::DecompiledFunction& GetFunction(int index);
        void LoadFunction(int index);
        void PlaceCursor(int line, int byteColumn);
        void FocusAddressInCurrent(uint32_t address);
        void NavigateTo(uint32_t functionAddress);
        void PushHistory();
        void RestoreLocation(const Location& location);
        void GoBack();
        void GoForward();
        void ShowContextMenu(const QPoint& pos);
        void UpdateSelections();
        void UpdateInfo();
        void ShowPlaceholder(const QString& text);

        std::pair<int, int> GetCursorLineColumn() const;
        std::pair<int, int> LineColumnAt(const QPoint& viewportPos) const;
        int ToQtColumn(int line, int byteColumn) const;
        int ToByteColumn(int line, int qtColumn) const;

        PseudocodeEditor* m_Editor;
        QLabel* m_Info;

        Disassembler* m_Disassembler;
        uint32_t m_ScriptHash;
        std::unique_ptr<Decompiler> m_Decompiler;
        std::unordered_map<int, Decomp::DecompiledFunction> m_Cache;

        int m_CurrentIndex;                               // -1, nothing shown
        const Decomp::DecompiledFunction* m_Current;      // points into m_Cache
        std::vector<Decomp::DecompiledSpan> m_FocusSpans; // spans of the instruction last navigated to
        int m_FocusLine;
        int m_ExecutionLine;
        std::optional<uint32_t> m_ExecutionAddress;
        std::map<int, std::vector<uint32_t>> m_BreakpointLines; // line to breakpoint addresses on it

        std::vector<Location> m_Back;
        std::vector<Location> m_Forward;
    };
}