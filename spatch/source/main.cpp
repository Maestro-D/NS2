#include "ConfigurationController.hpp"
#include "ServerController.hpp"

int main(int ac, char **av)
{
  ConfigurationController config;

  if (config.init() == -1)
    return 1;

  ServerController server(&config);

  return server.run();
}
