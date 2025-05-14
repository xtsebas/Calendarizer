#ifndef PRIORITY_SCHEDULER_H
#define PRIORITY_SCHEDULER_H

#include "../scheduler.h"
#include <queue>
#include <map>

class PriorityScheduler : public Scheduler {
public:
    PriorityScheduler();

    void add_process(const Process& process) override;
    std::string schedule_next(int current_time) override;
    std::string get_name() const override;
    std::vector<Process> get_pending_processes() const override;

    double average_waiting_time() const;

private:
    std::priority_queue<Process, std::vector<Process>, bool(*)(const Process&, const Process&)> ready_queue;
    std::map<std::string, int> waiting_times;
    Process current_process;
    int remaining_time;
    int current_time;
};

#endif