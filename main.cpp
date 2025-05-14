#include "process/process.h"
#include "utils/file_loader.h"
#include "tests/dummy_scheduler.h"
#include "scheduler/SJF/sjf_scheduler.h"
#include "scheduler/RoundRobin/rr_scheduler.h"
#include "scheduler/Priority/priority_scheduler.h"
#include "synchronizer/synchronizer_peterson.h"

#include <iostream>
#include <vector>
#include <thread>
#include <string>

using namespace std;

// --- synchronizer peterson ---
int counter = 0;
PetersonSynchronizer sync;

void task(int id) {
    for (int i = 0; i < 10000; ++i) {
        sync.lock(id);
        counter++;
        sync.unlock(id);
    }
}

int main() {
    try {
        // Carga de procesos
        auto procs = FileLoader::loadProcesses("data/processes_valid.txt");
        cout << "Cargados " << procs.size() << " procesos:\n";
        for (const auto& p : procs) {
            cout << "  PID=" << p.pid
                 << ", AT=" << p.arrivalTime
                 << ", BT=" << p.burstTime
                 << ", PRIO=" << p.priority << "\n";
        }

        // --- Prueba de PetersonSynchronizer ---
        cout << "\n=== Probando PetersonSynchronizer ===\n";
        counter = 0; 

        thread t1(task, 0);
        thread t2(task, 1);

        t1.join();
        t2.join();

        cout << "Contador final (esperado = 20000): " << counter << "\n";

        // --- DummyScheduler ---
        DummyScheduler dummy;
        for (const auto& p : procs)
            dummy.add_process(p);

        cout << "\n=== Probando " << dummy.get_name() << " ===\n";
        cout << "schedule_next(0): " << dummy.schedule_next(0) << "\n";

        // --- SJF Scheduler ---
        SJF_Scheduler sjf;
        for (const auto& p : procs)
            sjf.add_process(p);

        cout << "\n=== Probando " << sjf.get_name() << " ===\n";
        // simulamos 10 ciclos (ajusta según tu dataset)
        for (int t = 0; t < 10; ++t) {
            auto pid = sjf.schedule_next(t);
            if (!pid.empty())
                cout << "t=" << t << " -> " << pid << "\n";
        }

        // --- Round Robin Scheduler (quantum = 3) ---
        RR_Scheduler rr(3);
        for (const auto& p : procs)
            rr.add_process(p);

        cout << "\n=== Probando " << rr.get_name() << " (quantum=3) ===\n";
        for (int t = 0; t < 10; ++t) {
            auto pid = rr.schedule_next(t);
            if (!pid.empty())
                cout << "t=" << t << " -> " << pid << "\n";
        }

        // --- Priority Scheduler ---
        PriorityScheduler priority;
        for (const auto& p : procs)
            priority.add_process(p);

        cout << "\n=== Probando " << priority.get_name() << " ===\n";
        int t = 0;
        while (true) {
            auto pid = priority.schedule_next(t);
            if (!pid.empty())
                cout << "t=" << t << " -> " << pid << "\n";

            // Salimos si ya no hay procesos ejecutándose ni esperando
            if (priority.get_pending_processes().empty() && pid.empty())
                break;

            ++t;
        }
        cout << "Average Waiting Time: " << priority.average_waiting_time() << "\n";

    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
