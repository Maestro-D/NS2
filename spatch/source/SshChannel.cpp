#include <sys/ioctl.h>
#include <sys/wait.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <iostream>
#include "SshChannel.hpp"
#include "PseudoTerminal.hpp"

const size_t SshChannel::_bufferSize = 1048576;

const struct winsize SshChannel::_defaultWindowSize =
{
    0, // unsigned short ws_row
    0, // unsigned short ws_col
    0, // unsigned short ws_xpixel
    0, // unsigned short ws_ypixel
};

const struct channel_data_struct SshChannel::_defaultChannelData =
{
    NULL, // ssh_channel channel
    0, // int auth_attempts
    0, // int authenticated

    0, // pid_t pid
    -1, // socket_t pty_master
    -1, // socket_t pty_slave
    -1, // socket_t child_stdin
    -1, // socket_t child_stdout
    -1, // socket_t child_stderr
    NULL, // ssh_event event
    NULL, // struct winsize *winsize

    NULL, // User *user
    NULL, // SshChannel *self
};

const struct ssh_channel_callbacks_struct SshChannel::_defaultChannelCallbacks =
{
    sizeof(struct ssh_channel_callbacks_struct), // size_t size
    NULL, // void *userdata
    &SshChannel::dataFunction, // ssh_channel_data_callback
    NULL, // ssh_channel_eof_callback
    NULL, // ssh_channel_close_callback
    NULL, // ssh_channel_signal_callback
    NULL, // ssh_channel_exit_status_callback
    NULL, // ssh_channel_exit_signal_callback
    &SshChannel::ptyRequest, // ssh_channel_pty_request_callback
    &SshChannel::shellRequest, // ssh_channel_shell_request_callback
    NULL, // ssh_channel_auth_agent_req_callback
    NULL, // ssh_channel_x11_req_callback
    &SshChannel::ptyResize, // ssh_channel_pty_window_change_callback
    &SshChannel::execRequest, // ssh_channel_exec_request_callback
    NULL, // ssh_channel_env_request_callback
    NULL, // &subsystem_request // ssh_channel_subsystem_request_callback
};

const struct ssh_server_callbacks_struct SshChannel::_defaultServerCallbacks =
{
    sizeof(struct ssh_server_callbacks_struct), // size_t size
    NULL, // void *userdata
    &SshChannel::authenticateUser, // ssh_auth_password_callback
    NULL, // ssh_auth_none_callback
    NULL, // ssh_auth_gssapi_mic_callback
    NULL, // ssh_auth_pubkey_callback
    NULL, // ssh_service_request_callback
    &SshChannel::channelOpen, // ssh_channel_open_request_session_callback
    NULL, // ssh_gssapi_select_oid_callback
    NULL, // ssh_gssapi_accept_sec_ctx_callback
    NULL, // ssh_gssapi_verify_mic_callback
};

SshChannel::SshChannel(const AccessListController &acl, SshProxy &proxy, ssh_event &event, ssh_session &session)
    : _acl(acl), _proxy(proxy), _event(event), _session(session)
{
    init();
}

SshChannel::~SshChannel()
{
}

int SshChannel::dataFunction(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata)
{
    struct channel_data_struct *cdata = static_cast<struct channel_data_struct *>(userdata);

    (void) session;
    (void) channel;
    (void) is_stderr;

    if (len == 0 || cdata->pid < 1 || kill(cdata->pid, 0) < 0)
        return 0;
    return write(cdata->child_stdin, (char *) data, len);
}

int SshChannel::ptyRequest(ssh_session session, ssh_channel channel, const char *term, int cols, int rows, int py, int px, void *userdata)
{
    struct channel_data_struct *cdata = static_cast<struct channel_data_struct *>(userdata);

    (void) session;
    (void) channel;
    (void) term;

    cdata->winsize->ws_row = rows;
    cdata->winsize->ws_col = cols;
    cdata->winsize->ws_xpixel = px;
    cdata->winsize->ws_ypixel = py;

    if (PseudoTerminal::openPty(&cdata->pty_master, &cdata->pty_slave, cdata->winsize) != 0)
    {
        std::cerr << "Failed to open pty" << std::endl;
        return SSH_ERROR;
    }
    return SSH_OK;
}

int SshChannel::ptyResize(ssh_session session, ssh_channel channel, int cols, int rows, int py, int px, void *userdata)
{
    struct channel_data_struct *cdata = static_cast<struct channel_data_struct *>(userdata);

    (void) session;
    (void) channel;

    cdata->winsize->ws_row = rows;
    cdata->winsize->ws_col = cols;
    cdata->winsize->ws_xpixel = px;
    cdata->winsize->ws_ypixel = py;

    if (cdata->pty_master != -1)
        return ioctl(cdata->pty_master, TIOCSWINSZ, cdata->winsize);
    return SSH_ERROR;
}

int SshChannel::execPty(const char *command, struct channel_data_struct *cdata)
{
    switch ((cdata->pid = fork()))
    {
    case -1:
        close(cdata->pty_master);
        close(cdata->pty_slave);
        std::cerr << "Failed to fork" << std::endl;
        return SSH_ERROR;
    case 0:
        close(cdata->pty_master);
        if (PseudoTerminal::loginTty(cdata->pty_slave) != 0)
            exit(1);

            cdata->self->proxy().interactiveShell(*cdata->user, command);

        exit(0);
    default:
        close(cdata->pty_slave);
        // pty fd is bi-directional
        cdata->child_stdout = cdata->child_stdin = cdata->pty_master;
    }
    return SSH_OK;
}

int SshChannel::execRequest(ssh_session session, ssh_channel channel, const char *command, void *userdata)
{
    struct channel_data_struct *cdata = static_cast<struct channel_data_struct *>(userdata);

    (void) session;
    (void) channel;

    if(cdata->pid > 0)
        return SSH_ERROR;
    if (cdata->pty_master != -1 && cdata->pty_slave != -1)
        return execPty(command, cdata);
    return SSH_ERROR;
}

int SshChannel::shellRequest(ssh_session session, ssh_channel channel, void *userdata)
{
    struct channel_data_struct *cdata = static_cast<struct channel_data_struct *>(userdata);

    (void) session;
    (void) channel;

    if(cdata->pid > 0)
        return SSH_ERROR;
    if (cdata->pty_master != -1 && cdata->pty_slave != -1)
        return execPty(NULL, cdata);
    return SSH_ERROR;
}

int SshChannel::authenticateUser(ssh_session session, const char *username, const char *password, void *userdata)
{
    struct channel_data_struct *cdata = static_cast<struct channel_data_struct *>(userdata);
    SshChannel *self = static_cast<SshChannel *>(cdata->self);
    const User *user = NULL;

    (void) session;

    if ((user = self->_acl.authenticateLocalUser(username, password)))
    {
        cdata->authenticated = 1;
        cdata->user = user;
        return SSH_AUTH_SUCCESS;
    }
    cdata->auth_attempts++;
    return SSH_AUTH_DENIED;
}

ssh_channel SshChannel::channelOpen(ssh_session session, void *userdata)
{
    struct channel_data_struct *cdata = static_cast<struct channel_data_struct *>(userdata);

    cdata->channel = ssh_channel_new(session);
    return cdata->channel;
}

int SshChannel::processStdout(socket_t fd, int revents, void *userdata)
{
    ssh_channel channel = static_cast<ssh_channel>(userdata);
    char buf[_bufferSize];
    int n = -1;

    if (channel != NULL && (revents & POLLIN) != 0 && (n = read(fd, buf, _bufferSize)) > 0)
        ssh_channel_write(channel, buf, n);
    return n;
}

int SshChannel::processStderr(socket_t fd, int revents, void *userdata)
{
    ssh_channel channel = static_cast<ssh_channel>(userdata);
    char buf[_bufferSize];
    int n = -1;

    if (channel != NULL && (revents & POLLIN) != 0 && (n = read(fd, buf, _bufferSize)) > 0)
        ssh_channel_write_stderr(channel, buf, n);
    return n;
}

void SshChannel::init()
{
    struct winsize wsize = _defaultWindowSize;
    struct channel_data_struct cdata = _defaultChannelData;
    struct ssh_channel_callbacks_struct channel_cb = _defaultChannelCallbacks;
    struct ssh_server_callbacks_struct server_cb = _defaultServerCallbacks;
    int n = 0, rc;

    cdata.winsize = &wsize;
    cdata.self = this;
    channel_cb.userdata = &cdata;
    server_cb.userdata = &cdata;

    ssh_callbacks_init(&server_cb);
    ssh_callbacks_init(&channel_cb);

    ssh_set_server_callbacks(_session, &server_cb);

    if (ssh_handle_key_exchange(_session) != SSH_OK)
    {
        std::cerr << ssh_get_error(_session) << std::endl;
        return;
    }

    ssh_set_auth_methods(_session, SSH_AUTH_METHOD_PASSWORD);
    ssh_event_add_session(_event, _session);

    while (cdata.authenticated == 0 || cdata.channel == NULL)
    {
        // if the user has used up all attempts, or if he hasn't been able to authenticate in 10 seconds (n * 100ms), disconnect
        if (cdata.auth_attempts >= 3 || n >= 100)
            return;

        if (ssh_event_dopoll(_event, 100) == SSH_ERROR)
        {
            std::cerr << ssh_get_error(_session) << std::endl;
            return;
        }
        n++;
    }

    ssh_set_channel_callbacks(cdata.channel, &channel_cb);

    do {
        // poll the main event which takes care of the session, the channel and even our child process's stdout/stderr (once it's started)
        if (ssh_event_dopoll(_event, -1) == SSH_ERROR)
            ssh_channel_close(cdata.channel);

        // if child process's stdout/stderr has been registered with the event, or the child process hasn't started yet, continue
        if (cdata.event != NULL || cdata.pid == 0)
            continue;
        // executed only once, once the child process starts
        cdata.event = _event;
        // if stdout valid, add stdout to be monitored by the poll event
        if (cdata.child_stdout != -1 && ssh_event_add_fd(_event, cdata.child_stdout, POLLIN, processStdout, cdata.channel) != SSH_OK)
        {
            std::cerr << "Failed to register stdout to poll context" << std::endl;
            ssh_channel_close(cdata.channel);
        }
        // if stderr valid, add stderr to be monitored by the poll event
        if (cdata.child_stderr != -1 && ssh_event_add_fd(_event, cdata.child_stderr, POLLIN, processStderr, cdata.channel) != SSH_OK)
        {
            std::cerr << "Failed to register stderr to poll context" << std::endl;
            ssh_channel_close(cdata.channel);
        }
    } while (ssh_channel_is_open(cdata.channel) && (cdata.pid == 0 || waitpid(cdata.pid, &rc, WNOHANG) == 0));

    close(cdata.pty_master);
    close(cdata.child_stdin);
    close(cdata.child_stdout);
    close(cdata.child_stderr);

    // remove the descriptors from the polling context, since they are now closed, they will always trigger during the poll calls
    ssh_event_remove_fd(_event, cdata.child_stdout);
    ssh_event_remove_fd(_event, cdata.child_stderr);

    // if the child process exited
    if (kill(cdata.pid, 0) < 0 && WIFEXITED(rc))
    {
        rc = WEXITSTATUS(rc);
        ssh_channel_request_send_exit_status(cdata.channel, rc);
        // if client terminated the channel or the process did not exit nicely, but only if something has been forked
    }
    else if (cdata.pid > 0)
        kill(cdata.pid, SIGKILL);

    ssh_channel_send_eof(cdata.channel);
    ssh_channel_close(cdata.channel);

    // wait up to 5 seconds for the client to terminate the session
    for (n = 0; n < 50 && (ssh_get_status(_session) & (SSH_CLOSED | SSH_CLOSED_ERROR)) == 0; n++)
        ssh_event_dopoll(_event, 100);
}

SshProxy &SshChannel::proxy()
{
    return _proxy;
}
