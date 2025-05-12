#ifndef FILE_LOADER_H
#define FILE_LOADER_H

#include <string>
#include <vector>
#include "../process/process.h"

namespace FileLoader {

  std::vector<Process> loadProcesses(const std::string& filepath);

}

#endif
