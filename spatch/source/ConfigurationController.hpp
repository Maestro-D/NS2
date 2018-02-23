#ifndef		CONFIGURATIONCONTROLLER_HPP_
# define	CONFIGURATIONCONTROLLER_HPP_

#include <vector>
#include <iostream>
#include <map>
#include "ServerConfiguration.hpp"

class User;
class Endpoint;

class ConfigurationController
{

private:
    std::vector<User *> _users;
    std::vector<Endpoint *> _endpoints;
    const ServerConfiguration *_server;
    std::vector <std::pair <std::string, std::string>> getKeysAndValues(std::vector <std::pair <std::string, std::string>> pairs);
    std::string getServerPort(std::vector <std::pair <std::string, std::string>> pairs);
    std::vector<User *> getLocalUsers(std::vector <std::pair <std::string, std::string>> pairs);
    std::vector<Endpoint *> getEndpoints(std::vector <std::pair <std::string, std::string>> pairs);
    void getUsersControl(std::vector <std::pair <std::string, std::string>> pairs);
    std::vector<Endpoint *> getAvailableEndpoints(std::string values);
    void getRemoteUsers(std::vector <std::pair <std::string, std::string>> pairs);
    std::string getLocalUsername(std::string keys);
    std::string getEndpointName(std::string keys);
    std::vector<std::string> getAvailableUsernames(std::string values);
    const int exitError(std::string section);
public:
    ConfigurationController();
    ~ConfigurationController();
    const int init();
    const std::vector<User *> &users();
    const std::vector<Endpoint *> &endpoints();
    const ServerConfiguration *server();
};

#endif		/* !CONFIGURATIONCONTROLLER_HPP_ */
