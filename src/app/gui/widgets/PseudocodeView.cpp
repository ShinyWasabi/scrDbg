#include "PseudocodeView.hpp"
#include "decomp/Decompiler.hpp"
#include "gui/style/BreakpointStyle.hpp"
#include "pipe/PipeCommands.hpp"
#include <QApplication>
#include <QClipboard>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QVBoxLayout>

namespace scrDbgApp
{
    const QColor COLOR_CURRENT_LINE(0xEE, 0xF4, 0xFF);
    const QColor COLOR_FOCUS_LINE(0xFF, 0xF6, 0xD5);
    const QColor COLOR_FOCUS_SPAN(0xFF, 0xE0, 0x8A);
    const QColor COLOR_GUTTER(0xF3, 0xF3, 0xF3);
    const QColor COLOR_GUTTER_TEXT(0x80, 0x80, 0x80);

    class DecompilerHighlighter : public QSyntaxHighlighter
    {
    public:
        explicit DecompilerHighlighter(QTextDocument* parent)
            : QSyntaxHighlighter(parent)
        {
            auto format = [](const char* color, bool bold = false, bool italic = false) {
                QTextCharFormat f;
                f.setForeground(QColor(color));
                if (bold)
                    f.setFontWeight(QFont::Bold);
                f.setFontItalic(italic);
                return f;
            };

            m_Rules = {
                {QRegularExpression(R"(\b[A-Za-z_][A-Za-z0-9_]*(?=\())"), format("#795E26")},              // function calls
                {QRegularExpression(R"(\b(?:unk_0x[0-9A-F]+|[A-Z][A-Z0-9_]+)(?=\())"), format("#6F008A")}, // native calls
                {QRegularExpression(R"(\b(?:Global|Static)_\d+\b)"), format("#A0522D")},
                {QRegularExpression(R"(\b(?:Local|Arg|FrameSlot)_\d+\b)"), format("#001080")},
                {QRegularExpression(R"(\bf_(?:\d+)?\b)"), format("#267F99")},
                {QRegularExpression(R"(\b(?:var|void|if|else|while|switch|case|default|break|return|goto|joaat)\b)"), format("#0000FF", true)},
                {QRegularExpression(R"((?<![\w.])\d+(?:\.\d+)?(?:e[+-]?\d+)?f?\b)"), format("#098658")},
                {QRegularExpression(R"(^loc_[0-9A-F]+:)"), format("#AF00DB", true)},
                {QRegularExpression(R"(\bloc_[0-9A-F]+\b)"), format("#AF00DB")},
                {QRegularExpression(R"("(?:[^"\\]|\\.)*")"), format("#A31515")},
                {QRegularExpression(R"(/\*.*?\*/)"), format("#008000", false, true)},
                {QRegularExpression(R"(//[^\n]*)"), format("#008000", false, true)},
            };

            m_CommentStart = QRegularExpression(R"(/\*)");
            m_CommentEnd = QRegularExpression(R"(\*/)");
            m_CommentFormat = format("#008000", false, true);
        }

    protected:
        void highlightBlock(const QString& text) override
        {
            for (const auto& rule : m_Rules)
            {
                auto it = rule.Pattern.globalMatch(text);
                while (it.hasNext())
                {
                    auto match = it.next();
                    setFormat(match.capturedStart(), match.capturedLength(), rule.Format);
                }
            }

            // Multi-line /* */ blocks like script header
            setCurrentBlockState(0);
            int start = 0;
            if (previousBlockState() != 1)
            {
                auto match = m_CommentStart.match(text);
                start = match.hasMatch() ? match.capturedStart() : -1;
            }

            while (start >= 0)
            {
                auto end = m_CommentEnd.match(text, start);
                int length;
                if (!end.hasMatch())
                {
                    setCurrentBlockState(1);
                    length = text.length() - start;
                }
                else
                {
                    length = end.capturedEnd() - start;
                }

                setFormat(start, length, m_CommentFormat);

                auto next = m_CommentStart.match(text, start + length);
                start = next.hasMatch() ? next.capturedStart() : -1;
            }
        }

    private:
        struct Rule
        {
            QRegularExpression Pattern;
            QTextCharFormat Format;
        };

        std::vector<Rule> m_Rules;
        QRegularExpression m_CommentStart;
        QRegularExpression m_CommentEnd;
        QTextCharFormat m_CommentFormat;
    };

    class PseudocodeEditor : public QPlainTextEdit
    {
    public:
        explicit PseudocodeEditor(PseudocodeView* view);

        int GetGutterWidth() const;
        void PaintGutter(QPaintEvent* event);
        void GutterPressed(QMouseEvent* event);

        QWidget* GetGutter() const
        {
            return m_Gutter;
        }

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        class Gutter : public QWidget
        {
        public:
            explicit Gutter(PseudocodeEditor* editor)
                : QWidget(editor),
                  m_Editor(editor)
            {
                setCursor(Qt::PointingHandCursor);
                setToolTip("Click to toggle a breakpoint (F2).");
            }

            QSize sizeHint() const override
            {
                return QSize(m_Editor->GetGutterWidth(), 0);
            }

        protected:
            void paintEvent(QPaintEvent* event) override
            {
                m_Editor->PaintGutter(event);
            }

            void mousePressEvent(QMouseEvent* event) override
            {
                m_Editor->GutterPressed(event);
            }

        private:
            PseudocodeEditor* m_Editor;
        };

        void UpdateGutterWidth();

        PseudocodeView* m_View;
        Gutter* m_Gutter;
    };

    PseudocodeEditor::PseudocodeEditor(PseudocodeView* view)
        : QPlainTextEdit(view),
          m_View(view),
          m_Gutter(new Gutter(this))
    {
        connect(this, &QPlainTextEdit::blockCountChanged, this, [this](int) {
            UpdateGutterWidth();
        });
        connect(this, &QPlainTextEdit::updateRequest, this, [this](const QRect& rect, int dy) {
            if (dy)
                m_Gutter->scroll(0, dy);
            else
                m_Gutter->update(0, rect.y(), m_Gutter->width(), rect.height());
        });

        UpdateGutterWidth();
    }

    int PseudocodeEditor::GetGutterWidth() const
    {
        return 20 + fontMetrics().horizontalAdvance(QLatin1Char('0')) * 8 + 6;
    }

    void PseudocodeEditor::UpdateGutterWidth()
    {
        setViewportMargins(GetGutterWidth(), 0, 0, 0);
    }

    void PseudocodeEditor::resizeEvent(QResizeEvent* event)
    {
        QPlainTextEdit::resizeEvent(event);

        QRect rect = contentsRect();
        m_Gutter->setGeometry(QRect(rect.left(), rect.top(), GetGutterWidth(), rect.height()));
    }

    void PseudocodeEditor::PaintGutter(QPaintEvent* event)
    {
        QPainter painter(m_Gutter);
        painter.fillRect(event->rect(), COLOR_GUTTER);
        painter.setFont(font());

        const int lineHeight = fontMetrics().height();

        QTextBlock block = firstVisibleBlock();
        int line = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());

        while (block.isValid() && top <= event->rect().bottom())
        {
            if (block.isVisible() && bottom >= event->rect().top())
            {
                BreakpointStyle::DrawMarker(painter, QRect(2, top, 16, lineHeight), m_View->HasBreakpoint(line), m_View->IsExecutionLine(line));

                QString text = m_View->GetGutterText(line);
                if (!text.isEmpty())
                {
                    painter.setPen(COLOR_GUTTER_TEXT);
                    painter.drawText(18, top, m_Gutter->width() - 22, lineHeight, Qt::AlignRight | Qt::AlignVCenter, text);
                }
            }

            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            line++;
        }
    }

    void PseudocodeEditor::GutterPressed(QMouseEvent* event)
    {
        if (event->button() != Qt::LeftButton)
            return;

        QTextCursor cursor = cursorForPosition(QPoint(0, event->position().toPoint().y()));
        m_View->ToggleBreakpoint(cursor.blockNumber());
    }

    PseudocodeView::PseudocodeView(QWidget* parent)
        : QWidget(parent),
          m_Disassembler(nullptr),
          m_ScriptHash(0),
          m_CurrentIndex(-1),
          m_Current(nullptr),
          m_FocusLine(-1),
          m_ExecutionLine(-1)
    {
        QFont font("Consolas", 11);
        font.setStyleHint(QFont::Monospace);

        m_Editor = new PseudocodeEditor(this);
        m_Editor->setFont(font);
        m_Editor->setReadOnly(true);
        m_Editor->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        m_Editor->setLineWrapMode(QPlainTextEdit::NoWrap);
        m_Editor->setTabStopDistance(QFontMetricsF(font).horizontalAdvance(' ') * 4);
        m_Editor->setContextMenuPolicy(Qt::CustomContextMenu);
        new DecompilerHighlighter(m_Editor->document());

        m_Editor->installEventFilter(this);
        m_Editor->viewport()->installEventFilter(this);

        connect(m_Editor, &QWidget::customContextMenuRequested, this, &PseudocodeView::ShowContextMenu);
        connect(m_Editor, &QPlainTextEdit::cursorPositionChanged, this, [this]() {
            if (m_FocusLine >= 0 && m_Editor->textCursor().blockNumber() != m_FocusLine)
            {
                m_FocusSpans.clear();
                m_FocusLine = -1;
            }

            UpdateSelections();
            UpdateInfo();
        });

        m_Info = new QLabel(this);
        m_Info->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        m_Info->setStyleSheet("font-weight: bold; padding: 2px;");

        auto layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_Editor);
        layout->addWidget(m_Info);

        ShowPlaceholder("// Nothing to show.");
    }

    PseudocodeView::~PseudocodeView() = default;

    void PseudocodeView::SetDisassembler(Disassembler* disassembler, uint32_t scriptHash)
    {
        if (!disassembler && !m_Disassembler) // the owner clears on every update tick while no script is alive
            return;

        m_Back.clear();
        m_Forward.clear();
        m_Cache.clear();
        m_Current = nullptr;
        m_CurrentIndex = -1;
        m_BreakpointLines.clear();
        m_ExecutionLine = -1;

        m_Disassembler = disassembler;
        m_ScriptHash = scriptHash;
        m_Decompiler = disassembler ? g_Game->CreateDecompiler(*disassembler) : nullptr;

        if (!disassembler)
            ShowPlaceholder("// Nothing to show.");
        else if (!m_Decompiler)
            ShowPlaceholder("// Decompilation is not supported for this game yet.");
        else
            ShowPlaceholder("// Select an instruction in the disassembly and press Tab,\n// or right-click it and choose Decompile Function.");
    }

    bool PseudocodeView::IsSupported() const
    {
        return m_Decompiler != nullptr;
    }

    void PseudocodeView::ShowPlaceholder(const QString& text)
    {
        m_FocusSpans.clear();
        m_FocusLine = -1;
        m_Editor->setPlainText(text);
        UpdateSelections();
        UpdateInfo();
        m_Editor->GetGutter()->update();
    }

    void PseudocodeView::FocusEditor()
    {
        m_Editor->setFocus(Qt::OtherFocusReason);
    }

    const Decomp::DecompiledFunction& PseudocodeView::GetFunction(int index)
    {
        auto it = m_Cache.find(index);
        if (it != m_Cache.end())
            return it->second;

        QApplication::setOverrideCursor(Qt::WaitCursor);
        auto decompiled = m_Decompiler->DecompileFunction(index);
        QApplication::restoreOverrideCursor();

        return m_Cache.emplace(index, std::move(decompiled)).first->second;
    }

    void PseudocodeView::LoadFunction(int index)
    {
        m_Current = &GetFunction(index);
        m_CurrentIndex = index;
        m_FocusSpans.clear();
        m_FocusLine = -1;

        m_Editor->setPlainText(QString::fromStdString(m_Current->ToText()));

        m_ExecutionLine = -1;
        if (m_ExecutionAddress && *m_ExecutionAddress >= m_Current->Start && *m_ExecutionAddress < m_Current->End)
            m_ExecutionLine = m_Current->FindLine(*m_ExecutionAddress);

        RefreshBreakpoints();
        UpdateSelections();
        UpdateInfo();
    }

    int PseudocodeView::ToQtColumn(int line, int byteColumn) const
    {
        if (!m_Current || line < 0 || line >= static_cast<int>(m_Current->Lines.size()))
            return 0;

        const std::string& text = m_Current->Lines[line];
        byteColumn = std::clamp(byteColumn, 0, static_cast<int>(text.size()));
        return static_cast<int>(QString::fromUtf8(text.data(), byteColumn).size());
    }

    int PseudocodeView::ToByteColumn(int line, int qtColumn) const
    {
        if (!m_Current || line < 0 || line >= static_cast<int>(m_Current->Lines.size()))
            return 0;

        QString text = QString::fromStdString(m_Current->Lines[line]);
        return static_cast<int>(text.left(qtColumn).toUtf8().size());
    }

    std::pair<int, int> PseudocodeView::GetCursorLineColumn() const
    {
        QTextCursor cursor = m_Editor->textCursor();
        int line = cursor.blockNumber();
        return {line, ToByteColumn(line, cursor.positionInBlock())};
    }

    std::pair<int, int> PseudocodeView::LineColumnAt(const QPoint& viewportPos) const
    {
        QTextCursor cursor = m_Editor->cursorForPosition(viewportPos);
        int line = cursor.blockNumber();
        int column = cursor.positionInBlock();

        QRect caret = m_Editor->cursorRect(cursor);
        if (column > 0 && viewportPos.x() < caret.left())
            column--;

        return {line, ToByteColumn(line, column)};
    }

    void PseudocodeView::PlaceCursor(int line, int byteColumn)
    {
        QTextBlock block = m_Editor->document()->findBlockByNumber(line);
        if (!block.isValid())
            return;

        QTextCursor cursor(block);
        cursor.setPosition(block.position() + ToQtColumn(line, byteColumn));
        m_Editor->setTextCursor(cursor);
    }

    void PseudocodeView::FocusAddressInCurrent(uint32_t address)
    {
        int line = m_Current->FindLine(address);
        if (line < 0)
            return;

        m_FocusSpans.clear();
        for (const auto& span : m_Current->FindSpans(address))
        {
            if (span.Line == line)
                m_FocusSpans.push_back(span);
        }

        int column = 0;
        if (!m_FocusSpans.empty())
        {
            column = m_FocusSpans.front().Start;
        }
        else
        {
            const std::string& text = m_Current->Lines[line];
            while (column < static_cast<int>(text.size()) && (text[column] == '\t' || text[column] == ' '))
                column++;
        }

        m_FocusLine = line;
        PlaceCursor(line, column);
        m_Editor->centerCursor();
        UpdateSelections();
    }

    bool PseudocodeView::ShowAddress(uint32_t address)
    {
        if (!m_Decompiler)
            return false;

        int index = m_Decompiler->GetFunctionIndexForPc(address);
        if (index < 0)
            return false;

        if (index != m_CurrentIndex)
        {
            if (m_CurrentIndex >= 0)
            {
                PushHistory();
                m_Forward.clear();
            }

            LoadFunction(index);
        }

        FocusAddressInCurrent(address);
        return true;
    }

    std::optional<uint32_t> PseudocodeView::GetCurrentAddress() const
    {
        if (!m_Current)
            return std::nullopt;

        auto [line, column] = GetCursorLineColumn();
        if (auto address = m_Current->GetAddressAt(line, column))
            return address;

        const int count = static_cast<int>(m_Current->Lines.size()); // braces, else, declarations use the nearest line that has code
        for (int distance = 1; distance < count; distance++)
        {
            if (auto address = m_Current->GetLineAddress(line + distance))
                return address;
            if (auto address = m_Current->GetLineAddress(line - distance))
                return address;
        }

        return std::nullopt;
    }

    void PseudocodeView::NavigateTo(uint32_t functionAddress)
    {
        if (!m_Decompiler)
            return;

        int index = m_Decompiler->GetFunctionIndexForPc(functionAddress);
        if (index < 0)
            return;

        PushHistory();
        m_Forward.clear();

        if (index != m_CurrentIndex)
            LoadFunction(index);

        m_FocusLine = m_Current->FindLine(functionAddress);
        m_FocusSpans.clear();
        PlaceCursor(std::max(m_FocusLine, 0), 0);
        m_Editor->verticalScrollBar()->setValue(0);
        UpdateSelections();
    }

    void PseudocodeView::PushHistory()
    {
        if (m_CurrentIndex < 0)
            return;

        m_Back.push_back({m_CurrentIndex, m_Editor->textCursor().position(), m_Editor->verticalScrollBar()->value()});
        if (m_Back.size() > 128) // max history
            m_Back.erase(m_Back.begin());
    }

    void PseudocodeView::RestoreLocation(const Location& location)
    {
        if (!m_Decompiler || location.FunctionIndex < 0 || location.FunctionIndex >= m_Decompiler->GetFunctionCount())
            return;

        if (location.FunctionIndex != m_CurrentIndex)
            LoadFunction(location.FunctionIndex);

        m_FocusSpans.clear();
        m_FocusLine = -1;

        QTextCursor cursor = m_Editor->textCursor();
        cursor.setPosition(std::clamp(location.CursorPosition, 0, m_Editor->document()->characterCount() - 1));
        m_Editor->setTextCursor(cursor);
        m_Editor->verticalScrollBar()->setValue(location.ScrollValue);
        UpdateSelections();
    }

    void PseudocodeView::GoBack()
    {
        if (m_Back.empty() || m_CurrentIndex < 0)
            return;

        m_Forward.push_back({m_CurrentIndex, m_Editor->textCursor().position(), m_Editor->verticalScrollBar()->value()});
        Location location = m_Back.back();
        m_Back.pop_back();
        RestoreLocation(location);
    }

    void PseudocodeView::GoForward()
    {
        if (m_Forward.empty() || m_CurrentIndex < 0)
            return;

        m_Back.push_back({m_CurrentIndex, m_Editor->textCursor().position(), m_Editor->verticalScrollBar()->value()});
        Location location = m_Forward.back();
        m_Forward.pop_back();
        RestoreLocation(location);
    }

    void PseudocodeView::Invalidate()
    {
        if (!m_Disassembler)
            return;

        int index = m_CurrentIndex;
        int line = m_Editor->textCursor().blockNumber();
        int scroll = m_Editor->verticalScrollBar()->value();

        // The disassembler's function table may have changed with the code
        m_Cache.clear();
        m_Current = nullptr;
        m_CurrentIndex = -1;
        m_Decompiler = g_Game->CreateDecompiler(*m_Disassembler);

        if (!m_Decompiler || index < 0 || index >= m_Decompiler->GetFunctionCount())
        {
            SetDisassembler(m_Disassembler, m_ScriptHash);
            return;
        }

        LoadFunction(index);
        PlaceCursor(std::min(line, static_cast<int>(m_Current->Lines.size()) - 1), 0);
        m_Editor->verticalScrollBar()->setValue(scroll);
    }

    void PseudocodeView::RefreshBreakpoints()
    {
        m_BreakpointLines.clear();

        if (m_Current)
        {
            for (const auto& [script, pc] : PipeCommands::GetAllBreakpoints())
            {
                if (script != m_ScriptHash || pc < m_Current->Start || pc >= m_Current->End)
                    continue;

                int line = m_Current->FindLine(pc);
                if (line >= 0)
                    m_BreakpointLines[line].push_back(pc);
            }
        }

        UpdateSelections();
        m_Editor->GetGutter()->update();
    }

    void PseudocodeView::ToggleBreakpoint(int line)
    {
        if (!m_Current)
            return;

        // A line can hold breakpoints set from the disassembly on any of its instructions
        auto it = m_BreakpointLines.find(line);
        if (it != m_BreakpointLines.end())
        {
            for (uint32_t pc : it->second)
                PipeCommands::SetBreakpoint(m_ScriptHash, pc, false);
        }
        else if (auto address = m_Current->GetLineAddress(line))
        {
            PipeCommands::SetBreakpoint(m_ScriptHash, *address, true);
        }
        else
        {
            return; // braces, declarations, else - no code to break on
        }

        RefreshBreakpoints();
        emit BreakpointsChanged();
    }

    bool PseudocodeView::HasBreakpoint(int line) const
    {
        return m_BreakpointLines.count(line) != 0;
    }

    bool PseudocodeView::IsExecutionLine(int line) const
    {
        return line >= 0 && line == m_ExecutionLine;
    }

    QString PseudocodeView::GetGutterText(int line) const
    {
        if (!m_Current)
            return {};

        if (auto address = m_Current->GetLineAddress(line))
            return QString::number(*address, 16).toUpper();

        return {};
    }

    void PseudocodeView::SetExecutionAddress(std::optional<uint32_t> address)
    {
        const bool changed = address != m_ExecutionAddress;
        m_ExecutionAddress = address;

        if (changed && address && isVisible() && m_Decompiler) // follow a newly hit breakpoint
            ShowAddress(*address);

        int line = -1;
        if (m_Current && address && *address >= m_Current->Start && *address < m_Current->End)
            line = m_Current->FindLine(*address);

        if (changed)
            RefreshBreakpoints();

        if (line != m_ExecutionLine)
        {
            m_ExecutionLine = line;
            UpdateSelections();
            m_Editor->GetGutter()->update();
        }
    }

    void PseudocodeView::UpdateSelections()
    {
        QList<QTextEdit::ExtraSelection> selections;
        QTextDocument* document = m_Editor->document();

        auto addLine = [&](int line, const QColor& color) {
            QTextBlock block = document->findBlockByNumber(line);
            if (line < 0 || !block.isValid())
                return;

            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(color);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = QTextCursor(block);
            selection.cursor.clearSelection();
            selections.append(selection);
        };

        if (m_Current)
            addLine(m_Editor->textCursor().blockNumber(), COLOR_CURRENT_LINE);
        addLine(m_FocusLine, COLOR_FOCUS_LINE);
        for (const auto& [line, addresses] : m_BreakpointLines)
            addLine(line, BreakpointStyle::BREAKPOINT_LINE);
        addLine(m_ExecutionLine, BreakpointStyle::EXECUTION_LINE);

        for (const auto& span : m_FocusSpans)
        {
            QTextBlock block = document->findBlockByNumber(span.Line);
            if (!block.isValid())
                continue;

            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(COLOR_FOCUS_SPAN);
            selection.cursor = QTextCursor(block);
            selection.cursor.setPosition(block.position() + ToQtColumn(span.Line, span.Start));
            selection.cursor.setPosition(block.position() + ToQtColumn(span.Line, span.End), QTextCursor::KeepAnchor);
            selections.append(selection);
        }

        m_Editor->setExtraSelections(selections);
    }

    void PseudocodeView::UpdateInfo()
    {
        if (!m_Current)
        {
            m_Info->setText("- <none>");
            return;
        }

        QString name = QString::fromStdString(m_Current->Name);
        auto address = GetCurrentAddress();
        if (!address)
        {
            m_Info->setText(name);
            return;
        }

        m_Info->setText(QString("%1+0x%2 | 0x%3").arg(name, QString::number(*address - m_Current->Start, 16).toUpper(), QString::number(*address, 16).toUpper()));
    }

    void PseudocodeView::ShowContextMenu(const QPoint& pos)
    {
        if (!m_Editor->textCursor().hasSelection())
            m_Editor->setTextCursor(m_Editor->cursorForPosition(pos));

        auto [line, column] = LineColumnAt(pos);
        std::optional<uint32_t> link = m_Current ? m_Current->FindLinkAt(line, column) : std::nullopt;

        QMenu menu(this);

        QAction* jump = menu.addAction("Jump to Disassembly\tTab");
        jump->setEnabled(m_Current != nullptr);
        connect(jump, &QAction::triggered, this, [this]() {
            if (auto address = GetCurrentAddress())
                emit AddressActivated(*address);
        });

        if (link)
        {
            QAction* go = menu.addAction("Go to Function\tEnter");
            connect(go, &QAction::triggered, this, [this, target = *link]() {
                NavigateTo(target);
            });
        }

        const bool canBreak = m_Current && (HasBreakpoint(line) || m_Current->GetLineAddress(line));
        QAction* breakpoint = menu.addAction(HasBreakpoint(line) ? "Remove Breakpoint\tF2" : "Set Breakpoint\tF2");
        breakpoint->setEnabled(canBreak);
        connect(breakpoint, &QAction::triggered, this, [this, line]() {
            ToggleBreakpoint(line);
        });

        menu.addSeparator();

        QAction* back = menu.addAction("Back\tEsc");
        back->setEnabled(!m_Back.empty());
        connect(back, &QAction::triggered, this, &PseudocodeView::GoBack);

        QAction* forward = menu.addAction("Forward\tCtrl+Enter");
        forward->setEnabled(!m_Forward.empty());
        connect(forward, &QAction::triggered, this, &PseudocodeView::GoForward);

        menu.addSeparator();

        QAction* copy = menu.addAction("Copy");
        copy->setEnabled(m_Editor->textCursor().hasSelection());
        connect(copy, &QAction::triggered, m_Editor, &QPlainTextEdit::copy);

        QAction* copyAll = menu.addAction("Copy Function");
        copyAll->setEnabled(m_Current != nullptr);
        connect(copyAll, &QAction::triggered, this, [this]() {
            QGuiApplication::clipboard()->setText(m_Editor->toPlainText());
        });

        QAction* refresh = menu.addAction("Refresh\tF5");
        refresh->setEnabled(m_Current != nullptr);
        connect(refresh, &QAction::triggered, this, &PseudocodeView::Invalidate);

        menu.exec(m_Editor->viewport()->mapToGlobal(pos));
    }

    bool PseudocodeView::eventFilter(QObject* watched, QEvent* event)
    {
        if (watched == m_Editor && event->type() == QEvent::KeyPress)
        {
            auto key = static_cast<QKeyEvent*>(event);
            switch (key->key())
            {
            case Qt::Key_Tab:
                if (auto address = GetCurrentAddress())
                    emit AddressActivated(*address);
                return true;
            case Qt::Key_Escape:
                GoBack();
                return true;
            case Qt::Key_Return:
            case Qt::Key_Enter:
                if (key->modifiers() & Qt::ControlModifier)
                {
                    GoForward();
                }
                else if (m_Current)
                {
                    auto [line, column] = GetCursorLineColumn();
                    if (auto target = m_Current->FindLinkAt(line, column))
                        NavigateTo(*target);
                }
                return true;
            case Qt::Key_F2:
                ToggleBreakpoint(m_Editor->textCursor().blockNumber());
                return true;
            case Qt::Key_F5:
                Invalidate();
                return true;
            default:
                break;
            }
        }

        if (watched == m_Editor->viewport() && event->type() == QEvent::MouseButtonDblClick && m_Current)
        {
            auto mouse = static_cast<QMouseEvent*>(event);
            auto [line, column] = LineColumnAt(mouse->position().toPoint());
            if (auto target = m_Current->FindLinkAt(line, column))
            {
                NavigateTo(*target);
                return true;
            }
        }

        return QWidget::eventFilter(watched, event);
    }
}