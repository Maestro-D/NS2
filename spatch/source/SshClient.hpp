#ifndef SSH_CLIENT_HPP
# define SSH_CLIENT_HPP

# include <libssh/libssh.h>
# include <string>
# include "Endpoint.hpp"

class SshClient
{
private:
    const Endpoint &_endpoint;
    const std::string &_username;
    const char *_command;

public:
    SshClient(const Endpoint &endpoint, const std::string &username, const char *command);
    ~SshClient();

    void connect();

private:
    int _verifyKnownhost(ssh_session session);
    void _execRequest(ssh_session session);
};

#endif // !SSH_CLIENT_HPP
