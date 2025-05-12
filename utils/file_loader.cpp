#include "file_loader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace FileLoader {

std::vector<Process> loadProcesses(const std::string& filepath) {
  std::ifstream in(filepath);
  if (!in.is_open())
    throw std::runtime_error("No se pudo abrir archivo: " + filepath);

  std::vector<Process> processes;
  std::string line;
  int lineNum = 0;

  while (std::getline(in, line)) {
    ++lineNum;
    if (line.empty()) continue; 

    std::istringstream ss(line);
    std::string pid;
    int at, bt, prio;

    if (!std::getline(ss, pid, ',') ||
        !(ss >> at) || ss.get() != ',' ||
        !(ss >> bt) || ss.get() != ',' ||
        !(ss >> prio)) {
      throw std::runtime_error(
        "Formato inválido en línea " + std::to_string(lineNum) +
        " de " + filepath + ": '" + line + "'");
    }

    processes.emplace_back(pid, at, bt, prio);
  }

  return processes;
}

}