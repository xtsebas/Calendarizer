#include "process/process.h"
#include "utils/file_loader.h"
#include "tests/dummy_scheduler.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main() {
    try {
        auto procs = FileLoader::loadProcesses("data/processes_valid.txt");
        cout << "Cargados " << procs.size() << " procesos:\n";
        for (const auto& p : procs) {
            cout << "PID=" << p.pid
                 << ", AT=" << p.arrivalTime
                 << ", BT=" << p.burstTime
                 << ", PRIO=" << p.priority << "\n";
        }

        DummyScheduler scheduler;

        for (const auto& p : procs)
            scheduler.add_process(p);

        cout << "\nProbando clase base Scheduler con: " << scheduler.get_name() << "\n";
        cout << "schedule_next(0) devuelve: " << scheduler.schedule_next(0) << "\n";

    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
