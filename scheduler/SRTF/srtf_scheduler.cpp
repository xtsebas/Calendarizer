// File: scheduler/SRTF/srtf_scheduler.cpp

#include "srtf_scheduler.h"
#include <limits>
#include <algorithm>

// Constructor: inicializamos currentJobIndex_ a -1 y nada más
SRTF_Scheduler::SRTF_Scheduler()
    : currentJobIndex_(-1)
{}

// Agrega un nuevo proceso al vector interno, con su ráfaga completa
void SRTF_Scheduler::add_process(const Process& process) {
    jobs_.push_back(Job{ process, process.burstTime, false });
}

// Cada tick elegimos, entre los que han llegado y no han terminado, el que tenga menor remainingTime
std::string SRTF_Scheduler::schedule_next(int current_time) {
    // 1) Si había un job en curso (currentJobIndex_) y aún le queda remanente,
    //    podríamos preemptarlo si aparece uno con remainingTime más corto. Sin embargo,
    //    este bucle ya busca el mejor job en cada invocación, así que podemos
    //    ignorar la noción de “mantener el mismo job” y siempre recalcular.

    int bestIdx = -1;
    int bestRem = std::numeric_limits<int>::max();

    // 2) Buscar, entre los jobs no finalizados y que ya hayan llegado, el de menor.remainingTime
    for (int i = 0; i < (int)jobs_.size(); ++i) {
        auto& job = jobs_[i];
        if (!job.finished
            && job.proc.arrivalTime <= current_time
            && job.remainingTime < bestRem)
        {
            bestRem = job.remainingTime;
            bestIdx = i;
        }
    }

    // 3) Si no hay ninguno listo, la CPU está idle
    if (bestIdx < 0) {
        return "";
    }

    // 4) Ejecutamos 1 unidad de tiempo sobre ese job
    Job& curJob = jobs_[bestIdx];
    curJob.remainingTime--;
    std::string pid = curJob.proc.pid;

    // 5) Si terminó, marcamos finished
    if (curJob.remainingTime == 0) {
        curJob.finished = true;
    }

    return pid;
}

// Nombre del algoritmo
std::string SRTF_Scheduler::get_name() const {
    return "SRTF";
}

// Devuelve todos los procesos (no sólo los que han llegado) que aún no finalizaron
std::vector<Process> SRTF_Scheduler::get_pending_processes() const {
    std::vector<Process> out;
    for (const auto& job : jobs_) {
        if (!job.finished) {
            out.push_back(job.proc);
        }
    }
    return out;
}
