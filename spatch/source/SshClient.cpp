#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "SshClient.hpp"

#include <vector>
#include <string>
#include <iostream>

SshClient::SshClient(const Endpoint &endpoint, const std::string &username, const char *command)
    : _endpoint(endpoint), _username(username), _command(command)
{
}

SshClient::~SshClient()
{
}

void SshClient::connect()
{
    ssh_session my_ssh_session = NULL;
    int verbosity = SSH_LOG_NONE;
    int rc = SSH_ERROR;

    my_ssh_session = ssh_new();
    if (my_ssh_session == NULL)
      {
          std::cerr << "ssh_new() failure" << std::endl;
          return;
      }

    if (ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, _endpoint.ipAdress.c_str()) != 0 ||
        // ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY_STR, "3") != 0 ||
        ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity) != 0 ||
        ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT_STR, _endpoint.port.c_str()) != 0 ||
        ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, _username.c_str()) != 0)
      {
          std::cerr << "Error connecting to remote host: " << ssh_get_error(my_ssh_session) << std::endl;
          return;
      }

    rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK)
      {
          std::cerr << "Error connecting to remote host: " << ssh_get_error(my_ssh_session) << std::endl;
          return;
      }

    // Verify the server's identity
    if (_verifyKnownhost(my_ssh_session) < 0)
    {
      ssh_disconnect(my_ssh_session);
      ssh_free(my_ssh_session);
      return;
    }

    std::string configFolder = "/etc/spatch/";
    std::vector<std::string> keys = { "id_rsa", "id_dsa" };
    ssh_key pubkey = NULL, privkey = NULL;
    bool authenticated = false;

    for (auto key : keys)
    {
        const std::string privateKeyPath = configFolder + key + "_" + _endpoint.name;
        const std::string publicKeyPath = privateKeyPath + ".pub";

        if (ssh_pki_import_pubkey_file(publicKeyPath.c_str(), &pubkey) == SSH_OK)
        {
            if (ssh_userauth_try_publickey(my_ssh_session, NULL, pubkey) == SSH_AUTH_SUCCESS)
            {
                if (ssh_pki_import_privkey_file(privateKeyPath.c_str(), NULL, NULL, NULL, &privkey) == SSH_OK)
                {
                    if (ssh_userauth_publickey(my_ssh_session, NULL, privkey) == SSH_AUTH_SUCCESS)
                    {
                        authenticated = true;
                        break;
                    }
                }
            }
        }
    }

    if (!authenticated)
    {
        std::cerr << "Error authenticating: " << ssh_get_error(my_ssh_session) << std::endl;
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return;
    }

    _execRequest(my_ssh_session);

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    ssh_key_free(pubkey);
    ssh_key_free(privkey);
}

int SshClient::_verifyKnownhost(ssh_session session)
{
  int state, hlen;
  unsigned char *hash = NULL;
  char *hexa;
  char buf[10];

  state = ssh_is_server_known(session);
  hlen = ssh_get_pubkey_hash(session, &hash);
  if (hlen < 0)
    return -1;
  switch (state)
  {
    case SSH_SERVER_KNOWN_OK:
      break; /* ok */
    case SSH_SERVER_KNOWN_CHANGED:
        std::cerr << "Host key for server changed: it is now:" << std::endl;
        ssh_print_hexa("Public key hash", hash, hlen);
        std::cerr << "For security reasons, connection will be stopped" << std::endl;
        free(hash);
        return -1;
    case SSH_SERVER_FOUND_OTHER:
        std::cerr << "The host key for this server was not found but an other type of key exists." << std::endl
            << "An attacker might change the default server key to"
            << "confuse your client into thinking the key does not exist" << std::endl;
        free(hash);
        return -1;
    case SSH_SERVER_FILE_NOT_FOUND:
        std::cerr << "Could not find known host file." << std::endl
            << "If you accept the host key here, the file will be automatically created." << std::endl;
        /* fallback to SSH_SERVER_NOT_KNOWN behavior */
    case SSH_SERVER_NOT_KNOWN:
        hexa = ssh_get_hexa(hash, hlen);
        std::cerr << "The server is unknown. Do you trust the host key?" << std::endl
            << "Public key hash: " << hexa << std::endl;
        free(hexa);
        if (fgets(buf, sizeof(buf), stdin) == NULL)
        {
            free(hash);
            return -1;
        }
        if (strncasecmp(buf, "yes", 3) != 0)
        {
            free(hash);
            return -1;
        }
        if (ssh_write_knownhost(session) < 0)
        {
            std::cerr << "Error " << strerror(errno) << std::endl;
            free(hash);
            return -1;
        }
        break;
    case SSH_SERVER_ERROR:
        std::cerr << "Error " << ssh_get_error(session) << std::endl;
        free(hash);
        return -1;
  }
  free(hash);
  return 0;
}

void SshClient::_execRequest(ssh_session session)
{
    ssh_channel channel;
    int rc;
    char buffer[256];
    // int nbytes;

    channel = ssh_channel_new(session);
    if (channel == NULL)
      return;
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK)
    {
      ssh_channel_free(channel);
      return;
    }
    if (_command)
        rc = ssh_channel_request_exec(channel, _command);
    else
    {
        rc = ssh_channel_request_pty(channel);
        if (rc != SSH_OK) return ;
        rc = ssh_channel_change_pty_size(channel, 80, 24);
        if (rc != SSH_OK) return ;
        rc = ssh_channel_request_shell(channel);
        if (rc != SSH_OK) return ;
    }
        // rc = ssh_channel_request_shell(channel);

    if (rc != SSH_OK)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return;
    }

    // char buffer[256];
     int nbytes, nwritten;
     while (ssh_channel_is_open(channel) &&
            !ssh_channel_is_eof(channel))
     {
       struct timeval timeout;
       ssh_channel in_channels[2], out_channels[2];
       fd_set fds;
       int maxfd;
       timeout.tv_sec = 30;
       timeout.tv_usec = 0;
       in_channels[0] = channel;
       in_channels[1] = NULL;
       FD_ZERO(&fds);
       FD_SET(0, &fds);
       FD_SET(ssh_get_fd(session), &fds);
       maxfd = ssh_get_fd(session) + 1;
       ssh_select(in_channels, out_channels, maxfd, &fds, &timeout);
       if (out_channels[0] != NULL)
       {
         nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
         if (nbytes < 0) return ;
         if (nbytes > 0)
         {
           nwritten = write(1, buffer, nbytes);
           if (nwritten != nbytes) return ;
         }
       }
       if (FD_ISSET(0, &fds))
       {
         nbytes = read(0, buffer, sizeof(buffer));
         if (nbytes < 0) return ;
         if (nbytes > 0)
         {
           nwritten = ssh_channel_write(channel, buffer, nbytes);
           if (nbytes != nwritten) return ;
         }
       }
     }

    // nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    // while (nbytes > 0)
    // {
    //   if (write(1, buffer, nbytes) != (unsigned int) nbytes)
    //   {
    //     ssh_channel_close(channel);
    //     ssh_channel_free(channel);
    //     return;
    //   }
    //   nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    // }
    //
    // if (nbytes < 0)
    // {
    //   ssh_channel_close(channel);
    //   ssh_channel_free(channel);
    //   return;
    // }
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return;
}
