// main.cpp

#include "process/process.h"
#include "utils/file_loader.h"
#include "utils/metrics.h"
#include "tests/dummy_scheduler.h"
#include "scheduler/SJF/sjf_scheduler.h"
#include "scheduler/RoundRobin/rr_scheduler.h"
#include "scheduler/Priority/priority_scheduler.h"
#include "synchronizer/synchronizer_peterson.h"

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
        auto procs = FileLoader::loadProcesses("data/processes_valid.txt");
        cout << "Cargados " << procs.size() << " procesos:\n";
        for (const auto& p : procs) {
            cout << "  PID=" << p.pid
                 << ", AT=" << p.arrivalTime
                 << ", BT=" << p.burstTime
                 << ", PRIO=" << p.priority << "\n";
        }

        // 2) Prueba de PetersonSynchronizer
        cout << "\n=== Probando PetersonSynchronizer ===\n";
        counter = 0;
        thread t1(task, 0), t2(task, 1);
        t1.join();
        t2.join();
        cout << "Contador final (esperado = 20000): " << counter << "\n";

        // 3) DummyScheduler
        DummyScheduler dummy;
        for (const auto& p : procs) dummy.add_process(p);
        cout << "\n=== Probando " << dummy.get_name() << " ===\n";
        cout << "schedule_next(0): " << dummy.schedule_next(0) << "\n";

        // 4) SJF Scheduler
        SJF_Scheduler sjf;
        for (const auto& p : procs) sjf.add_process(p);

        vector<pair<string,int>> finishSJF;
        cout << "\n=== Probando " << sjf.get_name() << " ===\n";
        int time = 0;
        while (true) {
            auto pid = sjf.schedule_next(time);
            if (!pid.empty()) {
                cout << "t=" << time << " -> " << pid << "\n";
                // si acabó, registrar finish time
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
             << averageWaitingTime(finishSJF, procs) << "\n";

        // 5) Round Robin Scheduler (quantum = 3)
        RR_Scheduler rr(3);
        for (const auto& p : procs) rr.add_process(p);

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
             << averageWaitingTime(finishRR, procs) << "\n";

        // 6) Priority Scheduler
        PriorityScheduler prio;
        for (const auto& p : procs) prio.add_process(p);

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
        cout << "Avg WT (Priority): " 
             << prio.average_waiting_time() << "\n";

    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
