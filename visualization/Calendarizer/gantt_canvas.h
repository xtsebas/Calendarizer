#ifndef GANTT_CANVAS_H
#define GANTT_CANVAS_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QColor>

class GanttCanvas : public QWidget {
    Q_OBJECT

public:
    explicit GanttCanvas(QWidget *parent = nullptr);

    void reset();
    void addStep(int processIndex);
    void addIdleStep();

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    QVector<int> steps;  // índice del proceso (o -1 para IDLE)
    QVector<QColor> processColors;

    QColor getColorForProcess(int index);
};

#endif // GANTT_CANVAS_H
