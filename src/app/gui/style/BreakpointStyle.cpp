#include "BreakpointStyle.hpp"
#include <QGuiApplication>
#include <QPainter>

namespace scrDbgApp::BreakpointStyle
{
    void DrawMarker(QPainter& painter, const QRect& area, bool breakpoint, bool execution)
    {
        if (!breakpoint && !execution)
            return;

        int size = std::max(4, std::min({10, area.width() - 2, area.height() - 2}));
        QRect marker(area.left() + (area.width() - size) / 2, area.top() + (area.height() - size) / 2, size, size);

        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);

        if (breakpoint)
        {
            painter.setBrush(BREAKPOINT_MARKER);
            painter.drawEllipse(marker);
        }

        if (execution)
        {
            QPolygon arrow;
            arrow << marker.topLeft() << QPoint(marker.right() + 1, marker.top() + size / 2) << marker.bottomLeft() + QPoint(0, 1);
            painter.setBrush(EXECUTION_MARKER);
            painter.drawPolygon(arrow);
        }

        painter.restore();
    }

    const QPixmap& MarkerPixmap(int size, bool breakpoint, bool execution)
    {
        static std::map<std::tuple<int, bool, bool, qreal>, QPixmap> cache;

        const qreal dpr = qApp ? qApp->devicePixelRatio() : 1.0;
        auto key = std::make_tuple(size, breakpoint, execution, dpr);
        auto it = cache.find(key);
        if (it != cache.end())
            return it->second;

        QPixmap pixmap(QSize(size, size) * dpr);
        pixmap.setDevicePixelRatio(dpr);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        DrawMarker(painter, QRect(0, 0, size, size), breakpoint, execution);
        painter.end();

        return cache.emplace(key, pixmap).first->second;
    }
}