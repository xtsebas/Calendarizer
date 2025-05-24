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
#include "synchronizer.h"
#include "sync_canvas.h"
#include "synclegend.h"

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

    void startSyncSimulation();
    void updateSyncSimulation();
};

#endif // MAINWINDOW_H
