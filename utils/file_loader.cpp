#include "file_loader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace FileLoader {

  // Carga procesos desde un archivo
  // Formato esperado: <PID>,<BT>,<AT>,<Priority>
  // Donde:
  // - PID: Identificador del proceso
  // - BT: Burst Time (tiempo de ejecución)
  // - AT: Arrival Time (tiempo de llegada)
  // - Priority: Prioridad del proceso (menor número = mayor prioridad)
  vector<Process> loadProcesses(const string& filepath) {
    ifstream in(filepath);
    if (!in.is_open())
      throw runtime_error("No se pudo abrir archivo: " + filepath);

    vector<Process> processes;
    string line;
    int lineNum = 0;

    while (getline(in, line)) {
      ++lineNum;
      if (line.empty()) continue;

      istringstream ss(line);
      string pid;
      int bt, at, prio;  // Changed order to match <PID>, <BT>, <AT>, <Priority>

      if (!getline(ss, pid, ',') ||
          !(ss >> bt) || ss.get() != ',' ||  // Read burst time first
          !(ss >> at) || ss.get() != ',' ||  // Then read arrival time
          !(ss >> prio)) {
        throw runtime_error(
          "Formato inválido en línea " + to_string(lineNum) +
          " de " + filepath + ": '" + line + "'");
      }

      processes.emplace_back(pid, at, bt, prio);  // Still passing (pid, at, bt, prio) to match constructor
    }

    return processes;
  }

  vector<Resource> loadResources(const string& filepath) {
    ifstream in(filepath);
    if (!in.is_open())
      throw runtime_error("No se pudo abrir archivo: " + filepath);

    vector<Resource> resources;
    string line;
    int lineNum = 0;

    while (getline(in, line)) {
      ++lineNum;
      if (line.empty()) continue;

      istringstream ss(line);
      string name;
      int count;

      if (!getline(ss, name, ',') || !(ss >> count)) {
        throw runtime_error(
          "Formato inválido en línea " + to_string(lineNum) +
          " de " + filepath + ": '" + line + "'");
      }

      resources.emplace_back(name, count);
    }

    return resources;
  }

  vector<Action> loadActions(const string& filepath) {
    ifstream in(filepath);
    if (!in.is_open())
      throw runtime_error("No se pudo abrir archivo: " + filepath);

    vector<Action> actions;
    string line;
    int lineNum = 0;

    while (getline(in, line)) {
      ++lineNum;
      if (line.empty()) continue;

      istringstream ss(line);
      string pid, type, resource;
      int cycle;

      if (!getline(ss, pid, ',') ||
          !getline(ss, type, ',') ||
          !getline(ss, resource, ',') ||
          !(ss >> cycle)) {
        throw runtime_error(
          "Formato inválido en línea " + to_string(lineNum) +
          " de " + filepath + ": '" + line + "'");
      }

      actions.emplace_back(pid, type, resource, cycle);
    }

    return actions;
  }

}