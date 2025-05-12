
// main.cpp
#include "process/process.h"
#include "utils/file_loader.h"
#include <iostream>

using namespace std;

int main() {
    try {

        auto procs = FileLoader::loadProcesses("data/processes_valid.txt");
        cout << "Cargados " << procs.size() << " procesos:\n";
        for (auto& p : procs) {
            cout
              << "  PID=" << p.pid
              << ", AT=" << p.arrivalTime
              << ", BT=" << p.burstTime 
              << ", PRIO=" << p.priority 
              << "\n";
        }
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
