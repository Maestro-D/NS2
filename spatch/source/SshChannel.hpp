#ifndef SSH_CHANNEL_HPP
# define SSH_CHANNEL_HPP

# include <libssh/callbacks.h>
# include "AccessListController.hpp"
# include "SshProxy.hpp"

class SshChannel;

struct channel_data_struct
{
    // pointer to the channel the session will allocate
    ssh_channel channel;
    int auth_attempts;
    int authenticated;

    // pid of the child process the channel will spawn
    pid_t pid;
    // pty allocation
    socket_t pty_master;
    socket_t pty_slave;
    // communication with the child process
    socket_t child_stdin;
    socket_t child_stdout;
    // only used for subsystem and exec requests
    socket_t child_stderr;
    // event which is used to poll the above descriptors
    ssh_event event;
    // terminal size structure
    struct winsize *winsize;

    SshChannel *self;
    const User *user;
};

class SshChannel
{
private:
    static const size_t _bufferSize;

    static const struct winsize _defaultWindowSize;
    static const struct channel_data_struct _defaultChannelData;
    static const struct ssh_channel_callbacks_struct _defaultChannelCallbacks;
    static const struct ssh_server_callbacks_struct _defaultServerCallbacks;

public:
    const AccessListController &_acl;

private:
    SshProxy &_proxy;
    ssh_event &_event;
    ssh_session &_session;

public:
    SshChannel(const AccessListController &acl, SshProxy &proxy, ssh_event &event, ssh_session &session);
    ~SshChannel();

    static int dataFunction(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata);
    static int ptyRequest(ssh_session session, ssh_channel channel, const char *term, int cols, int rows, int py, int px, void *userdata);
    static int ptyResize(ssh_session session, ssh_channel channel, int cols, int rows, int py, int px, void *userdata);
    static int execPty(const char *command, struct channel_data_struct *cdata);
    static int execRequest(ssh_session session, ssh_channel channel, const char *command, void *userdata);
    static int shellRequest(ssh_session session, ssh_channel channel, void *userdata);
    static int authenticateUser(ssh_session session, const char *user, const char *pass, void *userdata);
    static ssh_channel channelOpen(ssh_session session, void *userdata);
    static int processStdout(socket_t fd, int revents, void *userdata);
    static int processStderr(socket_t fd, int revents, void *userdata);

    void init();
    SshProxy &proxy();
};

#endif // !SSH_CHANNEL_HPP
