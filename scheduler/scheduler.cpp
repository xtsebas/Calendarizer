#include "scheduler.h"
using namespace std;

Scheduler::Scheduler() {}

Scheduler::~Scheduler() {}

bool Scheduler::higher_priority(const Process& a, const Process& b) {
    // Menor número = mayor prioridad
    if (a.priority == b.priority)
        return a.arrivalTime > b.arrivalTime; // Si prioridades iguales, el que llegó antes va primero
    return a.priority > b.priority;
}
