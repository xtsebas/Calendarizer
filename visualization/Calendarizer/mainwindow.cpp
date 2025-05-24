#include "mainwindow.h"
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QTimer>

#include "file_loader.h"
#include "metrics.h"
#include "process.h"
#include "mutex_sync.h"
#include "semaphore_sync.h"
#include "synchronizer.h"
#include "synchronizer_peterson.h"
#include "FIFO/fifo_scheduler.h"
#include "Priority/priority_scheduler.h"
#include "RoundRobin/rr_scheduler.h"
#include "SJF/sjf_scheduler.h"
#include "SRTF/srtf_scheduler.h"
#include "scheduler.h"

#include "sync_canvas.h"
#include "synclegend.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    stackedWidget = new QStackedWidget(this);
    mainMenuWidget = createMainMenu();
    simulationWidget = createSimulationScreen();
    syncSimulationWidget = createSyncSimulationScreen();

    stackedWidget->addWidget(mainMenuWidget);
    stackedWidget->addWidget(simulationWidget);
    stackedWidget->addWidget(syncSimulationWidget);
    stackedWidget->setCurrentWidget(mainMenuWidget);

    setCentralWidget(stackedWidget);
    setWindowTitle("Calendarizador");
    resize(600, 400);
}

MainWindow::~MainWindow() {}

QWidget* MainWindow::createMainMenu() {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    syncTimer = nullptr;
    syncTick = 0;
    currentSync = nullptr;

    QPushButton *loadFromFileBtn = new QPushButton("Cargar desde archivo");
    QPushButton *manualInputBtn = new QPushButton("Ingreso manual");
    QPushButton *startSimulationBtn = new QPushButton("Iniciar simulación");
    QPushButton *syncSimulationBtn = new QPushButton("Simulación de sincronización");

    processTable = new QTableWidget;
    processTable->setColumnCount(4);
    QStringList headers = {"PID", "Arrival Time", "Burst Time", "Priority"};
    processTable->setHorizontalHeaderLabels(headers);
    processTable->horizontalHeader()->setStretchLastSection(true);
    processTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(loadFromFileBtn, &QPushButton::clicked, this, &MainWindow::loadProcessesFromFile);
    connect(manualInputBtn, &QPushButton::clicked, this, &MainWindow::showManualInputModal);
    connect(startSimulationBtn, &QPushButton::clicked, this, &MainWindow::goToSimulationScreen);
    connect(syncSimulationBtn, &QPushButton::clicked, this, &MainWindow::goToSyncSimulationScreen);

    multiAlgorithmGroup = new QGroupBox("Selecciona algoritmos a comparar:");
    QVBoxLayout *algLayout = new QVBoxLayout;

    fifoCheck = new QCheckBox("FIFO");
    sjfCheck = new QCheckBox("SJF");
    srtfCheck = new QCheckBox("SRTF");
    rrCheck = new QCheckBox("Round Robin");
    priorityCheck = new QCheckBox("Priority");

    algLayout->addWidget(fifoCheck);
    algLayout->addWidget(sjfCheck);
    algLayout->addWidget(srtfCheck);
    algLayout->addWidget(rrCheck);
    algLayout->addWidget(priorityCheck);

    multiAlgorithmGroup->setLayout(algLayout);
    layout->addWidget(multiAlgorithmGroup);

    layout->addWidget(new QLabel("Menú Principal"));
    layout->addWidget(loadFromFileBtn);
    layout->addWidget(manualInputBtn);
    layout->addWidget(startSimulationBtn);
    layout->addWidget(syncSimulationBtn);
    layout->addWidget(new QLabel("Procesos cargados:"));
    layout->addWidget(processTable);
    layout->addStretch();

    widget->setLayout(layout);
    return widget;
}

QWidget* MainWindow::createSimulationScreen() {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;

    QLabel *label = new QLabel("Ejecutando simulación...");
    QPushButton *backBtn = new QPushButton("Volver al menú");

    connect(backBtn, &QPushButton::clicked, this, &MainWindow::goToMainMenu);

    layout->addWidget(label);
    layout->addWidget(backBtn);
    layout->addStretch();

    widget->setLayout(layout);
    return widget;
}

QWidget* MainWindow::createSyncSimulationScreen() {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;

    QLabel *label = new QLabel("Simulación de sincronización");
    syncAlgorithmSelector = new QComboBox;
    syncAlgorithmSelector->addItems({"Peterson", "Mutex"});

    QPushButton *startBtn = new QPushButton("Iniciar simulación");
    QPushButton *backBtn = new QPushButton("Volver al menú");
    syncStatusLabel = new QLabel("Estado de la simulación");

    syncCanvas = new SyncCanvas;
    layout->addWidget(label);
    layout->addWidget(syncAlgorithmSelector);
    layout->addWidget(startBtn);
    layout->addWidget(syncCanvas);
    layout->addWidget(syncStatusLabel);
    layout->addWidget(backBtn);
    layout->addStretch();

    connect(startBtn, &QPushButton::clicked, this, &MainWindow::startSyncSimulation);
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::goToMainMenu);

    // Tabla de procesos
    syncProcessTable = new QTableWidget;
    syncProcessTable->setColumnCount(4);
    QStringList headers = {"PID", "Arrival", "Burst", "Priority"};
    syncProcessTable->setHorizontalHeaderLabels(headers);
    syncProcessTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    syncProcessTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(syncProcessTable);

    // Leyenda visual
    syncLegend = new SyncLegend;
    layout->addWidget(syncLegend);

    widget->setLayout(layout);
    return widget;
}


void MainWindow::startSyncSimulation() {
    if (syncTimer) {
        syncTimer->stop();
        delete syncTimer;
        syncTimer = nullptr;
    }

    if (loadedProcesses.empty()) {
        QMessageBox::critical(this, "Error", "No hay procesos cargados.");
        return;
    }

    QString alg = syncAlgorithmSelector->currentText();
    delete currentSync;
    currentSync = nullptr;

    if (alg == "Peterson")
        currentSync = new PetersonSynchronizer();
    else
        currentSync = new MutexSynchronizer();

    // Reconfigurar canvas con procesos cargados
    syncCanvas->setProcesses(loadedProcesses);
    syncCanvas->setMaxTicks(50);
    syncCanvas->reset();

    syncProcessTable->setRowCount(static_cast<int>(loadedProcesses.size()));
    for (size_t i = 0; i < loadedProcesses.size(); ++i) {
        const Process &p = loadedProcesses[i];
        syncProcessTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(p.pid)));
        syncProcessTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::number(p.arrivalTime)));
        syncProcessTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::number(p.burstTime)));
        syncProcessTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(p.priority)));
    }

    syncTick = 0;
    syncTimer = new QTimer(this);
    connect(syncTimer, &QTimer::timeout, this, &MainWindow::updateSyncSimulation);
    syncTimer->start(1000); // cada 1 segundo
}



void MainWindow::updateSyncSimulation() {
    bool allDone = true;
    int selectedPid = -1;
    int highestPriority = INT_MAX;

    // Elegir proceso con mayor prioridad
    for (size_t i = 0; i < loadedProcesses.size(); ++i) {
        const Process &p = loadedProcesses[i];
        if (p.burstTime > 0 && p.priority < highestPriority) {
            selectedPid = static_cast<int>(i);
            highestPriority = p.priority;
        }
    }

    if (selectedPid == -1) {
        syncTimer->stop();
        syncStatusLabel->setText("Todos los procesos terminaron.");
        return;
    }

    for (size_t i = 0; i < loadedProcesses.size(); ++i) {
        Process &p = loadedProcesses[i];

        if (p.burstTime <= 0) {
            syncCanvas->addStep(i, SyncStep::Finished);
        } else if (i == selectedPid) {
            currentSync->lock(i);
            syncCanvas->addStep(i, SyncStep::Critical);
            p.burstTime--;
            currentSync->unlock(i);
        } else {
            syncCanvas->addStep(i, SyncStep::Waiting);
        }

        if (p.burstTime > 0) {
            allDone = false;
        }
    }

    syncTick++;
    if (allDone || syncTick >= 50) {
        // Asegurar que todos terminen en verde visualmente
        for (size_t i = 0; i < loadedProcesses.size(); ++i) {
            syncCanvas->addStep(i, SyncStep::Finished);
        }

        syncTimer->stop();
        syncStatusLabel->setText("Simulación finalizada.");
    }

}





void MainWindow::showModal(QWidget *content, const QString &title) {
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    QVBoxLayout layout;
    layout.addWidget(content);
    dialog.setLayout(&layout);
    dialog.exec();
}

void MainWindow::showManualInputModal() {
    QWidget *modalContent = new QWidget;
    QFormLayout *formLayout = new QFormLayout;

    QLineEdit *pidEdit = new QLineEdit;
    QLineEdit *btEdit = new QLineEdit;
    QLineEdit *priorityEdit = new QLineEdit;

    formLayout->addRow("PID:", pidEdit);
    formLayout->addRow("Burst Time:", btEdit);
    formLayout->addRow("Prioridad:", priorityEdit);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, [=]() {
        QMessageBox::information(this, "Proceso agregado", "PID: " + pidEdit->text());
        buttons->parentWidget()->close();
    });
    connect(buttons, &QDialogButtonBox::rejected, [=]() {
        buttons->parentWidget()->close();
    });

    formLayout->addWidget(buttons);
    modalContent->setLayout(formLayout);

    showModal(modalContent, "Ingreso Manual");
}

void MainWindow::loadProcessesFromFile() {
    QString filename = QFileDialog::getOpenFileName(this, "Seleccionar archivo de procesos");
    if (!filename.isEmpty()) {
        std::vector<Process> procs = FileLoader::loadProcesses(filename.toStdString());
        processTable->setRowCount(static_cast<int>(procs.size()));

        for (size_t i = 0; i < procs.size(); ++i) {
            processTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(procs[i].pid)));
            processTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::number(procs[i].arrivalTime)));
            processTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::number(procs[i].burstTime)));
            processTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(procs[i].priority)));
        }

        QMessageBox::information(this, "Archivo cargado",
                                 "Archivo: " + filename + "\nProcesos cargados: " + QString::number(procs.size()));

        loadedProcesses = procs;
    }
}

void MainWindow::goToSimulationScreen() {
    if (loadedProcesses.empty()) {
        QMessageBox::warning(this, "Error", "Primero debes cargar procesos.");
        return;
    }

    QStringList selectedAlgs;
    if (fifoCheck->isChecked()) selectedAlgs << "FIFO";
    if (sjfCheck->isChecked()) selectedAlgs << "SJF";
    if (srtfCheck->isChecked()) selectedAlgs << "SRTF";
    if (rrCheck->isChecked()) selectedAlgs << "Round Robin";
    if (priorityCheck->isChecked()) selectedAlgs << "Priority";

    if (selectedAlgs.isEmpty()) {
        QMessageBox::warning(this, "Error", "Debes seleccionar al menos un algoritmo.");
        return;
    }

    QMessageBox::information(this, "Algoritmos elegidos", "Ejecutando: " + selectedAlgs.join(", "));
    stackedWidget->setCurrentWidget(simulationWidget);
}

void MainWindow::goToSyncSimulationScreen() {
    stackedWidget->setCurrentWidget(syncSimulationWidget);
}

void MainWindow::goToMainMenu() {
    stackedWidget->setCurrentWidget(mainMenuWidget);
}
