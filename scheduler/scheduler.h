#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <string>
#include "../process/process.h"

class Scheduler {
public:
    Scheduler();
    virtual ~Scheduler();

    // Añade un nuevo proceso al scheduler
    virtual void add_process(const Process& process) = 0;

    // Ejecuta una unidad de tiempo y retorna el PID ejecutado o "" si no hay ninguno
    virtual std::string schedule_next(int current_time) = 0;

    // Nombre del algoritmo
    virtual std::string get_name() const = 0;

    // Procesos pendientes
    virtual std::vector<Process> get_pending_processes() const = 0;

    virtual double average_waiting_time() const = 0;


protected:
    // Preferencia: CPU-bound vs I/O-bound no se incluye aquí (solo info básica disponible en Process)
    static bool higher_priority(const Process& a, const Process& b);
};

#endif // SCHEDULER_H
