#include "priority_scheduler.h"
#include <algorithm>
#include <stdexcept>       // por si queremos lanzar errores
#include <unordered_map>

PriorityScheduler::PriorityScheduler()
    : current_time(0), current_index(0),
      remaining_time(0), executing(false)
{}

void PriorityScheduler::add_process(const Process& process) {
    sorted_processes.push_back(process);
}

std::string PriorityScheduler::schedule_next(int t) {
    current_time = t;

    // En la primera llamada, ordenamos según prioridad (más bajo == más prioritario).
    // No consideramos arrivalTime en la ordenación: 
    // aquí asumimos que "enteros" ya están prefiltrados para llegar en t=0 o similares.
    if (current_index == 0 && t == 0) {
        std::sort(sorted_processes.begin(), sorted_processes.end(),
                  [](const Process& a, const Process& b) {
                      return a.priority < b.priority;
                  });
    }

    // Si no estamos ejecutando ninguno y aún quedan por arrancar:
    if (!executing && current_index < (int)sorted_processes.size()) {
        // Tomamos el proceso actual
        current_process = sorted_processes[current_index];
        executing = true;
        remaining_time = current_process.burstTime;
    }

    if (!executing) {
        // No hay proceso por correr en este tick
        return "";
    }

    // Ejecutamos una unidad de tiempo:
    std::string pid = current_process.pid;
    remaining_time--;

    // Si acabó su ráfaga, calculamos su WT real y avanzamos al siguiente:
    if (remaining_time == 0) {
        // Finish time = current_time + 1
        int finishTime = current_time + 1;
        // WT = finishTime - arrivalTime - burstTime
        int wt = finishTime - current_process.arrivalTime - current_process.burstTime;
        if (wt < 0) 
            wt = 0; // por seguridad, en caso de llegada tardía
        
        waiting_times[current_process.pid] = wt;

        // Marcamos que ya no se está ejecutando ninguno, y pasamos al siguiente
        executing = false;
        current_index++;
    }

    return pid;
}

std::string PriorityScheduler::get_name() const {
    return "Priority";
}

std::vector<Process> PriorityScheduler::get_pending_processes() const {
    std::vector<Process> pending;
    for (int i = current_index; i < (int)sorted_processes.size(); ++i) {
        pending.push_back(sorted_processes[i]);
    }
    return pending;
}

double PriorityScheduler::average_waiting_time() const {
    if (waiting_times.empty()) return 0.0;
    double total = 0;
    for (const auto& kv : waiting_times) {
        total += kv.second;
    }
    return total / waiting_times.size();
}
