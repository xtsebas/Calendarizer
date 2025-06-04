#include "priority_scheduler.h"
#include <limits>

PriorityScheduler::PriorityScheduler()
    : executing(false), currentJobIndex(-1)
{}

// Al agregar un proceso, lo convertimos en un Job con su burstTime inicial
void PriorityScheduler::add_process(const Process& process) {
    jobs.push_back(Job{ process, process.burstTime, false });
}

std::string PriorityScheduler::schedule_next(int current_time) {
    int bestIdx = -1;
    int bestPrio = std::numeric_limits<int>::max();

    for (int i = 0; i < (int)jobs.size(); ++i) {
        const Job& job = jobs[i];
        if (!job.finished && job.proc.priority < bestPrio) {
            bestPrio = job.proc.priority;
            bestIdx = i;
        }
    }

    if (bestIdx < 0) {
        executing = false;
        currentJobIndex = -1;
        return "";
    }

    Job& job = jobs[bestIdx];

    if (job.start_time == -1) {
        job.start_time = current_time;
    }

    job.remainingTime--;
    std::string pid = job.proc.pid;
    currentJobIndex = bestIdx;
    executing = true;

    if (job.remainingTime == 0) {
        job.finished = true;
        executing = false;
        int wt = std::max(0, job.start_time);
        waiting_times[pid] = wt;

    }

    return pid;
}



std::vector<Process> PriorityScheduler::get_pending_processes() const {
    std::vector<Process> pending;
    for (const auto &job : jobs) {
        if (!job.finished) {
            pending.push_back(job.proc);
        }
    }
    return pending;
}

// Calcula el promedio de WT guardados en el mapa
double PriorityScheduler::average_waiting_time() const {
    if (waiting_times.empty()) return 0.0;
    double total = 0;
    for (const auto &kv : waiting_times) {
        total += kv.second;
    }
    return total / waiting_times.size();
}
