#ifndef SYNC_CANVAS_H
#define SYNC_CANVAS_H

#include <QWidget>
#include <vector>
#include <QString>
#include <QColor>
#include <unordered_map>

struct SyncStep {
    enum State { Waiting, Critical, Finished, Acquire, Release };
    int pid;
    int tick;
    State state;
    std::string actionType;  // "READ" o "WRITE"
};

class SyncCanvas : public QWidget {
    Q_OBJECT

public:
    SyncCanvas(QWidget *parent = nullptr);
    void addStep(int pid, int tick, SyncStep::State state, const std::string& actionType = "");
    void setMaxTicks(int ticks);
    void reset();
    void setProcesses(const std::vector<Process> &procs);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<Process> processes;
    std::vector<SyncStep> steps;
    int maxTicks = 100;
    std::unordered_map<int, int> tickPerPid;
};

#endif // SYNC_CANVAS_H
//dieguito
