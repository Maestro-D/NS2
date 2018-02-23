#include "ServerController.hpp"
#include "ConfigurationController.hpp"
#include "ServerConfiguration.hpp"

ServerController::ServerController(ConfigurationController *config)
    : _config(config), _acl(config), _proxy(_acl), _session(_acl, _proxy, "localhost", _config->server()->port)
{
}

ServerController::~ServerController()
{
}

int ServerController::run()
{
    return _session.init() != 0 || _session.handleNewSessions() != 0;
}
