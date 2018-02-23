#ifndef		ACCESSLISTCONTROLLER_HPP_
# define	ACCESSLISTCONTROLLER_HPP_

#include <string>
#include <vector>

class User;
class Endpoint;
class ConfigurationController;

class AccessListController
{

private:
    ConfigurationController *_configurationController;

public:
    AccessListController(ConfigurationController *configurationController);
    ~AccessListController();
    const User *authenticateLocalUser(const std::string &user, const std::string &password) const;
    const std::vector<Endpoint *> getAvailableEndpointsForUser(const User &user) const;
    const std::vector<std::string> getAvailableRemoteUsernamesForUserAtEndpoint(const User &user, const Endpoint &endpoint) const;
};

#endif		/* !ACCESSLISTCONTROLLER_HPP_ */
