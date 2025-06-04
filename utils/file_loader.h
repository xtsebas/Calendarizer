#ifndef FILE_LOADER_H
#define FILE_LOADER_H

#include <string>
#include <vector>
#include "../process/process.h"

namespace FileLoader {

  struct Resource {
    std::string name;
    int count;

    Resource(const std::string& name, int count)
      : name(name), count(count) {}
  };

  struct Action {
    std::string pid;
    std::string type;
    std::string resource;
    int cycle;

    Action(const std::string& pid, const std::string& type, const std::string& resource, int cycle)
      : pid(pid), type(type), resource(resource), cycle(cycle) {}
  };

  /**
   * Carga procesos desde un archivo
   * Formato esperado: <PID>,<BT>,<AT>,<Priority>
   * @param filepath Ruta al archivo
   * @return Vector de procesos cargados
   */
  std::vector<Process> loadProcesses(const std::string& filepath);
  std::vector<Resource> loadResources(const std::string& filepath);
  std::vector<Action> loadActions(const std::string& filepath);

}

#endif
