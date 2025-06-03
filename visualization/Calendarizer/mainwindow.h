#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QTimer>


#include "process.h"
#include "../scheduler/scheduler.h"
#include "../scheduler/FIFO/fifo_scheduler.h"
#include "../scheduler/SJF/sjf_scheduler.h"
#include "../scheduler/SRTF/srtf_scheduler.h"
#include "../scheduler/RoundRobin/rr_scheduler.h"
#include "../scheduler/Priority/priority_scheduler.h"
#include "../visualization/Calendarizer/gantt_canvas.h"
#include "../synchronizer/synchronizer_peterson.h"
#include "../synchronizer/mutex_sync.h"
#include "../visualization/Calendarizer/sync_canvas.h"
#include "../visualization/Calendarizer/synclegend.h"

struct RunItem {
    Scheduler                              *sched     = nullptr;
    GanttCanvas                            *canvas    = nullptr;
    QString                                  name;
    bool                                     finished  = false;
    std::vector<std::pair<std::string,int>>  finishLog;  // <pid, finishTime>
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Menú principal
    void showManualInputModal();
    void goToSimulationScreen();
    void goToMainMenu();
    void loadProcessesFromFile();
    void goToSyncSimulationScreen();

    // Simulación scheduling
    void updateAllSchedulers();

    // Simulación sincronización
    void startSyncSimulation();
    void updateSyncSimulation();

private:
    // ---------- Datos generales ----------
    std::vector<Process>          loadedProcesses;
    std::vector<std::string>      selectedAlgorithms;

    // ---------- Menú Principal ----------
    QWidget                      *mainMenuWidget       = nullptr;
    QTableWidget                 *processTable         = nullptr;
    QGroupBox                    *multiAlgorithmGroup  = nullptr;
    QCheckBox                    *fifoCheck            = nullptr;
    QCheckBox                    *sjfCheck             = nullptr;
    QCheckBox                    *srtfCheck            = nullptr;
    QCheckBox                    *rrCheck              = nullptr;
    QCheckBox                    *priorityCheck        = nullptr;

    // ---------- Pantalla de Simulación de Scheduling ----------
    QStackedWidget               *stackedWidget        = nullptr;
    QWidget                      *simulationWidget     = nullptr;
    QScrollArea                  *scrollArea           = nullptr;
    QWidget                      *ganttContainer       = nullptr;
    QVBoxLayout                  *ganttContainerLayout = nullptr;
    QTableWidget                 *processesTable       = nullptr;
    QTableWidget                 *metricsTable         = nullptr;
    QPlainTextEdit               *metricsOutput        = nullptr;

    QTimer                       *simulationTimer      = nullptr;
    int                           simulationTick       = 0;
    std::vector<RunItem>          runItems;

    // ---------- Pantalla de Sincronización ----------
    QWidget                      *syncSimulationWidget = nullptr;
    QPlainTextEdit               *syncLog              = nullptr;
    QComboBox                    *syncAlgorithmSelector= nullptr;
    QTableWidget                 *syncProcessTable     = nullptr;
    QTimer                       *syncTimer            = nullptr;
    int                           syncTick             = 0;
    QLabel                       *syncStatusLabel      = nullptr;
    SyncCanvas                   *syncCanvas           = nullptr;
    SyncLegend                   *syncLegend           = nullptr;
    Synchronizer                 *currentSync          = nullptr;
    bool                          startedLog[2]        = { false, false };
    bool                          finishedLog[2]       = { false, false };
    std::vector<Process>          syncProcesses;

    // Creación de pantallas
    QWidget* createMainMenu();
    QWidget* createSimulationScreen();
    QWidget* createSyncSimulationScreen();

    // Método de ayuda
    void showModal(QWidget *content, const QString &title);
    void logSyncMessage(const QString &message);
};

#endif // MAINWINDOW_H
