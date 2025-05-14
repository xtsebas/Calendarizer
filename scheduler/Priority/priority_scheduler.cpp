#include "priority_scheduler.h"
#include "../scheduler.h"

PriorityScheduler::PriorityScheduler()
    : ready_queue(Scheduler::higher_priority),
      remaining_time(0), current_time(0),
      current_process({ "", 0, 0, 0 }) {}

void PriorityScheduler::add_process(const Process& process) {
    ready_queue.push(process);
}

std::string PriorityScheduler::schedule_next(int t) {
    current_time = t;

    if (remaining_time > 0) {
        remaining_time--;
        return current_process.pid;
    }

    if (ready_queue.empty()) return "";

    current_process = ready_queue.top();
    ready_queue.pop();

    // Tiempo de espera = tiempo actual - 0 (inicio del sistema)
    waiting_times[current_process.pid] = current_time;

    remaining_time = current_process.burstTime - 1;  // ejecuta 1 ahora
    return current_process.pid;
}

std::string PriorityScheduler::get_name() const {
    return "Priority";
}

std::vector<Process> PriorityScheduler::get_pending_processes() const {
    std::vector<Process> pending;
    auto temp = ready_queue;
    while (!temp.empty()) {
        pending.push_back(temp.top());
        temp.pop();
    }
    return pending;
}

double PriorityScheduler::average_waiting_time() const {
    if (waiting_times.empty()) return 0.0;

    double total = 0;
    for (const auto& [pid, wait] : waiting_times) {
        total += wait;
    }
    return total / waiting_times.size();
}