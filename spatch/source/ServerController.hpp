#ifndef SERVER_CONTROLLER_HPP
# define SERVER_CONTROLLER_HPP

# include "AccessListController.hpp"
# include "SshProxy.hpp"
# include "SshSession.hpp"

class ServerController
{
private:
    ConfigurationController *_config;
    AccessListController _acl;
    SshProxy _proxy;
    SshSession _session;

public:
    ServerController(ConfigurationController *config);
    ~ServerController();

    int run();
};

#endif // !SERVER_CONTROLLER_HPP
