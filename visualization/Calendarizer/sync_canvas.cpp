#include "sync_canvas.h"
#include <QPainter>
#include <QPen>
#include <QBrush>

SyncCanvas::SyncCanvas(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(300);
}

void SyncCanvas::reset() {
    steps.clear();
    tickPerPid.clear(); // <- resetear ticks por PID
    update();
}

void SyncCanvas::setMaxTicks(int ticks) {
    maxTicks = ticks;
}

void SyncCanvas::setProcesses(const std::vector<Process> &procs) {
    processes = procs;
}

void SyncCanvas::addStep(int pid, int tick, SyncStep::State state, const std::string& actionType) {
    steps.push_back({pid, tick, state, actionType});
    update();
}

void SyncCanvas::paintEvent(QPaintEvent *) {
    if (processes.empty()) return;

    QPainter painter(this);
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    const int marginLeft = 50; // espacio para PIDs
    const int rowHeight = height() / processes.size();
    const int colWidth = (width() - marginLeft) / maxTicks;

    // Dibujar etiquetas de tiempo en eje X
    for (int tick = 0; tick < maxTicks; ++tick) {
        int x = marginLeft + tick * colWidth;
        painter.drawText(x + 2, 10, QString::number(tick));
    }

    // Dibujar estado de cada proceso
    for (int i = 0; i < steps.size(); ++i) {
        const SyncStep &s = steps[i];
        int row = s.pid;
        int col = s.tick;

        QRect rect(marginLeft + col * colWidth, row * rowHeight + 20, colWidth - 2, rowHeight - 2);

        switch (s.state) {
            case SyncStep::Waiting:
                painter.fillRect(rect, Qt::yellow);
                break;
            case SyncStep::Critical: {
                QString type = QString::fromStdString(s.actionType).trimmed().toUpper();
                if (type == "READ")
                    painter.fillRect(rect, QColor(255, 100, 100));  // rojo claro
                else if (type == "WRITE")
                    painter.fillRect(rect, QColor(200, 0, 0));      // rojo oscuro
                else
                    painter.fillRect(rect, Qt::red);
                break;
            }
            case SyncStep::Finished:
                painter.fillRect(rect, Qt::green);
                break;
            // Para mutex/Peterson:
            case SyncStep::Acquire:
                painter.fillRect(rect, QColor(100, 149, 237)); // azul claro
                break;
            case SyncStep::Release:
                painter.fillRect(rect, QColor(128, 0, 128));   // púrpura
                break;
            // Para semáforo:
            case SyncStep::Wait:
                painter.fillRect(rect, QColor(100, 149, 237)); // azul claro (igual que Acquire)
                break;
            case SyncStep::Signal:
                painter.fillRect(rect, QColor(128, 0, 128));   // púrpura (igual que Release)
                break;
        }

        painter.drawRect(rect);
    }

    // Dibujar etiquetas PID en eje Y
    for (size_t i = 0; i < processes.size(); ++i) {
        QString pidLabel = "PID: " + QString::fromStdString(processes[i].pid);
        painter.drawText(5, i * rowHeight + rowHeight / 2 + 10, pidLabel);
    }

    if (!steps.empty()) {
        setMinimumWidth(steps.back().tick * 20 + 200);
    }

}


//dieguito
