#ifndef GANTT_CANVAS_H
#define GANTT_CANVAS_H

#include <QStringList>
#include <QWidget>
#include <QVector>
#include <QString>
#include <QColor>

// GanttCanvas: pinta un diagrama de Gantt ciclo a ciclo.
// Ahora almacena un QStringList de etiquetas (PIDs) y un ancho fijo por paso.
class GanttCanvas : public QWidget {
    Q_OBJECT

public:
    explicit GanttCanvas(QWidget *parent = nullptr);

    // Limpia todos los pasos y vuelve el ancho mínimo a 0
    void reset();

    // Añade un paso en el que el proceso `processIndex` se ejecuta
    void addStep(int processIndex);

    // Añade un paso idle (sin proceso)
    void addIdleStep();

    // Recibe la lista de PIDs (etiquetas) que se mostrarán en cada bloque
    void setProcessLabels(const QStringList &labels);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    QVector<int> steps;         // ÍNDICE del proceso en cada paso (o -1 para Idle)
    QVector<QColor> processColors;
    QStringList processLabels;  // Lista de PIDs (ordenada según `loadedProcesses`)

    // Ancho en píxeles que ocupará cada paso en el Gantt
    static constexpr int kStepWidth = 50;

    QColor getColorForProcess(int index) const;
};

#endif // GANTT_CANVAS_H
