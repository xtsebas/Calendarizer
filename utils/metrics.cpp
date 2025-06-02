#include "metrics.h"
#include <numeric>
#include <stdexcept>
#include <unordered_map>

using namespace std;

double averageWaitingTime(
    const vector<pair<string,int>>& finishTimes,
    const vector<Process>& procs)
{
    // Mapea pid -> Process
    unordered_map<string, Process> mp;
    for (auto& p : procs) mp[p.pid] = p;

    vector<double> waitings;
    waitings.reserve(finishTimes.size());
    for (auto& [pid, t_finish] : finishTimes) {
        auto it = mp.find(pid);
        if (it == mp.end())
            throw runtime_error("PID desconocido en finishTimes: " + pid);
        const Process& p = it->second;
        // WT = finishTime - arrivalTime - burstTime
        double wt = static_cast<double>(t_finish - p.arrivalTime - p.burstTime);
        waitings.push_back(wt);
    }
    if (waitings.empty()) return 0.0;
    double sum = accumulate(waitings.begin(), waitings.end(), 0.0);
    return sum / waitings.size();
}

double averageTurnaroundTime(
    const vector<pair<string,int>>& finishTimes,
    const vector<Process>& procs)
{
    // Mapea pid -> Process
    unordered_map<string, Process> mp;
    for (auto& p : procs) mp[p.pid] = p;

    vector<double> tts;
    tts.reserve(finishTimes.size());
    for (auto& [pid, t_finish] : finishTimes) {
        auto it = mp.find(pid);
        if (it == mp.end())
            throw runtime_error("PID desconocido en finishTimes: " + pid);
        const Process& p = it->second;
        // TT = finishTime - arrivalTime
        double tt = static_cast<double>(t_finish - p.arrivalTime);
        tts.push_back(tt);
    }
    if (tts.empty()) return 0.0;
    double sum = accumulate(tts.begin(), tts.end(), 0.0);
    return sum / tts.size();
}
