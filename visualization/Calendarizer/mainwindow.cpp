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

std::function<void(QString)> syncLogger = nullptr;

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
    startedLog[0] = startedLog[1] = false;
    finishedLog[0] = finishedLog[1] = false;

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

    ganttOutput = new QPlainTextEdit;
    ganttOutput->setReadOnly(true);

    // NUEVO: Canvas visual
    priorityCanvas = new GanttCanvas;

    // NUEVO: Tabla de procesos
    priorityProcessTable = new QTableWidget;
    priorityProcessTable->setColumnCount(4);
    QStringList headers = {"PID", "Arrival", "Burst", "Priority"};
    priorityProcessTable->setHorizontalHeaderLabels(headers);
    priorityProcessTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    priorityProcessTable->horizontalHeader()->setStretchLastSection(true);

    // NUEVO: Label para mostrar estado
    priorityStatusLabel = new QLabel;

    layout->addWidget(priorityCanvas);
    layout->addWidget(priorityProcessTable);
    layout->addWidget(priorityStatusLabel);
    layout->addWidget(ganttOutput);  // sigue incluido, útil para debug
    layout->addWidget(label);
    layout->addWidget(backBtn);
    layout->addStretch();

    widget->setLayout(layout);
    return widget;
}


void MainWindow::logSyncMessage(const QString &message) {
    syncLog->appendPlainText(message);
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

    syncLog = new QPlainTextEdit;
    syncLog->setReadOnly(true);
    syncLog->setMaximumHeight(150);
    layout->addWidget(syncLog);

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
    startedLog[0] = startedLog[1] = false;
    finishedLog[0] = finishedLog[1] = false;

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

    // Limpiar logs anteriores
    syncLog->clear();

    syncLogger = [this](const QString &msg) {
        logSyncMessage(msg);
    };

    logSyncMessage("Simulación iniciada con el algoritmo: " + alg);

    syncProcesses.clear();
    if (alg == "Peterson" || alg == "Mutex") {
        if (loadedProcesses.size() < 2) return;
        syncProcesses = { loadedProcesses[0], loadedProcesses[1] };
    } else {
        syncProcesses = loadedProcesses;
    }

    syncCanvas->setProcesses(syncProcesses);
    syncCanvas->setMaxTicks(50);
    syncCanvas->reset();

    syncProcessTable->setRowCount(static_cast<int>(syncProcesses.size()));
    for (size_t i = 0; i < syncProcesses.size(); ++i) {
        const Process &p = syncProcesses[i];
        syncProcessTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(p.pid)));
        syncProcessTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::number(p.arrivalTime)));
        syncProcessTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::number(p.burstTime)));
        syncProcessTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(p.priority)));
    }

    syncTick = 0;
    syncTimer = new QTimer(this);
    connect(syncTimer, &QTimer::timeout, this, &MainWindow::updateSyncSimulation);
    syncTimer->start(1000);
}

void MainWindow::updateSyncSimulation() {
    bool allDone = true;
    int selectedPid = -1;
    int highestPriority = INT_MAX;

    QString alg = syncAlgorithmSelector->currentText();

    if (syncProcesses.size() > 2) {
        syncTimer->stop();
        syncStatusLabel->setText("Error: solo se permiten 2 procesos en este modo.");
        return;
    }

    for (size_t i = 0; i < syncProcesses.size(); ++i) {
        const Process &p = syncProcesses[i];
        if (p.burstTime > 0 && p.priority < highestPriority) {
            selectedPid = static_cast<int>(i);
            highestPriority = p.priority;
        }
    }

    for (size_t i = 0; i < syncProcesses.size(); ++i) {
        Process &p = syncProcesses[i];

        if (p.burstTime <= 0) {
            syncCanvas->addStep(i, SyncStep::Finished);
            continue;
        }

        if (static_cast<int>(i) == selectedPid) {
            int other = 1 - i;

            if (!startedLog[i]) {
                if (alg == "Peterson") {
                    logSyncMessage(QString("Proceso %1: flag[%1] = true, turn = %2 (%3)").arg(i).arg(other).arg(alg));
                    logSyncMessage(QString("Proceso %1: esperando (%3)").arg(i).arg(alg));
                } else if (alg == "Mutex") {
                    logSyncMessage(QString("Proceso %1: esperando (%2)").arg(i).arg(alg));
                }
            }

            currentSync->lock(i);

            if (!startedLog[i]) {
                startedLog[i] = true;

                if (alg == "Mutex") {
                    logSyncMessage(QString("Proceso %1: acquire() ejecutado (%2)").arg(i).arg(alg));
                }

                logSyncMessage(QString("Proceso %1: entra a sección crítica tiempo %2 (%3)").arg(i).arg(syncTick).arg(alg));
            }

            syncCanvas->addStep(i, SyncStep::Critical);
            p.burstTime--;

            if (p.burstTime == 0 && !finishedLog[i]) {
                finishedLog[i] = true;
                logSyncMessage(QString("Proceso %1: sale de sección crítica tiempo %2 (%3)").arg(i).arg(syncTick + 1).arg(alg));

                if (alg == "Mutex") {
                    logSyncMessage(QString("Proceso %1: release() ejecutado (%2)").arg(i).arg(alg));
                }
            }

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
        for (size_t i = 0; i < syncProcesses.size(); ++i) {
            syncCanvas->addStep(i, SyncStep::Finished);
        }

        logSyncMessage("SIMULACIÓN TERMINADA");
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

    selectedAlgorithms.clear();
    if (fifoCheck->isChecked()) selectedAlgorithms.push_back("FIFO");
    if (sjfCheck->isChecked()) selectedAlgorithms.push_back("SJF");
    if (srtfCheck->isChecked()) selectedAlgorithms.push_back("SRTF");
    if (rrCheck->isChecked()) selectedAlgorithms.push_back("Round Robin");
    if (priorityCheck->isChecked()) selectedAlgorithms.push_back("Priority");

    if (selectedAlgorithms.empty()) {
        QMessageBox::warning(this, "Error", "Debes seleccionar al menos un algoritmo.");
        return;
    }

    // Mostrar mensaje informativo
    QStringList algNames;
    for (const std::string& alg : selectedAlgorithms) {
        algNames << QString::fromStdString(alg);
    }

    QMessageBox::information(this, "Algoritmos elegidos", "Ejecutando: " + algNames.join(", "));
    stackedWidget->setCurrentWidget(simulationWidget);

    // Solo ejecutamos Gantt si es Priority único
    if (selectedAlgorithms.size() == 1 && selectedAlgorithms[0] == "Priority") {
        startPrioritySimulation();
    } else {
        ganttOutput->clear();
    }
}


void MainWindow::startPrioritySimulation() {
    qDebug() << "Iniciando simulación por prioridad...";

    if (priorityScheduler) {
        delete priorityScheduler;
        priorityScheduler = nullptr;
    }
    priorityScheduler = new PriorityScheduler();

    // Ordenar procesos por prioridad (menor valor = mayor prioridad)
    priorityReadyQueue = loadedProcesses;
    std::sort(priorityReadyQueue.begin(), priorityReadyQueue.end(), [](const Process& a, const Process& b) {
        return a.priority < b.priority;
    });

    // Agregar todos los procesos desde el inicio (ignora arrivalTime)
    for (const auto& process : priorityReadyQueue) {
        priorityScheduler->add_process(process);
    }

    priorityTick = 0;
    priorityCanvas->reset();

    // Llenar tabla de procesos
    priorityProcessTable->setRowCount(static_cast<int>(loadedProcesses.size()));
    for (size_t i = 0; i < loadedProcesses.size(); ++i) {
        const Process &p = loadedProcesses[i];
        priorityProcessTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(p.pid)));
        priorityProcessTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::number(p.arrivalTime)));
        priorityProcessTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::number(p.burstTime)));
        priorityProcessTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(p.priority)));
    }

    priorityStatusLabel->setText("Ejecutando...");

    if (priorityTimer) {
        priorityTimer->stop();
        delete priorityTimer;
        priorityTimer = nullptr;
    }

    priorityTimer = new QTimer(this);
    connect(priorityTimer, &QTimer::timeout, this, [=]() mutable {
        qDebug() << "Tick" << priorityTick;

        std::string pid = priorityScheduler->schedule_next(priorityTick);
        if (!pid.empty()) {
            int index = -1;
            for (size_t i = 0; i < loadedProcesses.size(); ++i) {
                if (loadedProcesses[i].pid == pid) {
                    index = static_cast<int>(i);
                    break;
                }
            }
            if (index != -1)
                priorityCanvas->addStep(index);
        } else {
            priorityCanvas->addIdleStep();
        }

        priorityTick++;

        if (priorityScheduler->get_pending_processes().empty()) {
            priorityTimer->stop();
            double wtavg = priorityScheduler->average_waiting_time();
            priorityStatusLabel->setText(QString("Simulación finalizada. Tiempo de espera promedio: %1").arg(wtavg));
            qDebug() << "Simulación terminada. WTavg:" << wtavg;
        }
    });

    priorityTimer->start(1000);
}


void MainWindow::goToSyncSimulationScreen() {
    stackedWidget->setCurrentWidget(syncSimulationWidget);
}

void MainWindow::goToMainMenu() {
    stackedWidget->setCurrentWidget(mainMenuWidget);
}
