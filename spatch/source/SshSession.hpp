#ifndef SSH_SESSION_HPP
# define SSH_SESSION_HPP

# include <libssh/server.h>
# include <signal.h>
# include <iostream>
# include <string>
# include "AccessListController.hpp"
# include "SshProxy.hpp"

class SshSession
{
private:
    static const std::string _keysFolder;

    const AccessListController &_acl;
    SshProxy &_proxy;
    const std::string _bindAddr;
    const std::string _bindPort;

    struct sigaction _signalActions;

    ssh_bind _sshBind;
    ssh_session _session;
    ssh_event _event;

public:
    SshSession(const AccessListController &acl, SshProxy &proxy, const std::string &bindAddr, const std::string &bindPort);
    ~SshSession();

    int init();
    int close();
    int handleNewSessions();

private:
    static void sigchldHandler(int signo);
    int _initSignalHandlers();
    int _accept();
    void _fork();
};

#endif // !SSH_SESSION_HPP
