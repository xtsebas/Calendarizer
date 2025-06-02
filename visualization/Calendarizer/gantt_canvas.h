#ifndef GANTT_CANVAS_H
#define GANTT_CANVAS_H

#include <QStringList>
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
    void setProcessLabels(const QStringList &labels);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    QVector<int> steps;  // índice del proceso (o -1 para IDLE)
    QVector<QColor> processColors;

    QStringList processLabels;

    QColor getColorForProcess(int index);
};

#endif // GANTT_CANVAS_H
