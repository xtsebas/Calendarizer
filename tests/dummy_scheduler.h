#ifndef DUMMY_SCHEDULER_H
#define DUMMY_SCHEDULER_H

#include "../scheduler/scheduler.h"

class DummyScheduler : public Scheduler {
private:
    std::vector<Process> procesos;

public:
    void add_process(const Process& process) override {
        procesos.push_back(process);
    }

    std::string schedule_next(int current_time) override {
        return procesos.empty() ? "" : procesos[0].pid; // solo muestra el primero
    }

    std::string get_name() const override {
        return "DummyScheduler";
    }

    std::vector<Process> get_pending_processes() const override {
        return procesos;
    }
};

#endif
