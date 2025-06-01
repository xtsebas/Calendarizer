#ifndef SRTF_SCHEDULER_H
#define SRTF_SCHEDULER_H

#include "../scheduler.h"
#include <vector>

class SRTF_Scheduler : public Scheduler {
public:
    SRTF_Scheduler();
    ~SRTF_Scheduler() override = default;

    void add_process(const Process& process) override;
    std::string schedule_next(int current_time) override;
    std::string get_name() const override;
    std::vector<Process> get_pending_processes() const override;

    // <-- Implementamos el método puro para no ser abstractos
    double average_waiting_time() const override {
        return 0.0;
    }

private:
    struct Job {
        Process proc;
        int remainingTime;
        bool finished;
    };

    std::vector<Job> jobs_;
    int currentJobIndex_;
};

#endif // SRTF_SCHEDULER_H
