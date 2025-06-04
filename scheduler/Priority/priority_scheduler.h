#ifndef PRIORITY_SCHEDULER_H
#define PRIORITY_SCHEDULER_H

#include "../scheduler.h"
#include <vector>
#include <unordered_map>

class PriorityScheduler : public Scheduler {
public:
    PriorityScheduler();

    void add_process(const Process& process) override;
    std::string schedule_next(int current_time) override;
    std::string get_name() const override { return "Priority"; }
    std::vector<Process> get_pending_processes() const override;
    double average_waiting_time() const override;

private:
    struct Job {
        Process proc;
        int remainingTime;
        bool finished;
    };

    std::vector<Job> jobs;                         // Lista interna de trabajos
    bool executing = false;                        // Indica si hay un Job en curso
    int currentJobIndex = -1;                      // Índice del Job actual

    // Aquí guardaremos el waiting time (WT) cuando un Job termine:
    std::unordered_map<std::string, int> waiting_times;
};

#endif // PRIORITY_SCHEDULER_H
