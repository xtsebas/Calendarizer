#include "synclegend.h"
#include <QPainter>

SyncLegend::SyncLegend(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(60);
}

void SyncLegend::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    int boxSize = 20;
    int spacing = 10;
    int x = 10;
    int y = 20;

    struct { QColor color; QString label; } items[] = {
        { Qt::red, "→ Critical section" },
        { Qt::yellow, "→ Waiting" },
        { Qt::green, "→ Done" },
        { QColor(100, 149, 237), "→ Acquire" },
        { QColor(128, 0, 128), "→ Release" },
        { QColor(255, 100, 100), "→ Critical READ" },
        { QColor(200, 0, 0), "→ Critical WRITE" },

    };

    for (auto &item : items) {
        painter.setBrush(item.color);
        painter.drawRect(x, y, boxSize, boxSize);
        painter.drawText(x + boxSize + spacing, y + boxSize - 4, item.label);
        x += boxSize + spacing + 150;
    }
}

//dieguito
