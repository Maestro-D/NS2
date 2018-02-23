#include "ConfigurationController.hpp"
#include "ConfigurationReader.hpp"
#include "Endpoint.hpp"
#include "ServerConfiguration.hpp"
#include "User.hpp"

ConfigurationController::ConfigurationController()
    : _server(NULL)
{
}

ConfigurationController::~ConfigurationController()
{
    delete this->_server;
}

const int ConfigurationController::init()
{
    std::map <std::string, std::vector <std::pair <std::string, std::string>>> datas;
    std::vector <std::pair <std::string, std::string>> keysAndValues;

    ConfigurationReader* foo2 = new ConfigurationReader("/etc/spatch/config.ini");
    // ConfigurationReader* foo2 = new ConfigurationReader("./config/spatch/config.ini");
    datas = foo2->getDatas();

    for (auto const &it : datas) {
        if (it.first == "server") {
            if (it.second.size() > 1) {
                return(this->exitError(it.first));
            }
            this->_server = new ServerConfiguration(this->getServerPort(it.second));
        } else if (it.first == "local_users") {
            this->_users = this->getLocalUsers(it.second);
        } else if (it.first == "endpoints") {
            this->_endpoints = this->getEndpoints(it.second);
        } else if (it.first == "users_control") {
            this->getUsersControl(it.second);
        } else if (it.first == "remote_users") {
            this->getRemoteUsers(it.second);
        }
    }
    delete foo2;
    return 0;
}

const std::vector<User *> &ConfigurationController::users()
{
    return (this->_users);
}

const std::vector<Endpoint *> &ConfigurationController::endpoints()
{
    return (this->_endpoints);
}

const ServerConfiguration *ConfigurationController::server()
{
    return (this->_server);
}

std::vector <std::pair <std::string, std::string>> ConfigurationController::getKeysAndValues(std::vector <std::pair <std::string, std::string>> pairs)
{
    std::vector <std::pair <std::string, std::string>> keysAndValues;

    keysAndValues = {};
    for (auto const &it : pairs) {
        keysAndValues.push_back(std::pair <std::string, std::string>(it.first, it.second));
    }
    return (keysAndValues);
}

std::string ConfigurationController::getServerPort(std::vector <std::pair <std::string, std::string>> pairs)
{
    return (this->getKeysAndValues(pairs).front().second);
}

std::vector<User *> ConfigurationController::getLocalUsers(std::vector <std::pair <std::string, std::string>> pairs)
{
    std::vector<User *> users;

    for (auto const &it : this->getKeysAndValues(pairs)) {
        users.push_back(new User(it.first, it.second));
    }
    return (users);
}

std::vector<Endpoint *> ConfigurationController::getEndpoints(std::vector <std::pair <std::string, std::string>> pairs)
{
    std::vector<Endpoint *> endpoints;
    std::string ipAdress;
    std::string port;

    for (auto const &it : this->getKeysAndValues(pairs)) {
        ipAdress = it.second.substr(0, (it.second.find('|')));
        port = it.second.substr((it.second.find('|') + 1), (it.second.length() - 1));
        endpoints.push_back(new Endpoint(it.first, ipAdress, port));
    }
    return (endpoints);
}

void ConfigurationController::getUsersControl(std::vector <std::pair <std::string, std::string>> pairs)
{
    for (auto const &it : this->getKeysAndValues(pairs)) {
        int i = 0;
        for (; i < this->_users.size() && this->_users[i]->name != it.first; i++);
        if (i <= this->_users.size() - 1) {
            this->_users[i]->availableEndpoints = this->getAvailableEndpoints(it.second);
        }
    }
}

std::vector<Endpoint *> ConfigurationController::getAvailableEndpoints(std::string values)
{
    std::vector<Endpoint *> availableEndpoints;
    std::string delimiter = "|";
    size_t pos = values.find(delimiter);
    std::string value;

    if ((int)pos == -1) {
        pos = values.length();
    }
    while (values != "" && (int)pos != -1) {
        pos = values.find(delimiter);
        value = values.substr(0, pos);
        if (value[0] != '\0' && value != "\n" && value != "\r") {
          for (int i = 0; i < this->_endpoints.size(); i++) {
            if (this->_endpoints[i]->name == value) {
              availableEndpoints.push_back(this->_endpoints[i]);
            }
          }
        }
        values.erase(0, pos + delimiter.length());
    }
    return (availableEndpoints);
}

void ConfigurationController::getRemoteUsers(std::vector <std::pair <std::string, std::string>> pairs)
{
    std::string localUsername;
    std::string endpointName;

    for (auto const &it : this->getKeysAndValues(pairs)) {
        localUsername = this->getLocalUsername(it.first);
        endpointName = this->getEndpointName(it.first);
        int i = 0;
        for (; i < this->_endpoints.size() && this->_endpoints[i]->name != endpointName; i++);
        int j = 0;
        for (; j < this->_users.size() && this->_users[j]->name != localUsername; j++);
        if ((i <= this->_endpoints.size() - 1) && (j <= this->_users.size() - 1)) {
            this->_endpoints[i]->usersAccessControl[this->_users[j]] = this->getAvailableUsernames(it.second);
        }
    }
}

std::string ConfigurationController::getLocalUsername(std::string keys)
{
    return (keys.substr(0, keys.find("|")));
}

std::string ConfigurationController::getEndpointName(std::string keys)
{
    return (keys.substr(keys.find("|") + 1, keys.length()));
}

std::vector<std::string> ConfigurationController::getAvailableUsernames(std::string values)
{
    std::vector<std::string> usernames;
    std::string delimiter = "|";
    size_t pos = values.find(delimiter);
    std::string value;

    if ((int)pos == -1) {
        pos = values.length();
    }
    while (values != "" && (int)pos != -1) {
        pos = values.find(delimiter);
        value = values.substr(0, pos);
        if (value[0] != '\0' && value != "\n" && value != "\r") {
            usernames.push_back(value);
        }
        values.erase(0, pos + delimiter.length());
    }
    return (usernames);
}

const int ConfigurationController::exitError(std::string section)
{
    std::cerr << "Can't read spatch config file correctly, please check [" << section << "] configuration." << std::endl;
    return (-1);
}
