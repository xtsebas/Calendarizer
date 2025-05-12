#include "rr_scheduler.h"
#include <stdexcept>

RR_Scheduler::RR_Scheduler(int quantum)
  : quantum_(quantum)
  , timeSliceRemaining_(0)
  , currentJobIndex_(-1)
{}

void RR_Scheduler::add_process(const Process& process) {
    jobs_.push_back(Job{ process, process.burstTime, false });
}

std::string RR_Scheduler::schedule_next(int current_time) {
    // 1) Encolar nuevas llegadas
    for (int i = 0; i < (int)jobs_.size(); ++i) {
        auto& job = jobs_[i];
        if (!job.enqueued
            && job.proc.arrivalTime <= current_time
            && job.remainingTime > 0)
        {
            readyQueue_.push(i);
            job.enqueued = true;
        }
    }

    // 2) Si no hay job en curso o se agotó quantum, obtener siguiente
    if (currentJobIndex_ < 0 || timeSliceRemaining_ == 0) {
        // Si el job previo sigue con remanente, reencolarlo
        if (currentJobIndex_ >= 0) {
            auto& prev = jobs_[currentJobIndex_];
            if (prev.remainingTime > 0)
                readyQueue_.push(currentJobIndex_);
        }
        // Tomar siguiente de la cola
        if (readyQueue_.empty()) {
            currentJobIndex_ = -1;
            return "";
        }
        currentJobIndex_   = readyQueue_.front();
        readyQueue_.pop();
        timeSliceRemaining_ = quantum_;
    }

    // 3) Ejecutar 1 unidad de tiempo
    auto& cur = jobs_[currentJobIndex_];
    cur.remainingTime--;
    timeSliceRemaining_--;
    std::string pid = cur.proc.pid;

    // 4) Si terminó, liberar CPU
    if (cur.remainingTime == 0) {
        currentJobIndex_ = -1;
    }

    return pid;
}

std::string RR_Scheduler::get_name() const {
    return "RoundRobin";
}

std::vector<Process> RR_Scheduler::get_pending_processes() const {
    std::vector<Process> out;
    for (auto& job : jobs_) {
        if (job.remainingTime > 0)
            out.push_back(job.proc);
    }
    return out;
}
