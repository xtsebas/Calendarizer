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
    std::string get_name() const override;
    std::vector<Process> get_pending_processes() const override;
    double average_waiting_time() const override;

private:
    std::vector<Process> sorted_processes;
    std::unordered_map<std::string, int> waiting_times;

    int current_time;
    int current_index;
    int remaining_time;
    bool executing;
    Process current_process;
};

#endif // PRIORITY_SCHEDULER_H
