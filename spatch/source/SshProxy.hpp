#ifndef SSH_PROXY_HPP
# define SSH_PROXY_HPP

# include <vector>
# include <string>
# include <utility>
# include "AccessListController.hpp"

class SshProxy
{
private:
    static const std::vector<std::string> _shellCommands;

    const AccessListController &_acl;

public:
    SshProxy(const AccessListController &acl);
    ~SshProxy();

    std::vector<std::string> tokenizeLine(const std::string &line);
    void interactiveShell(const User &user, const char *command);

private:
    bool _dispatchCommand(const User &user, Endpoint *&endpoint, std::string &alias, const std::vector<std::string> tokens, const char *command);
    bool _listCommand(const User &user, const std::vector<std::string> tokens);
    bool _endpointCommand(const User &user, Endpoint *&endpoint, std::string &alias, const std::vector<std::string> tokens);
    bool _aliasCommand(const User &user, Endpoint *&endpoint, std::string &alias, const std::vector<std::string> tokens);
    bool _connectCommand(const User &user, Endpoint *&endpoint, std::string &alias, const std::vector<std::string> tokens, const char *command);
    bool _helpCommand(const std::vector<std::string> tokens);
    bool _exitCommand(const std::vector<std::string> tokens);
};

#endif // !SSH_PROXY_HPP
