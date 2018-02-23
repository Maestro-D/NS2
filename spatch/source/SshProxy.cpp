#include <unistd.h>
#include <iostream>
#include <string>
#include "SshProxy.hpp"
#include "SshClient.hpp"
#include "Endpoint.hpp"

const std::vector<std::string> SshProxy::_shellCommands =
{
    "list",
    "endpoint",
    "alias",
    "connect",
    "help",
    "exit",
};

SshProxy::SshProxy(const AccessListController &acl)
    : _acl(acl)
{
}

SshProxy::~SshProxy()
{
}

std::vector<std::string> SshProxy::tokenizeLine(const std::string &line)
{
    const std::string delimiter(" "), replace("\t");
    std::vector<std::string> tokens;
    std::string buffer = line;
    std::string token;
    size_t pos = 0;

    while (buffer.find(replace) != std::string::npos)
        buffer.replace(buffer.find(replace), replace.size(), delimiter);
    while ((pos = buffer.find(delimiter)) != std::string::npos)
    {
        token = buffer.substr(0, pos);
        if (token != "")
            tokens.push_back(token);
        buffer.erase(0, pos + delimiter.length());
    }
    if (buffer != "")
        tokens.push_back(buffer);
    return tokens;
}

void SshProxy::interactiveShell(const User &user, const char *command)
{
    Endpoint *selectedEndpoint = NULL;
    std::string selectedAlias;
    std::string buffer;
    bool end = false;

    std::cout << std::endl << "Type 'help' to display a list of available commands" << std::endl;
    while (!end)
    {
        buffer = "";

        if (selectedEndpoint)
            std::cout << "# selected endpoint -> '" << selectedEndpoint->name << "'" << std::endl;
        if (selectedAlias.size() != 0)
            std::cout << "# selected alias -> '" << selectedAlias << "'" << std::endl;
        std::cout << "spatch $> ";

        std::getline(std::cin, buffer);
        std::vector<std::string> tokens = tokenizeLine(buffer);
        end = _dispatchCommand(user, selectedEndpoint, selectedAlias, tokens, command);
    }
}

bool SshProxy::_dispatchCommand(const User &user, Endpoint *&endpoint, std::string &alias, const std::vector<std::string> tokens, const char *cmd)
{
    if (tokens.size() != 0)
    {
        for (std::string command : _shellCommands)
        {
            if (tokens[0] == command)
            {
                if (command == "list")
                    return _listCommand(user, tokens);
                else if (command == "endpoint")
                    return _endpointCommand(user, endpoint, alias, tokens);
                else if (command == "alias")
                    return _aliasCommand(user, endpoint, alias, tokens);
                else if (command == "connect")
                    return _connectCommand(user, endpoint, alias, tokens, cmd);
                else if (command == "help")
                    return _helpCommand(tokens);
                else if (command == "exit")
                    return _exitCommand(tokens);
            }
        }
        std::cerr << "Unknown command '" << tokens[0] << "'" << std::endl << std::endl;
    }
    return false;
}

bool SshProxy::_listCommand(const User &user, const std::vector<std::string> tokens)
{
    std::cout << "Warning: Not implemented" << std::endl;
    return false;
}

bool SshProxy::_endpointCommand(const User &user, Endpoint *&endpoint, std::string &alias, const std::vector<std::string> tokens)
{
    const std::vector<Endpoint *> availableEndpoints = _acl.getAvailableEndpointsForUser(user);
    bool found = false;

    if (tokens.size() == 1)
    {
        std::cout << "Available endpoints:" << std::endl;
        for (auto availableEndpoint : availableEndpoints)
        {
            std::cout << " - " << availableEndpoint->name << std::endl;
            found = true;
        }
        if (!found)
            std::cout << "endpoint : no available endpoint found" << std::endl;
    }
    else if (tokens.size() == 2)
    {
        for (auto availableEndpoint : availableEndpoints)
        {
            if (tokens[1] == availableEndpoint->name)
            {
                endpoint = availableEndpoint;
                alias = "";
                found = true;
            }
        }
        if (!found)
            std::cout << "endpoint : '" << tokens[1] << "' not found" << std::endl;
    }
    else
        std::cout << "endpoint : too many arguments" << std::endl;
    std::cout << std::endl;
    return false;
}

bool SshProxy::_aliasCommand(const User &user, Endpoint *&endpoint, std::string &alias, const std::vector<std::string> tokens)
{
    std::vector<std::string> availableAliases = _acl.getAvailableRemoteUsernamesForUserAtEndpoint(user, *endpoint);
    bool found = false;

    if (!endpoint)
        std::cout << "alias : you must select an endpoint before selecting an alias" << std::endl;
    else if (tokens.size() == 1)
    {
        std::cout << "Available aliases (usernames) for endpoint '" << endpoint->name << "' :" << std::endl;
        for (auto availableAlias : availableAliases)
        {
            std::cout << " - " << availableAlias << std::endl;
            found = true;
        }
        if (!found)
            std::cout << "alias : no available alias(es) found, you will not be able to log in '" << endpoint->name << "'" << std::endl;
    }
    else if (tokens.size() == 2)
    {
        for (auto availableAlias : availableAliases)
        {
            if (tokens[1] == availableAlias)
            {
                alias = availableAlias;
                found = true;
            }
        }
        if (!found)
            std::cout << "alias : '" << tokens[1] << "' not found for endpoint '" << endpoint->name << "'" << std::endl;
    }
    else
        std::cout << "alias : too many arguments" << std::endl;
    std::cout << std::endl;
    return false;
}

bool SshProxy::_connectCommand(const User &user, Endpoint *&endpoint, std::string &alias, const std::vector<std::string> tokens, const char *command)
{
    SshClient client(*endpoint, alias, command);

    client.connect();
    return true;
}

bool SshProxy::_helpCommand(const std::vector<std::string> tokens)
{
    std::cout
        << "list :\t\t\tlist all current proxified connections and users" << std::endl
        << "endpoint [name] :\tlist all available endpoint OR select an endpoint " << std::endl
        << "alias [name] :\t\tlist all available usernames (aliases) for selected endpoint OR select an alias" << std::endl
        << "connect :\t\tconnect to the previously selected endpoint and username alias" << std::endl
        << "help :\t\t\tdisplay this message" << std::endl
        << "exit :\t\t\tterminate current connection" << std::endl << std::endl;
    return false;
}

bool SshProxy::_exitCommand(const std::vector<std::string> tokens)
{
    std::cout << "Exiting..." << std::endl;
    return true;
}
