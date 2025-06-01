#ifndef RR_SCHEDULER_H
#define RR_SCHEDULER_H

#include "../scheduler.h"
#include <vector>
#include <queue>

class RR_Scheduler : public Scheduler {
public:
    explicit RR_Scheduler(int quantum);
    ~RR_Scheduler() override = default;

    void add_process(const Process& process) override;
    std::string schedule_next(int current_time) override;
    std::string get_name() const override;
    std::vector<Process> get_pending_processes() const override;
    double average_waiting_time() const override {
        return 0.0;
    }

private:
    struct Job {
        Process proc;
        int remainingTime;
        bool enqueued;  
    };

    int quantum_;
    int timeSliceRemaining_;
    int currentJobIndex_;     
    std::vector<Job> jobs_;
    std::queue<int> readyQueue_;
};

#endif
