#include "gantt_canvas.h"
#include <QPainter>
#include <QColor>
#include <QRandomGenerator>
#include <QTextOption>

GanttCanvas::GanttCanvas(QWidget *parent)
    : QWidget(parent)
{
    reset();
}

void GanttCanvas::reset() {
    steps.clear();
    processColors.clear();
    // Cuando reseteamos, forzamos el ancho a 0 para que vuelva a “compacto”
    setMinimumWidth(0);
    update();
}

void GanttCanvas::addStep(int processIndex) {
    steps.append(processIndex);
    // Si es un índice nuevo, asignar color aleatorio
    while (processColors.size() <= processIndex) {
        QColor color = QColor::fromHsv(QRandomGenerator::global()->bounded(360), 255, 200);
        processColors.append(color);
    }
    // Ajustar mínimo ancho del widget: (#pasos × ancho por paso)
    setMinimumWidth(steps.size() * kStepWidth);
    update();
}

void GanttCanvas::addIdleStep() {
    steps.append(-1);
    setMinimumWidth(steps.size() * kStepWidth);
    update();
}

QSize GanttCanvas::sizeHint() const {
    // Altura fija de 100 px; ancho = max(800, pasos × kStepWidth)
    return QSize(qMax(steps.size() * kStepWidth, 800), 100);
}

QColor GanttCanvas::getColorForProcess(int index) const {
    if (index < 0 || index >= processColors.size())
        return QColor(150, 150, 150); // Gris para Idle
    return processColors[index];
}

void GanttCanvas::setProcessLabels(const QStringList &labels) {
    processLabels = labels;
}

void GanttCanvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    int h = height();
    int stepWidth = kStepWidth;

    for (int i = 0; i < steps.size(); ++i) {
        int x = i * stepWidth;
        int index = steps[i];
        QColor color = getColorForProcess(index);

        // Dibujar el rectángulo del proceso (o Idle)
        painter.setBrush(color);
        painter.setPen(Qt::black);
        painter.drawRect(x, 0, stepWidth, h);

        // Etiqueta: PID real o "Idle"
        if (index >= 0 && index < processLabels.size()) {
            QTextOption opt;
            opt.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            painter.drawText(QRectF(x + 2, 0, stepWidth - 4, h),
                             processLabels[index], opt);
        } else {
            painter.drawText(x + 4, h / 2, "Idle");
        }

        // Número de ciclo en la parte inferior
        painter.drawText(x, h - 5, QString::number(i));
    }
}
