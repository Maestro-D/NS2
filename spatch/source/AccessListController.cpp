#include "AccessListController.hpp"
#include "ConfigurationController.hpp"
#include "User.hpp"
#include "Endpoint.hpp"

AccessListController::AccessListController(ConfigurationController *configurationController)
    :_configurationController(configurationController)
{
}

AccessListController::~AccessListController()
{
}

const User *AccessListController::authenticateLocalUser(const std::string &user, const std::string &password) const
{
    for (auto const &it : this->_configurationController->users()) {
        if (it->name == user && it->password == password) {
            return (it);
        }
    }
    return (NULL);
}

const std::vector<Endpoint *> AccessListController::getAvailableEndpointsForUser(const User &user) const
{
    for (auto const &it : this->_configurationController->users()) {
        if (it->name == user.name && it->password == user.password) {
            return (it->availableEndpoints);
        }
    }
    return (std::vector<Endpoint *>());
}

const std::vector<std::string> AccessListController::getAvailableRemoteUsernamesForUserAtEndpoint(const User &user, const Endpoint &endpoint) const
{
    for (auto it : user.availableEndpoints) {
        if (it->name == endpoint.name) {
            if ((it->usersAccessControl).find((User *)&user) != it->usersAccessControl.end()) {
               return (it->usersAccessControl.at((User *)&user));
           }
        }
    }
    return (std::vector<std::string>());
}
