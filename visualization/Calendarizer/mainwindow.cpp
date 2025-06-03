#include "mainwindow.h"
#include <iomanip>

// — Qt —
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
#include <QInputDialog>
#include <QScrollArea>
#include <QPlainTextEdit>

// — Lógica de procesos y schedulers —
#include "file_loader.h"
#include "metrics.h"
#include "process.h"
#include "../scheduler/FIFO/fifo_scheduler.h"
#include "../scheduler/SJF/sjf_scheduler.h"
#include "../scheduler/SRTF/srtf_scheduler.h"
#include "../scheduler/RoundRobin/rr_scheduler.h"
#include "../scheduler/Priority/priority_scheduler.h"
#include "../visualization/Calendarizer/gantt_canvas.h"

// — Sincronización —
#include "../synchronizer/synchronizer_peterson.h"
#include "../synchronizer/mutex_sync.h"
#include "../visualization/Calendarizer/sync_canvas.h"
#include "../visualization/Calendarizer/synclegend.h"
#include "../synchronizer/semaphore_sync.h"

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
    QVBoxLayout *mainLayout = new QVBoxLayout;

    // 1) Título
    QLabel *title = new QLabel("Simulación de Scheduling");
    title->setAlignment(Qt::AlignCenter);

    // 2) ScrollArea con GanttCanvas(s)
    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    ganttContainer = new QWidget;
    ganttContainerLayout = new QVBoxLayout;
    ganttContainer->setLayout(ganttContainerLayout);
    scrollArea->setWidget(ganttContainer);

    // 3) Tabla de procesos (solo para referencia)
    processesTable = new QTableWidget;
    processesTable->setColumnCount(4);
    QStringList headers = { "PID", "Arrival Time", "Burst Time", "Priority" };
    processesTable->setHorizontalHeaderLabels(headers);
    processesTable->horizontalHeader()->setStretchLastSection(true);
    processesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 4) Área de métricas
    metricsOutput = new QPlainTextEdit;
    metricsOutput->setReadOnly(true);
    metricsOutput->setPlaceholderText("Aquí aparecerán las métricas (Avg WT) de cada algoritmo…");

    // — Nueva tabla para mostrar métricas en modo múltiple —
    metricsTable = new QTableWidget;
    metricsTable->setColumnCount(4);
    QStringList metHeaders = { "Algoritmo", "Avg WT", "Avg TT", "Throughput" };
    metricsTable->setHorizontalHeaderLabels(metHeaders);
    metricsTable->horizontalHeader()->setStretchLastSection(true);
    metricsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // Inicialmente, la ocultamos
    metricsTable->setVisible(false);

    // 5) Botón “Volver al menú”
    QPushButton *backBtn = new QPushButton("Volver al menú");
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::goToMainMenu);

    // 6) Montar en layout principal
    mainLayout->addWidget(title);
    mainLayout->addWidget(new QLabel("Gantt de cada algoritmo:"));
    mainLayout->addWidget(scrollArea, /*stretch=*/1);
    mainLayout->addWidget(new QLabel("Procesos:"));
    mainLayout->addWidget(processesTable);
    mainLayout->addWidget(new QLabel("Métricas:"));
    mainLayout->addWidget(metricsOutput);
    mainLayout->addWidget(metricsTable);   // <-- insertamos la tabla justo debajo del QPlainTextEdit
    mainLayout->addWidget(backBtn);
    mainLayout->addStretch();

    widget->setLayout(mainLayout);
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
    syncAlgorithmSelector->addItems({"Peterson", "Mutex", "Semáforo"});

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

    // 1) Instanciar el sincronizador correcto según la opción escogida
    if (alg == "Peterson") {
        currentSync = new PetersonSynchronizer();
    }
    else if (alg == "Mutex") {
        currentSync = new MutexSynchronizer();
    }
    else if (alg == "Semáforo") {
        // Semáforo binario (count = 1). Si quisieras otro conteo, cambia el parámetro.
        currentSync = new SemaphoreSynchronizer(1);
    }
    else {
        // Si llegara un texto inesperado, abortamos.
        QMessageBox::critical(this, "Error", "Algoritmo de sincronización desconocido.");
        return;
    }

    // 2) Limpiar y preparar la interfaz
    syncLog->clear();
    syncLogger = [this](const QString &msg) {
        logSyncMessage(msg);
    };
    logSyncMessage("Simulación iniciada con el algoritmo: " + alg);

    // 3) Seleccionar los procesos a sincronizar
    //    Tanto Peterson, Mutex y Semáforo requieren exactamente 2 procesos.
    if (loadedProcesses.size() < 2) {
        QMessageBox::warning(this, "Error", "Se necesitan al menos 2 procesos para esta simulación.");
        return;
    }
    // Solo tomamos los dos primeros procesos de loadedProcesses:
    syncProcesses = { loadedProcesses[0], loadedProcesses[1] };

    // 4) Configurar el canvas y la tabla
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

    // 5) Iniciar el timer que va a llamar a updateSyncSimulation cada segundo
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

    // 1) Recopilar todos los algoritmos marcados
    selectedAlgorithms.clear();
    if (fifoCheck->isChecked())     selectedAlgorithms.push_back("FIFO");
    if (sjfCheck->isChecked())      selectedAlgorithms.push_back("SJF");
    if (srtfCheck->isChecked())     selectedAlgorithms.push_back("SRTF");
    if (rrCheck->isChecked())       selectedAlgorithms.push_back("Round Robin");
    if (priorityCheck->isChecked()) selectedAlgorithms.push_back("Priority");

    if (selectedAlgorithms.empty()) {
        QMessageBox::warning(this, "Error", "Debes seleccionar al menos un algoritmo.");
        return;
    }

    // 2) Cambiar a la pantalla de simulación
    stackedWidget->setCurrentWidget(simulationWidget);

    // 3) Limpiar canvases y métricas previas
    metricsOutput->clear();
    metricsTable->clearContents();
    metricsTable->setRowCount(0); // limpiamos filas
    QLayoutItem *child;
    while ((child = ganttContainerLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    runItems.clear();
    simulationTick = 0;

    // 4) Llenar la tabla de procesos (referencia visual), sin importar cuántos algoritmos haya
    processesTable->setRowCount(static_cast<int>(loadedProcesses.size()));
    for (int i = 0; i < loadedProcesses.size(); ++i) {
        const Process &p = loadedProcesses[i];
        processesTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(p.pid)));
        processesTable->setItem(i, 1, new QTableWidgetItem(QString::number(p.arrivalTime)));
        processesTable->setItem(i, 2, new QTableWidgetItem(QString::number(p.burstTime)));
        processesTable->setItem(i, 3, new QTableWidgetItem(QString::number(p.priority)));
    }

    // ========= CASO B: >1 ALGORITMO → solo mostramos la tabla de métricas =========
    if (selectedAlgorithms.size() > 1) {
        //  a) Ocultar el diagrama de Gantt
        scrollArea->setVisible(false);

        //  b) Ocultar el cuadro de texto metricsOutput
        metricsOutput->setVisible(false);

        //  c) Mostrar la tabla metricsTable
        metricsTable->setVisible(true);

        //  d) Configurar filas y columnas en metricsTable
        int n = static_cast<int>(selectedAlgorithms.size());
        metricsTable->setRowCount(n);

        //   — Poner ancho fijo a las columnas para que quepan bien —
        metricsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        //   Las demás (1,2,3) podemos hacer que ajusten por contenido:
        metricsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        metricsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        metricsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

        //  e) Para cada algoritmo, simular “rápido” y llenar la tabla
        for (int row = 0; row < n; ++row) {
            const QString chosenAlg = QString::fromStdString(selectedAlgorithms[row]);
            Scheduler *sched = nullptr;
            int quantum = 0;

            //  e.1) Instanciar el Scheduler
            if (chosenAlg == "FIFO") {
                sched = new FIFO_Scheduler();
            }
            else if (chosenAlg == "SJF") {
                sched = new SJF_Scheduler();
            }
            else if (chosenAlg == "SRTF") {
                sched = new SRTF_Scheduler();
            }
            else if (chosenAlg == "Round Robin") {
                bool ok;
                quantum = QInputDialog::getInt(
                    this,
                    "Quantum de Round Robin",
                    "Ingresa el quantum para comparar los algoritmos:",
                    3,    // valor por defecto
                    1,    // mínimo
                    100,  // máximo
                    1,    // paso
                    &ok
                    );
                if (!ok) quantum = 3;
                sched = new RR_Scheduler(quantum);
            }
            else if (chosenAlg == "Priority") {
                sched = new PriorityScheduler();
            }

            //  e.2) Agregar los mismos procesos
            for (const Process &p : loadedProcesses) {
                sched->add_process(p);
            }

            //  e.3) Simular “rápido” sin dibujar, para obtener finishLog
            std::vector<std::pair<std::string,int>> finishLog;
            int t = 0;
            auto pending = sched->get_pending_processes();
            while (!pending.empty()) {
                std::string pid = sched->schedule_next(t);
                if (!pid.empty()) {
                    // Si ya no está pendiente, registramos finishTime
                    auto nuevosPend = sched->get_pending_processes();
                    bool siguePend = false;
                    for (auto &pr : nuevosPend) {
                        if (pr.pid == pid) { siguePend = true; break; }
                    }
                    if (!siguePend) {
                        finishLog.emplace_back(pid, t + 1);
                    }
                }
                ++t;
                pending = sched->get_pending_processes();
            }

            //  e.4) Calcular métricas
            double avgWT = averageWaitingTime(finishLog, loadedProcesses);
            double avgTT = averageTurnaroundTime(finishLog, loadedProcesses);
            int totalTicks = t > 0 ? t : 1;
            int totalProcs = static_cast<int>(finishLog.size());
            double throughput = static_cast<double>(totalProcs) / totalTicks;

            //  e.5) Llenar la fila “row” de metricsTable
            //      Columna 0: Nombre de algoritmo (string),
            //      si es “Round Robin” agregamos “(q=<quantum>)”
            QString labelAlg = chosenAlg;
            if (chosenAlg == "Round Robin") {
                labelAlg += QString(" (q=%1)").arg(quantum);
            }
            metricsTable->setItem(row, 0, new QTableWidgetItem(labelAlg));
            metricsTable->setItem(row, 1, new QTableWidgetItem(
                                              QString::number(avgWT, 'f', 6)));
            metricsTable->setItem(row, 2, new QTableWidgetItem(
                                              QString::number(avgTT, 'f', 6)));
            metricsTable->setItem(row, 3, new QTableWidgetItem(
                                              QString::number(throughput, 'f', 6)));

            delete sched;
        }

        return;
    }

    // 1.a) Mostrar el Gantt
    scrollArea->setVisible(true);
    // 1.b) Ocultar la tabla que usamos para varios algoritmos
    metricsTable->setVisible(false);
    // 1.c) Mostrar el cuadro de texto metricsOutput (dibujará al final)
    metricsOutput->setVisible(true);

    // 2) Preparar un único Scheduler y un único GanttCanvas
    QString chosenAlgStd = QString::fromStdString(selectedAlgorithms[0]);
    Scheduler   *sched    = nullptr;
    GanttCanvas *canvas   = new GanttCanvas;
    QString      name     = chosenAlgStd;

    // 2.a) Instanciar el Scheduler correspondiente
    if (chosenAlgStd == "FIFO") {
        sched = new FIFO_Scheduler();
    }
    else if (chosenAlgStd == "SJF") {
        sched = new SJF_Scheduler();
    }
    else if (chosenAlgStd == "SRTF") {
        sched = new SRTF_Scheduler();
    }
    else if (chosenAlgStd == "Round Robin") {
        bool ok;
        int quantum = QInputDialog::getInt(
            this,
            "Quantum de Round Robin",
            "Ingresa el quantum:",
            3, 1, 100, 1, &ok
            );
        if (!ok) quantum = 3;
        sched = new RR_Scheduler(quantum);
        name += QString(" (q=%1)").arg(quantum);
    }
    else if (chosenAlgStd == "Priority") {
        sched = new PriorityScheduler();
    }

    // 2.b) Agregar procesos al scheduler
    for (const Process &p : loadedProcesses) {
        sched->add_process(p);
    }

    // 2.c) Preparar lista de PIDs y asignar al GanttCanvas
    {
        QStringList pidList;
        pidList.reserve(static_cast<int>(loadedProcesses.size()));
        for (const Process &p : loadedProcesses) {
            pidList.append(QString::fromStdString(p.pid));
        }
        canvas->setProcessLabels(pidList);
    }

    // 2.d) Añadir el canvas al layout
    ganttContainerLayout->addWidget(canvas);

    // 2.e) Guardar en runItems para la simulación en tiempo real
    RunItem item;
    item.sched    = sched;
    item.canvas   = canvas;
    item.name     = name;
    item.finished = false;
    runItems.push_back(item);

    // 3) Arrancar el timer que pinta el Gantt cada segundo
    if (simulationTimer) {
        simulationTimer->stop();
        delete simulationTimer;
    }
    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::updateAllSchedulers);
    simulationTimer->start(1000);
}

void MainWindow::goToSyncSimulationScreen() {
    stackedWidget->setCurrentWidget(syncSimulationWidget);
}

void MainWindow::goToMainMenu() {
    stackedWidget->setCurrentWidget(mainMenuWidget);
}


void MainWindow::updateAllSchedulers() {
    // 1) Recorrer cada RunItem
    for (auto &item : runItems) {
        if (item.finished)
            continue;

        // 1.1) Ejecutar schedule_next en este tick
        std::string pid = item.sched->schedule_next(simulationTick);

        if (!pid.empty()) {
            // 1.2) Pintar el paso en el canvas correspondiente
            int indexProc = -1;
            for (int i = 0; i < loadedProcesses.size(); ++i) {
                if (loadedProcesses[i].pid == pid) {
                    indexProc = i;
                    break;
                }
            }
            if (indexProc >= 0) {
                item.canvas->addStep(indexProc);
            }

            // 1.3) Si este pid ya no está en pendientes, guardar finishTime
            auto pendientes = item.sched->get_pending_processes();
            bool siguePendiente = false;
            for (const Process &p : pendientes) {
                if (p.pid == pid) {
                    siguePendiente = true;
                    break;
                }
            }
            if (!siguePendiente) {
                item.finishLog.push_back({ pid, simulationTick + 1 });
            }
        } else {
            // 1.4) CPU Idle → pintar IdleStep
            item.canvas->addIdleStep();
        }

        // 1.5) Si este scheduler ya no tiene procesos pendientes, marcarlo terminado
        if (item.sched->get_pending_processes().empty()) {
            item.finished = true;

            // Calcular Avg WT y Avg TT
            double avgWT = averageWaitingTime(item.finishLog, loadedProcesses);
            double avgTT = averageTurnaroundTime(item.finishLog, loadedProcesses);

            // (Opcional) Calcular throughput
            int totalTicks   = simulationTick;
            int totalProcs   = static_cast<int>(item.finishLog.size());
            double throughput = (totalTicks > 0)
                                    ? (static_cast<double>(totalProcs) / totalTicks)
                                    : 0.0;

            // Mostrar todas las métricas juntas
            metricsOutput->appendPlainText(
                QString("%1 → Avg WT: %2, Avg TT: %3, Throughput: %4 p/tick")
                    .arg(item.name)
                    .arg(QString::number(avgWT, 'f', 6))
                    .arg(QString::number(avgTT, 'f', 6))
                    .arg(QString::number(throughput, 'f', 6))
                );
        }
    }

    // 2) Verificar si TODOS terminaron
    bool todos = true;
    for (const auto &it : runItems) {
        if (!it.finished) {
            todos = false;
            break;
        }
    }
    if (todos) {
        simulationTimer->stop();
        metricsOutput->appendPlainText("\n=== SIMULACIÓN COMPLETADA ===");
    }

    // 3) Subir el tick global
    simulationTick++;
}
