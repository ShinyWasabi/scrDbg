#pragma once
#include <QColor>
#include <QPixmap>

class QPainter;
class QRect;

namespace scrDbgApp::BreakpointStyle
{
    // Shared by the disassembly and pseudocode views so breakpoints look the same in both
    inline const QColor BREAKPOINT_LINE{0xFF, 0xD6, 0xD6};
    inline const QColor EXECUTION_LINE{0xC8, 0xF0, 0xC8};
    inline const QColor BREAKPOINT_MARKER{0xD0, 0x30, 0x30};
    inline const QColor EXECUTION_MARKER{0x20, 0xA0, 0x20};

    void DrawMarker(QPainter& painter, const QRect& area, bool breakpoint, bool execution);
    const QPixmap& MarkerPixmap(int size, bool breakpoint, bool execution);
}