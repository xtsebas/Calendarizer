#include "scheduler.h"
using namespace std;

Scheduler::Scheduler() {}

Scheduler::~Scheduler() {}

bool Scheduler::higher_priority(const Process& a, const Process& b) {
    return a.priority < b.priority;
}
