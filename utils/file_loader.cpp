#include "file_loader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace FileLoader {

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
    int at, bt, prio;

    if (!getline(ss, pid, ',') ||
        !(ss >> at) || ss.get() != ',' ||
        !(ss >> bt) || ss.get() != ',' ||
        !(ss >> prio)) {
      throw runtime_error(
        "Formato inválido en línea " + to_string(lineNum) +
        " de " + filepath + ": '" + line + "'");
    }

    processes.emplace_back(pid, at, bt, prio);
  }

  return processes;
}

}