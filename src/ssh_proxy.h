//
// Created by limone-m on 02/12/17.
//

#ifndef SSHPROXY_SSH_PROXY_H
#define SSHPROXY_SSH_PROXY_H

int forwarding_client(char *user, char *passwd, char *endpoint, int port, ssh_channel chan);
int verify_knownhost(ssh_session session);
int show_remote_processes(ssh_session session);
int interactive_shell_session(ssh_session session, ssh_channel channel, char *endpoint, int port);

#endif //SSHPROXY_SSH_PROXY_H
