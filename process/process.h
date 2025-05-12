#ifndef PROCESS_H
#define PROCESS_H

#include <string>

struct Process {
  std::string pid;
  int arrivalTime;
  int burstTime;
  int priority;

  Process(const std::string& pid_, int at, int bt, int prio);

  bool operator<(const Process& other) const;
};

#endif 
