#ifndef SJF_SCHEDULER_H
#define SJF_SCHEDULER_H

#include "../scheduler.h"
#include <vector>

class SJF_Scheduler : public Scheduler {
public:
    SJF_Scheduler();
    ~SJF_Scheduler() override = default;

    // Añade un nuevo proceso
    void add_process(const Process& process) override;

    // Ejecuta una unidad de tiempo y retorna el PID o "" si no hay nada
    std::string schedule_next(int current_time) override;

    // Nombre del algoritmo
    std::string get_name() const override;

    // Procesos aún no terminados
    std::vector<Process> get_pending_processes() const override;

    double average_waiting_time() const override {
        // Stub por ahora. Devuelve 0.0.
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

#endif
