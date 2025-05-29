#include "priority_scheduler.h"
#include <algorithm>

PriorityScheduler::PriorityScheduler()
    : current_time(0), current_index(0),
    remaining_time(0), executing(false)
{}

void PriorityScheduler::add_process(const Process& process) {
    sorted_processes.push_back(process);
}

std::string PriorityScheduler::schedule_next(int t) {
    current_time = t;

    if (current_index == 0 && t == 0) {
        std::sort(sorted_processes.begin(), sorted_processes.end(), [](const Process& a, const Process& b) {
            return a.priority < b.priority; // menor número = más prioritario
        });
    }

    if (!executing && current_index < sorted_processes.size()) {
        current_process = sorted_processes[current_index];
        executing = true;
        remaining_time = current_process.burstTime;

        // tiempo de espera = tiempo actual - tiempo de llegada (simulado como 0)
        int accumulated = 0;
        for (size_t i = 0; i < current_index; ++i) {
            accumulated += sorted_processes[i].burstTime;
        }
        waiting_times[current_process.pid] = accumulated;
    }

    if (!executing) return "";

    std::string pid = current_process.pid;
    remaining_time--;

    if (remaining_time == 0) {
        executing = false;
        current_index++;
    }

    return pid;
}

std::string PriorityScheduler::get_name() const {
    return "Priority (sin arrival)";
}

std::vector<Process> PriorityScheduler::get_pending_processes() const {
    std::vector<Process> pending;
    for (int i = current_index; i < sorted_processes.size(); ++i)
        pending.push_back(sorted_processes[i]);
    return pending;
}

double PriorityScheduler::average_waiting_time() const {
    if (waiting_times.empty()) return 0.0;
    double total = 0;
    for (const auto& [_, wt] : waiting_times)
        total += wt;
    return total / waiting_times.size();
}
