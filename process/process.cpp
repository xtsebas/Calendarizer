#include "process.h"

using namespace std;
// Constructor
Process::Process(const string& pid_, int at, int bt, int prio)
  : pid(pid_), arrivalTime(at), burstTime(bt), priority(prio)
{}

// Por defecto ordena por arrivalTime, luego pid
bool Process::operator<(const Process& other) const {
  if (arrivalTime != other.arrivalTime)
    return arrivalTime < other.arrivalTime;
  return pid < other.pid;
}
