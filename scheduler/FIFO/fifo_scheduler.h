#ifndef FIFO_SCHEDULER_H
#define FIFO_SCHEDULER_H

#include "../scheduler.h"
#include <vector>
#include <queue>
#include <string>

class FIFO_Scheduler : public Scheduler {
public:
    FIFO_Scheduler();
    ~FIFO_Scheduler() override = default;

    void add_process(const Process& process) override;
    std::string schedule_next(int current_time) override;
    std::string get_name() const override;
    std::vector<Process> get_pending_processes() const override;
    double average_waiting_time() const override {
        return 0.0;
    }

private:
    std::vector<Process> processes_;
    std::vector<int>     remaining_times_;
    std::vector<bool>    enqueued_;
    std::queue<int>      ready_queue_;
    int                  current_job_;
};

#endif
