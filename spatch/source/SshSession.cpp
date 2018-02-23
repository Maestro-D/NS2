#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include "SshSession.hpp"
#include "SshChannel.hpp"

const std::string SshSession::_keysFolder = "/etc/ssh/";

SshSession::SshSession(const AccessListController &acl, SshProxy &proxy, const std::string &bindAddr, const std::string &bindPort)
    : _acl(acl), _proxy(proxy), _bindAddr(bindAddr), _bindPort(bindPort)
{
}

SshSession::~SshSession()
{
    close();
}

int SshSession::init()
{
    if (_initSignalHandlers() != 0)
        return -1;
    if (ssh_init() == SSH_ERROR || !(_sshBind = ssh_bind_new()))
    {
        std::cerr << "Error during ssh initialization" << std::endl;
        return -1;
    }
    if (ssh_bind_options_set(_sshBind, SSH_BIND_OPTIONS_LOG_VERBOSITY_STR, "3") == SSH_ERROR ||
        ssh_bind_options_set(_sshBind, SSH_BIND_OPTIONS_BINDADDR, _bindAddr.c_str()) == SSH_ERROR ||
        ssh_bind_options_set(_sshBind, SSH_BIND_OPTIONS_BINDPORT_STR, _bindPort.c_str()) == SSH_ERROR ||
        ssh_bind_options_set(_sshBind, SSH_BIND_OPTIONS_RSAKEY, (_keysFolder + "ssh_host_rsa_key").c_str()) == SSH_ERROR ||
        ssh_bind_options_set(_sshBind, SSH_BIND_OPTIONS_DSAKEY, (_keysFolder + "ssh_host_dsa_key").c_str()) == SSH_ERROR ||
        ssh_bind_listen(_sshBind) < 0)
    {
        std::cerr << ssh_get_error(_sshBind) << std::endl;
        return -1;
    }
    return 0;
}

void SshSession::sigchldHandler(int signo)
{
    (void)signo;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int SshSession::_initSignalHandlers()
{
    _signalActions.sa_handler = SshSession::sigchldHandler;
    sigemptyset(&_signalActions.sa_mask);
    _signalActions.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &_signalActions, NULL) != 0)
    {
        std::cerr << "Failed to register SIGCHLD handler" << std::endl;
        return -1;
    }
    return 0;
}

int SshSession::close()
{
    ssh_bind_free(_sshBind);
    ssh_finalize();
    return 0;
}

int SshSession::handleNewSessions()
{
    while (42)
    {
        if ((_session = ssh_new()))
        {
            if (_accept() != SSH_ERROR) // Blocks until there is a new incoming connection
                _fork();
            // Since the session has been passed to a child fork, do some cleaning in parent process
            ssh_disconnect(_session);
            ssh_free(_session);
        }
        else
        {
            std::cerr << "Failed to allocate new session" << std::endl;
        }
    }
}

int SshSession::_accept()
{
    if (ssh_bind_accept(_sshBind, _session) == SSH_ERROR)
    {
        std::cerr << ssh_get_error(_sshBind) << std::endl;
        return SSH_ERROR;
    }
    return 0;
}

void SshSession::_fork()
{
    switch (fork())
    {
    case 0:
        _signalActions.sa_handler = SIG_DFL; // Remove the SIGCHLD handler inherited from parent
        sigaction(SIGCHLD, &_signalActions, NULL);
        ssh_bind_free(_sshBind); // Remove socket binding, which allows us to restart the parent process, without terminating existing sessions
        if ((_event = ssh_event_new()))
        {
            // Blocks until the SSH session ends by either child process exiting, or client disconnecting
            SshChannel channel(_acl, _proxy, _event, _session);
            ssh_event_free(_event);
        }
        else
        {
            std::cerr << "Could not create polling context" << std::endl;
        }
        ssh_disconnect(_session);
        ssh_free(_session);
        exit(0);
    case -1:
        std::cerr << "Failed to fork" << std::endl;
    }
}
