 #ifndef METRICS_H
 #define METRICS_H

#include <vector>
#include <string>
#include "../process/process.h"

// Recopila una lista de <pid, finishTime> en el map y
// calcula WT = finishTime - arrivalTime - burstTime
double averageWaitingTime(const std::vector<std::pair<std::string,int>>& finishTimes, const std::vector<Process>& procs);

 #endif