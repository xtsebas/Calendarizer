#include "process/process.h"
#include "utils/file_loader.h"
#include "utils/metrics.h"
#include "tests/dummy_scheduler.h"
#include "scheduler/SJF/sjf_scheduler.h"
#include "scheduler/RoundRobin/rr_scheduler.h"
#include "scheduler/Priority/priority_scheduler.h"
#include "synchronizer/synchronizer_peterson.h"
#include "synchronizer/mutex_sync.h"
#include "scheduler/FIFO/fifo_scheduler.h"

#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <algorithm>

using namespace std;

// --- Peterson Synchronizer test ---
int counter = 0;
PetersonSynchronizer sync;
void task(int id) {
    for (int i = 0; i < 10000; ++i) {
        sync.lock(id);
        ++counter;
        sync.unlock(id);
    }
}

int main() {
    try {
        // 1) Carga de procesos
        auto procesos = FileLoader::loadProcesses("data/processes_valid.txt");
        cout << "Cargados " << procesos.size() << " procesos:\n";
        for (const auto& p : procesos) {
            cout << "  PID=" << p.pid
                 << ", AT=" << p.arrivalTime
                 << ", BT=" << p.burstTime
                 << ", PRIO=" << p.priority << "\n";
        }

        // 2) Carga de recursos
        auto recursos = FileLoader::loadResources("data/resources.txt");
        cout << "Cargados " << recursos.size() << " recursos:\n";
        for (const auto& r : recursos) {
            cout << "  NAME=" << r.name
                 << ", COUNT=" << r.count << "\n";
        }

        // 3) Carga de acciones
        auto acciones = FileLoader::loadActions("data/actions.txt");
        cout << "Cargados " << acciones.size() << " acciones:\n";
        for (const auto& a : acciones) {
            cout << "  PID=" << a.pid
                 << ", type=" << a.type
                 << ", resource=" << a.resource
                 << ", cycle=" << a.cycle << "\n";
        }

        // 4) Prueba de PetersonSynchronizer
        cout << "\n=== Probando PetersonSynchronizer ===\n";
        counter = 0;
        thread t1(task, 0), t2(task, 1);
        t1.join();
        t2.join();
        cout << "Contador final (esperado = 20000): " << counter << "\n";

        // 5) Prueba de MutexSynchronizer
        cout << "\n=== Probando MutexSynchronizer ===\n";
        counter = 0;
        MutexSynchronizer mtx_sync;

        auto task_mutex = [&](int id) {
            for (int i = 0; i < 10000; ++i) {
                mtx_sync.lock(id);
                ++counter;
                mtx_sync.unlock(id);
            }
        };

        thread m1(task_mutex, 0), m2(task_mutex, 1);
        m1.join();
        m2.join();
        cout << "Contador final (esperado = 20000): " << counter << "\n";

        // 6) DummyScheduler
        DummyScheduler dummy;
        for (const auto& p : procesos) dummy.add_process(p);
        cout << "\n=== Probando " << dummy.get_name() << " ===\n";
        cout << "schedule_next(0): " << dummy.schedule_next(0) << "\n";

        // 7) SJF Scheduler
        SJF_Scheduler sjf;
        for (const auto& p : procesos) sjf.add_process(p);

        vector<pair<string,int>> finishSJF;
        cout << "\n=== Probando " << sjf.get_name() << " ===\n";
        int time = 0;
        while (true) {
            auto pid = sjf.schedule_next(time);
            if (!pid.empty()) {
                cout << "t=" << time << " -> " << pid << "\n";
                // Si acabó, registrar finish time
                auto pending = sjf.get_pending_processes();
                if (none_of(pending.begin(), pending.end(),
                            [&](auto& pr){ return pr.pid == pid; }))
                {
                    finishSJF.emplace_back(pid, time + 1);
                }
            }
            if (sjf.get_pending_processes().empty()) break;
            ++time;
        }
        cout << "Avg WT (SJF): "
             << averageWaitingTime(finishSJF, procesos) << "\n";

        // 8) Round Robin Scheduler (quantum = 3)
        RR_Scheduler rr(3);
        for (const auto& p : procesos) rr.add_process(p);

        vector<pair<string,int>> finishRR;
        cout << "\n=== Probando " << rr.get_name()
             << " (quantum=3) ===\n";
        time = 0;
        while (true) {
            auto pid = rr.schedule_next(time);
            if (!pid.empty()) {
                cout << "t=" << time << " -> " << pid << "\n";
                auto pending = rr.get_pending_processes();
                if (none_of(pending.begin(), pending.end(),
                            [&](auto& pr){ return pr.pid == pid; }))
                {
                    finishRR.emplace_back(pid, time + 1);
                }
            }
            if (rr.get_pending_processes().empty()) break;
            ++time;
        }
        cout << "Avg WT (RR):  "
             << averageWaitingTime(finishRR, procesos) << "\n";

        // 9) Priority Scheduler
        PriorityScheduler prio;
        for (const auto& p : procesos) prio.add_process(p);

        vector<pair<string,int>> finishPR;
        cout << "\n=== Probando " << prio.get_name() << " ===\n";
        time = 0;
        while (true) {
            auto pid = prio.schedule_next(time);
            if (!pid.empty()) {
                cout << "t=" << time << " -> " << pid << "\n";
                auto pending = prio.get_pending_processes();
                if (none_of(pending.begin(), pending.end(),
                            [&](auto& pr){ return pr.pid == pid; }))
                {
                    finishPR.emplace_back(pid, time + 1);
                }
            }
            if (prio.get_pending_processes().empty()) break;
            ++time;
        }
        // Usamos el cálculo externo de metrics.py para WT real:
        cout << "Avg WT (Priority): "
             << averageWaitingTime(finishPR, procesos) << "\n";

        // 10) FIFO Scheduler
        FIFO_Scheduler fifo;
        for (const auto& p : procesos) fifo.add_process(p);

        vector<pair<string,int>> finishFIFO;
        cout << "\n=== Probando " << fifo.get_name() << " ===\n";
        time = 0;
        while (true) {
            auto pid = fifo.schedule_next(time);
            if (!pid.empty()) {
                cout << "t=" << time << " -> " << pid << "\n";
                auto pending = fifo.get_pending_processes();
                if (none_of(pending.begin(), pending.end(),
                            [&](const Process& pr){ return pr.pid == pid; }))
                {
                    finishFIFO.emplace_back(pid, time + 1);
                }
            }
            if (fifo.get_pending_processes().empty()) break;
            ++time;
        }
        cout << "Avg WT (FIFO): "
             << averageWaitingTime(finishFIFO, procesos) << "\n";

    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
