#include "mainwindow.h"
#include <iomanip>

// — Qt —
#include <QLabel>
#include <set>
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
#include "../synchronizer/semaphore_sync.h"
#include "../visualization/Calendarizer/sync_canvas.h"
#include "../visualization/Calendarizer/synclegend.h"

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

    QPushButton *loadFromFileBtn = new QPushButton("Cargar procesos desde archivo");
    QPushButton *loadResourcesBtn = new QPushButton("Cargar recursos desde archivo");
    QPushButton *loadActionsBtn = new QPushButton("Cargar acciones desde archivo");
    auto *clearProcsBtn     = new QPushButton("Limpiar procesos");
    auto *clearResourcesBtn = new QPushButton("Limpiar recursos");
    auto *clearActionsBtn   = new QPushButton("Limpiar acciones");
    QPushButton *manualInputBtn = new QPushButton("Ingreso manual");
    QPushButton *startSimulationBtn = new QPushButton("Iniciar simulación");
    QPushButton *syncSimulationBtn = new QPushButton("Simulación de sincronización");

    processTable = new QTableWidget;
    processTable->setColumnCount(4);
    QStringList headers = {"PID", "Arrival Time", "Burst Time", "Priority"};
    processTable->setHorizontalHeaderLabels(headers);
    processTable->horizontalHeader()->setStretchLastSection(true);
    processTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    resourceTable = new QTableWidget;
    resourceTable->setColumnCount(2);
    resourceTable->setHorizontalHeaderLabels({"Nombre", "Cantidad"});
    resourceTable->horizontalHeader()->setStretchLastSection(true);
    resourceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    actionTable = new QTableWidget;
    actionTable->setColumnCount(4);
    actionTable->setHorizontalHeaderLabels({"PID", "Acción", "Recurso", "Ciclo"});
    actionTable->horizontalHeader()->setStretchLastSection(true);
    actionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(loadFromFileBtn, &QPushButton::clicked, this, &MainWindow::loadProcessesFromFile);
    connect(loadResourcesBtn, &QPushButton::clicked, this, &MainWindow::loadResourcesFromFile);
    connect(loadActionsBtn, &QPushButton::clicked, this, &MainWindow::loadActionsFromFile);
    connect(manualInputBtn, &QPushButton::clicked, this, &MainWindow::showManualInputModal);
    connect(startSimulationBtn, &QPushButton::clicked, this, &MainWindow::goToSimulationScreen);
    connect(syncSimulationBtn, &QPushButton::clicked, this, &MainWindow::goToSyncSimulationScreen);
    connect(clearProcsBtn,     &QPushButton::clicked, this, &MainWindow::clearProcesses);
    connect(clearResourcesBtn, &QPushButton::clicked, this, &MainWindow::clearResources);
    connect(clearActionsBtn,   &QPushButton::clicked, this, &MainWindow::clearActions);


    // 1) Grupo de selección de algoritmos
    multiAlgorithmGroup = new QGroupBox("Selecciona algoritmos a comparar:");
    QVBoxLayout *algLayout = new QVBoxLayout;

    fifoCheck     = new QCheckBox("FIFO");
    sjfCheck      = new QCheckBox("SJF");
    srtfCheck     = new QCheckBox("SRTF");
    rrCheck       = new QCheckBox("Round Robin");
    priorityCheck = new QCheckBox("Priority");

    algLayout->addWidget(fifoCheck);
    algLayout->addWidget(sjfCheck);
    algLayout->addWidget(srtfCheck);
    algLayout->addWidget(rrCheck);
    algLayout->addWidget(priorityCheck);
    multiAlgorithmGroup->setLayout(algLayout);

    // 2) Layout horizontal que contiene el grupo de algoritmos + botones "Limpiar"
    QHBoxLayout *algAndClearLayout = new QHBoxLayout;
    algAndClearLayout->addWidget(multiAlgorithmGroup);

    QVBoxLayout *clearBtnsLayout = new QVBoxLayout;
    clearBtnsLayout->addWidget(clearProcsBtn);
    clearBtnsLayout->addWidget(clearResourcesBtn);
    clearBtnsLayout->addWidget(clearActionsBtn);
    clearBtnsLayout->addStretch(); // Empuja los botones hacia arriba

    algAndClearLayout->addLayout(clearBtnsLayout);
    layout->addLayout(algAndClearLayout);

    layout->addWidget(new QLabel("Menú Principal"));
    QHBoxLayout *fileButtonsLayout = new QHBoxLayout;
    fileButtonsLayout->addWidget(loadFromFileBtn);
    fileButtonsLayout->addWidget(loadResourcesBtn);
    fileButtonsLayout->addWidget(loadActionsBtn);
    fileButtonsLayout->addWidget(manualInputBtn);

    layout->addLayout(fileButtonsLayout);
    layout->addWidget(startSimulationBtn);
    layout->addWidget(syncSimulationBtn);

    layout->addWidget(new QLabel("Procesos cargados:"));
    layout->addWidget(processTable);
    layout->addStretch();
    layout->addWidget(new QLabel("Recursos cargados:"));
    layout->addWidget(resourceTable);
    layout->addWidget(new QLabel("Acciones cargadas:"));
    layout->addWidget(actionTable);

    widget->setLayout(layout);
    return widget;
}

void MainWindow::loadResourcesFromFile() {
    QString filename = QFileDialog::getOpenFileName(this, "Seleccionar archivo de recursos");
    if (!filename.isEmpty()) {
        auto resources = FileLoader::loadResources(filename.toStdString());
        resourceTable->setRowCount(static_cast<int>(resources.size()));

        for (size_t i = 0; i < resources.size(); ++i) {
            resourceTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(resources[i].name)));
            resourceTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::number(resources[i].count)));
        }

        loadedResources = std::move(resources);
        QMessageBox::information(this, "Archivo cargado",
                                 "Archivo: " + filename + "\nRecursos cargados: " + QString::number(loadedResources.size()));
    }
}

void MainWindow::clearProcesses() {
    loadedProcesses.clear();
    processTable->setRowCount(0);
}

void MainWindow::clearResources() {
    loadedResources.clear();
    resourceTable->setRowCount(0);
}

void MainWindow::clearActions() {
    loadedActions.clear();
    actionTable->setRowCount(0);
}

void MainWindow::loadActionsFromFile() {
    QString filename = QFileDialog::getOpenFileName(this, "Seleccionar archivo de acciones");
    if (!filename.isEmpty()) {
        auto actions = FileLoader::loadActions(filename.toStdString());
        actionTable->setRowCount(static_cast<int>(actions.size()));

        for (size_t i = 0; i < actions.size(); ++i) {
            actionTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(actions[i].pid)));
            actionTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(actions[i].type)));
            actionTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::fromStdString(actions[i].resource)));
            actionTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(actions[i].cycle)));
        }

        loadedActions = std::move(actions);
        QMessageBox::information(this, "Archivo cargado",
                                 "Archivo: " + filename + "\nAcciones cargadas: " + QString::number(loadedActions.size()));
    }
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
    metricsTable->horizontalHeader()->setStretchLastSection(true); metricsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
    mainLayout->addWidget(metricsTable);
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
    syncAlgorithmSelector->addItems({"Mutex", "Semáforo"});

    QPushButton *startBtn = new QPushButton("Iniciar simulación");
    QPushButton *backBtn = new QPushButton("Volver al menú");
    syncStatusLabel = new QLabel("Estado de la simulación");

    // Área de log (arriba):
    syncLog = new QPlainTextEdit;
    syncLog->setReadOnly(true);
    syncLog->setMaximumHeight(150);
    layout->addWidget(syncLog);

    // ─── SyncCanvas dentro de QScrollArea ───
    syncCanvas = new SyncCanvas;

    // 1) Elevamos la altura mínima (por defecto era 300)
    syncCanvas->setMinimumHeight(300);

    // 2) Permitimos que crezca si hay espacio extra
    syncCanvas->setSizePolicy(QSizePolicy::Expanding,
                              QSizePolicy::Expanding);

    const int maxTicks = 50;
    syncCanvas->setMaxTicks(maxTicks);
    syncCanvas->setMinimumWidth(maxTicks * 20 + 200);

    QScrollArea *syncScrollArea = new QScrollArea;
    syncScrollArea->setWidgetResizable(true);
    syncScrollArea->setWidget(syncCanvas);
    syncScrollArea->setMinimumHeight(500);

    layout->addWidget(label);
    layout->addWidget(syncAlgorithmSelector);
    layout->addWidget(startBtn);
    layout->addWidget(syncScrollArea);   // ← aquí se ve el área coloreada más alta
    layout->addWidget(syncStatusLabel);
    layout->addWidget(backBtn);
    // ────────────────────────────────────────

    connect(startBtn, &QPushButton::clicked,
            this, &MainWindow::startSyncSimulation);
    connect(backBtn, &QPushButton::clicked,
            this, &MainWindow::goToMainMenu);

    // Tabla de acciones y leyenda (debajo)
    syncProcessTable = new QTableWidget;
    syncProcessTable->setColumnCount(4);
    QStringList headers = {"PID", "Acción", "Recurso", "Ciclo"};
    syncProcessTable->setHorizontalHeaderLabels(headers);
    syncProcessTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    syncProcessTable->horizontalHeader()->setStretchLastSection(true);
    syncProcessTable->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Expanding);
    layout->addWidget(syncProcessTable);

    syncLegend = new SyncLegend;
    layout->addWidget(syncLegend);

    layout->addStretch();
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

    if (loadedActions.empty()) {
        QMessageBox::critical(this, "Error", "No hay acciones cargadas.");
        return;
    }

    QString alg = syncAlgorithmSelector->currentText();

    if (currentSync) {
        delete currentSync;
        currentSync = nullptr;
    }

    try {
        if (alg == "Peterson") currentSync = new PetersonSynchronizer();
        else if (alg == "Mutex") currentSync = new MutexSynchronizer();
        else if (alg == "Semáforo") currentSync = new SemaphoreSynchronizer(1);
        else {
            QMessageBox::critical(this, "Error", "Algoritmo desconocido.");
            return;
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Error al crear sincronizador: %1").arg(e.what()));
        return;
    }

    syncLog->clear();
    syncLogger = [this](const QString &msg) { logSyncMessage(msg); };
    logSyncMessage("Simulación iniciada con el algoritmo: " + alg);

    // Procesos únicos desde loadedActions
    syncProcesses.clear();
    std::set<std::string> seen;
    for (const auto& action : loadedActions) {
        if (seen.insert(action.pid).second) {
            syncProcesses.emplace_back(action.pid, 0, 0, 0);
        }
    }

    if (syncProcesses.empty()) {
        QMessageBox::warning(this, "Error", "No se encontraron procesos válidos.");
        return;
    }

    syncCanvas->setProcesses(syncProcesses);
    syncCanvas->setMaxTicks(50);
    syncCanvas->reset();

    syncProcessTable->setRowCount(static_cast<int>(loadedActions.size()));
    for (size_t i = 0; i < loadedActions.size(); ++i) {
        const auto& a = loadedActions[i];
        syncProcessTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(a.pid)));
        syncProcessTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(a.type)));
        syncProcessTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(a.resource)));
        syncProcessTable->setItem(i, 3, new QTableWidgetItem(QString::number(a.cycle)));
    }

    syncTick = 0;
    syncTimer = new QTimer(this);
    connect(syncTimer, &QTimer::timeout, this, &MainWindow::updateSyncSimulation);
    syncTimer->start(1000);
}



struct ActionState {
    FileLoader::Action action;
    int step = 0;           // 0=Waiting, 1=Acquire, 2=Critical, 3=Release, 4=Finished, 5=Done
    int index;              // índice del proceso en el canvas
    int criticalTicks = 0;  // cuántos ticks ya pasó en "Critical" (para forzar un ciclo completo)
};

void MainWindow::updateSyncSimulation() {
    static std::vector<ActionState> activeActions;
    static std::map<std::string, int> pidToIndex;

    if (!currentSync) {
        logSyncMessage("Error: Sincronizador no inicializado");
        syncTimer->stop();
        return;
    }

    if (syncTick == 0) {
        activeActions.clear();
        pidToIndex.clear();

        for (size_t i = 0; i < syncProcesses.size(); ++i)
            pidToIndex[syncProcesses[i].pid] = static_cast<int>(i);

        for (const auto& a : loadedActions) {
            if (pidToIndex.count(a.pid)) {
                activeActions.push_back(ActionState{ a, 0, pidToIndex[a.pid] });
            }
        }

        qDebug() << "[INIT] Acciones totales:" << activeActions.size();
    }

    qDebug() << "[TICK]" << syncTick;

    bool allDone = true;

    for (auto& state : activeActions) {
        if (syncTick < state.action.cycle) {
            allDone = false;
            continue;
        }

        switch (state.step) {
            case 0:
                syncCanvas->addStep(state.index, syncTick, SyncStep::Waiting, state.action.type);
                state.step = 1;
                allDone = false;
                break;

            case 1:
                if (currentSync->try_lock(state.index)) {
                    // Si es semáforo, usamos Wait; si no, usamos Acquire
                    if (dynamic_cast<SemaphoreSynchronizer*>(currentSync)) {
                        syncCanvas->addStep(state.index, syncTick, SyncStep::Wait, state.action.type);
                    } else {
                        syncCanvas->addStep(state.index, syncTick, SyncStep::Acquire, state.action.type);
                    }
                    state.step = 2;
                } else {
                    syncCanvas->addStep(state.index, syncTick, SyncStep::Waiting, state.action.type);
                }
                allDone = false;
                break;

            case 2:
                // Pintar "Critical" y permanecer un ciclo completo antes de avanzar
                syncCanvas->addStep(state.index, syncTick, SyncStep::Critical, state.action.type);

                if (state.criticalTicks == 0) {
                    // Este es el primer tick de "Critical": incrementamos y nos quedamos en step=2
                    state.criticalTicks = 1;
                } else {
                    // Ya cumplió un tick completo en "Critical": pasamos a Release
                    state.step = 3;
                }
                allDone = false;
                break;

            case 3:
                currentSync->unlock(state.index);
                // Si es semáforo, usamos Signal; si no, usamos Release
                if (dynamic_cast<SemaphoreSynchronizer*>(currentSync)) {
                    syncCanvas->addStep(state.index, syncTick, SyncStep::Signal, state.action.type);
                } else {
                    syncCanvas->addStep(state.index, syncTick, SyncStep::Release, state.action.type);
                }
                state.step = 4;
                allDone = false;
                break;

            case 4:
                syncCanvas->addStep(state.index, syncTick, SyncStep::Finished, state.action.type);
                state.step = 5;
                break;

            case 5:
                // Ya terminado
                break;
        }

        if (state.step != 5) {
            allDone = false;
        }
    }

    syncTick++;

    if (allDone) {
        for (size_t i = 0; i < syncProcesses.size(); ++i)
            syncCanvas->addStep(static_cast<int>(i), syncTick, SyncStep::Finished);
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
                if (chosenAlg == "Priority") {
                    Process pCopy = p;
                    pCopy.arrivalTime = 0;
                    sched->add_process(pCopy);
                } else {
                    sched->add_process(p);
                }
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
            double avgWT;
            if (chosenAlg == "Priority") {
                avgWT = sched->average_waiting_time();
            } else {
                avgWT = averageWaitingTime(finishLog, loadedProcesses);
            }
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
        if (chosenAlgStd == "Priority") {
            Process pCopy = p;
            pCopy.arrivalTime = 0;
            sched->add_process(pCopy);
        } else {
            sched->add_process(p);
        }
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
    if (loadedActions.empty()) {
        QMessageBox::warning(this, "Error", "No se han cargado acciones.");
        return;
    }
    syncCanvas->reset();
    syncTick = 0;
    startedLog[0] = startedLog[1] = false;
    finishedLog[0] = finishedLog[1] = false;
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
        std::string pid;
        if (item.name.startsWith("Priority")) {
            pid = item.sched->schedule_next(simulationTick); 
        } else {
            pid = item.sched->schedule_next(simulationTick);
        }

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
            double avgWT;
            if (item.name.startsWith("Priority")) {
                avgWT = item.sched->average_waiting_time();
            } else {
                avgWT = averageWaitingTime(item.finishLog, loadedProcesses);
            }
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
