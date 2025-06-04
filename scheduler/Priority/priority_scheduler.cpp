#include "priority_scheduler.h"
#include <limits>

PriorityScheduler::PriorityScheduler()
    : executing(false), currentJobIndex(-1)
{}

// Al agregar un proceso, lo convertimos en un Job con su burstTime inicial
void PriorityScheduler::add_process(const Process& process) {
    jobs.push_back(Job{ process, process.burstTime, false });
}

// Cada tick (current_time) devolvemos el PID que corre en esta unidad, o "" si CPU idle
std::string PriorityScheduler::schedule_next(int current_time) {
    // Caso A: Si ya hay un Job ejecutándose, continuar hasta que termine
    if (executing && currentJobIndex >= 0) {
        Job &job = jobs[currentJobIndex];
        job.remainingTime--;
        std::string pid = job.proc.pid;

        if (job.remainingTime == 0) {
            // El Job finaliza en current_time + 1
            job.finished = true;
            executing = false;
            int finishTime = current_time + 1;
            int wt = finishTime - job.proc.arrivalTime - job.proc.burstTime;
            if (wt < 0) wt = 0;  // Evitar negativo por precaución
            waiting_times[pid] = wt;
        }
        return pid;
    }

    // Caso B: No hay Job en ejecución, buscamos uno nuevo entre los que ya llegaron (arrivalTime <= current_time),
    //          tomando el de menor priority (menor número = más prioridad).
    int bestIdx = -1;
    int bestPrio = std::numeric_limits<int>::max();

    for (int i = 0; i < (int)jobs.size(); ++i) {
        const Job &job = jobs[i];
        if (!job.finished && job.proc.arrivalTime <= current_time) {
            if (job.proc.priority < bestPrio) {
                bestPrio = job.proc.priority;
                bestIdx = i;
            }
        }
    }

    if (bestIdx < 0) {
        // No hay procesos listos en este tick
        return "";
    }

    // Iniciar ejecución del Job seleccionado
    currentJobIndex = bestIdx;
    executing = true;
    jobs[bestIdx].remainingTime--;
    std::string pid = jobs[bestIdx].proc.pid;

    if (jobs[bestIdx].remainingTime == 0) {
        // Si su ráfaga fue de 1 unidad o restante era 1, entonces termina inmediatamente
        jobs[bestIdx].finished = true;
        executing = false;
        int finishTime = current_time + 1;
        int wt = finishTime - jobs[bestIdx].proc.arrivalTime - jobs[bestIdx].proc.burstTime;
        if (wt < 0) wt = 0;
        waiting_times[pid] = wt;
    }
    return pid;
}

std::vector<Process> PriorityScheduler::get_pending_processes() const {
    std::vector<Process> pending;
    for (const auto &job : jobs) {
        if (!job.finished) {
            pending.push_back(job.proc);
        }
    }
    return pending;
}

// Calcula el promedio de WT guardados en el mapa
double PriorityScheduler::average_waiting_time() const {
    if (waiting_times.empty()) return 0.0;
    double total = 0;
    for (const auto &kv : waiting_times) {
        total += kv.second;
    }
    return total / waiting_times.size();
}
