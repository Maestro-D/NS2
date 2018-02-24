//
// Created by limone-m on 02/12/17.
//

#ifndef SSHPROXY_SSH_PROXY_H
#define SSHPROXY_SSH_PROXY_H

int forwarding_client(char *user, char *passwd, char *endpoint, int port);
int verify_knownhost(ssh_session session);
int show_remote_processes(ssh_session session);

#endif //SSHPROXY_SSH_PROXY_H
