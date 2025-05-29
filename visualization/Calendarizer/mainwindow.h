#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialog>
#include <QTableWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include "Priority/priority_scheduler.h"
#include "gantt_canvas.h"
#include "synchronizer.h"
#include "sync_canvas.h"
#include "synclegend.h"
#include <QPlainTextEdit>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showManualInputModal();
    void goToSimulationScreen();
    void goToMainMenu();
    void loadProcessesFromFile();
    void goToSyncSimulationScreen();

private:
    std::vector<std::string> selectedAlgorithms;

    QTimer *priorityTimer = nullptr;
    PriorityScheduler *priorityScheduler = nullptr;
    int priorityTick;
    std::vector<Process> priorityReadyQueue;
    QTableWidget *priorityProcessTable;
    GanttCanvas *priorityCanvas;
    QLabel *priorityStatusLabel;
    void startPrioritySimulation();


    QStackedWidget *stackedWidget;
    QWidget *mainMenuWidget;
    QWidget *simulationWidget;
    QComboBox *algorithmSelector;
    std::vector<Process> loadedProcesses;
    QGroupBox *multiAlgorithmGroup;
    QCheckBox *fifoCheck;
    QCheckBox *sjfCheck;
    QCheckBox *srtfCheck;
    QCheckBox *rrCheck;
    QCheckBox *priorityCheck;
    QWidget *syncSimulationWidget;

    QPlainTextEdit *ganttOutput;
    QComboBox *syncAlgorithmSelector;
    QLabel *syncStatusLabel;
    QTimer *syncTimer;
    int syncTick;

    SyncCanvas *syncCanvas;
    Synchronizer *currentSync;

    QTableWidget *syncProcessTable;
    SyncLegend *syncLegend;

    QWidget* createMainMenu();
    QWidget* createSimulationScreen();
    QWidget* createSyncSimulationScreen();
    void showModal(QWidget *content, const QString &title);
    QTableWidget *processTable;

    bool startedLog[2];
    bool finishedLog[2];

    QPlainTextEdit *syncLog;
    void logSyncMessage(const QString &message);
    std::vector<Process> syncProcesses;
    void startSyncSimulation();
    void updateSyncSimulation();
};

#endif // MAINWINDOW_H
