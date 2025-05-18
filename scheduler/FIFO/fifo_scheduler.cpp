#include "fifo_scheduler.h"

FIFO_Scheduler::FIFO_Scheduler()
  : current_job_(-1)
{}

void FIFO_Scheduler::add_process(const Process& process) {
    processes_.push_back(process);
    remaining_times_.push_back(process.burstTime);
    enqueued_.push_back(false);
}

std::string FIFO_Scheduler::schedule_next(int current_time) {
    for (size_t i = 0; i < processes_.size(); ++i) {
        if (!enqueued_[i]
            && processes_[i].arrivalTime <= current_time
            && remaining_times_[i] > 0)
        {
            ready_queue_.push(i);
            enqueued_[i] = true;
        }
    }

    if (current_job_ < 0) {
        if (ready_queue_.empty()) {
            return "";  // CPU idle
        }
        current_job_ = ready_queue_.front();
        ready_queue_.pop();
    }

    --remaining_times_[current_job_];
    std::string pid = processes_[current_job_].pid;

    if (remaining_times_[current_job_] == 0) {
        current_job_ = -1;
    }

    return pid;
}

std::string FIFO_Scheduler::get_name() const {
    return "FIFO";
}

std::vector<Process> FIFO_Scheduler::get_pending_processes() const {
    std::vector<Process> pending;
    for (size_t i = 0; i < processes_.size(); ++i) {
        if (remaining_times_[i] > 0) {
            pending.push_back(processes_[i]);
        }
    }
    return pending;
}
