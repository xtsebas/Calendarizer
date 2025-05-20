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

    QWidget* createMainMenu();
    QWidget* createSimulationScreen();
    void showModal(QWidget *content, const QString &title);
    QTableWidget *processTable;
};

#endif // MAINWINDOW_H
