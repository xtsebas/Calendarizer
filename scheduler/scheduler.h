#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <queue>
#include <string>

struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;
    bool is_io_bound;

    Process(int p, int at, int bt, int prio = 0, bool io = false)
        : pid(p), arrival_time(at), burst_time(bt), remaining_time(bt),
          priority(prio), is_io_bound(io) {}
};

class Scheduler {
public:
    Scheduler() {}
    virtual ~Scheduler() {}

    // Carga inicial de procesos
    virtual void add_process(const Process& process) = 0;

    // Ejecuta una unidad de tiempo de CPU, retorna el PID del proceso ejecutado o -1 si no hay ninguno
    virtual int schedule_next(int current_time) = 0;

    // Devuelve el nombre del algoritmo (útil para testing o visualización)
    virtual std::string get_name() const = 0;

    // Permite obtener los procesos restantes (para métricas o debugging)
    virtual std::vector<Process> get_pending_processes() const = 0;

protected:
    // Puedes definir criterios comunes aquí, como prioridad de CPU-bound sobre I/O-bound
    static bool prefer_cpu_bound(const Process& a, const Process& b) {
        return !a.is_io_bound && b.is_io_bound;
    }
};

#endif // SCHEDULER_H