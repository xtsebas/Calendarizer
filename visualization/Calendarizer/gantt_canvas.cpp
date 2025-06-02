#include "gantt_canvas.h"
#include <QPainter>
#include <QColor>
#include <QRandomGenerator>
#include <QStringList>

GanttCanvas::GanttCanvas(QWidget *parent)
    : QWidget(parent)
{
    reset();
}

void GanttCanvas::reset() {
    steps.clear();
    processColors.clear();
    update();
}

void GanttCanvas::addStep(int processIndex) {
    steps.append(processIndex);
    while (processColors.size() <= processIndex) {
        QColor color = QColor::fromHsv(QRandomGenerator::global()->bounded(360), 255, 200);
        processColors.append(color);
    }
    update();
}

void GanttCanvas::addIdleStep() {
    steps.append(-1);
    update();
}

QSize GanttCanvas::sizeHint() const {
    return QSize(800, 100);
}

QColor GanttCanvas::getColorForProcess(int index) {
    if (index < 0 || index >= processColors.size())
        return QColor(150, 150, 150); // Gris para idle
    return processColors[index];
}

// — Implementación del nuevo setter —
void GanttCanvas::setProcessLabels(const QStringList &labels) {
    processLabels = labels;
}

void GanttCanvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    int w = width();
    int h = height();
    int stepWidth = steps.size() > 0 ? w / steps.size() : w;

    for (int i = 0; i < steps.size(); ++i) {
        int x = i * stepWidth;
        int index = steps[i];
        QColor color = getColorForProcess(index);
        painter.setBrush(color);
        painter.setPen(Qt::black);
        painter.drawRect(x, 0, stepWidth, h);

        // — Aquí cambiaremos la etiqueta:
        //    Si index >= 0, usamos `processLabels[index]`; en Idle dibujamos “Idle”.
        if (index >= 0 && index < processLabels.size()) {
            painter.drawText(x + 4, h / 2, processLabels[index]);
        } else {
            painter.drawText(x + 4, h / 2, "Idle");
        }

        // Línea de tiempo (número de ciclo)
        painter.drawText(x, h - 5, QString::number(i));
    }
}
