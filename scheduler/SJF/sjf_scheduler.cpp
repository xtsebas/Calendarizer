#include "sjf_scheduler.h"
#include <limits>
#include <algorithm>

SJF_Scheduler::SJF_Scheduler()
  : currentJobIndex_(-1)
{}

void SJF_Scheduler::add_process(const Process& process) {
    jobs_.push_back(Job{ process, process.burstTime, false });
}

std::string SJF_Scheduler::schedule_next(int current_time) {
    // Si no hay trabajo en curso, elegir el siguiente
    if (currentJobIndex_ < 0) {
        int bestIdx = -1;
        int bestRem = std::numeric_limits<int>::max();
        for (int i = 0; i < (int)jobs_.size(); ++i) {
            auto& job = jobs_[i];
            if (!job.finished
                && job.proc.arrivalTime <= current_time
                && job.remainingTime < bestRem)
            {
                bestRem = job.remainingTime;
                bestIdx = i;
            }
        }
        if (bestIdx >= 0) {
            currentJobIndex_ = bestIdx;
        } else {
            // no hay proceso listo
            return "";
        }
    }

    // Ejecutar 1 unidad de tiempo
    auto& cur = jobs_[currentJobIndex_];
    cur.remainingTime--;
    std::string pid = cur.proc.pid;

    // Si terminó, marcar y desmontar CPU
    if (cur.remainingTime == 0) {
        cur.finished = true;
        currentJobIndex_ = -1;
    }

    return pid;
}

std::string SJF_Scheduler::get_name() const {
    return "SJF";
}

std::vector<Process> SJF_Scheduler::get_pending_processes() const {
    std::vector<Process> out;
    for (auto& job : jobs_) {
        if (!job.finished) out.push_back(job.proc);
    }
    return out;
}
