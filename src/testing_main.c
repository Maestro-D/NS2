#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>

#define PROXY_USER "root"
#define PROXY_PWD "root"
#define LINUX_USER "aiscky"
#define LINUX_PWD "root"
#define HOSTNAME "localhost"
#define RSA_FOLDER "/etc/ssh/ssh_host_rsa_key"

static int port = 55;
static int authenticated=0;
static ssh_channel channel=0;
static int shell=0;

static int pty_request(ssh_session session, ssh_channel channel, const char *term,
  int x,int y, int px, int py, void *userdata){
    (void) session;
    (void) channel;
    (void) term;
    (void) x;
    (void) y;
    (void) px;
    (void) py;
    (void) userdata;
    printf("Allocated terminal\n");
    return 0;
  }

  static int shell_request(ssh_session session, ssh_channel channel, void *userdata){
    (void)session;
    (void)channel;
    (void)userdata;
    shell=1;
    printf("Allocated shell\n");
    return 0;
  }

  struct ssh_channel_callbacks_struct channel_cb = {
    .channel_pty_request_function = pty_request,
    .channel_shell_request_function = shell_request
  };

  static ssh_channel new_session_channel(ssh_session session, void *userdata){
    (void) session;
    (void) userdata;
    if(channel != NULL)
    return NULL;
    printf("Allocated session channel\n");
    channel = ssh_channel_new(session);
    ssh_callbacks_init(&channel_cb);
    ssh_set_channel_callbacks(channel, &channel_cb);
    return channel;
  }

  static int copy_server_chan_to_client(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata) {
    ssh_channel client_channel = *(ssh_channel*)userdata;
    int sz;
    (void)session;
    (void)channel;
    (void)is_stderr;

    printf("Trying to write to client\n");
    sz = ssh_channel_write(client_channel, data, len);
    return sz;
  }

  static void chan_close(ssh_session session, ssh_channel channel, void *userdata) {
    int fd = *(int*)userdata;
    (void)session;
    (void)channel;

    close(fd);
  }

  struct ssh_channel_callbacks_struct ccb = {
    .channel_data_function = copy_server_chan_to_client,
    .channel_eof_function = chan_close,
    .channel_close_function = chan_close,
    .userdata = NULL
  };

  static int auth_password(ssh_session session, const char *user,
    const char *password, void *userdata) {
      printf("User : %s\n", user);
      printf("Password : %s\n", password);
      if(strcmp(user, PROXY_USER)) {
        return SSH_AUTH_DENIED;
      }
      if(strcmp(password, PROXY_PWD)) {
        return SSH_AUTH_DENIED;
      }
      authenticated = 1;
      return SSH_AUTH_SUCCESS;;
    }

    int main() {
      ssh_session session;
      ssh_bind sshbind;
      ssh_event authentication_event;
      int rc = 0;

      struct ssh_server_callbacks_struct cb = {
        .userdata = NULL,
        .auth_password_function = auth_password,
        .channel_open_request_session_function = new_session_channel
      };

      sshbind = ssh_bind_new();
      session = ssh_new();

      /* Settings of SSH session */
      ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDADDR, HOSTNAME);
      ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT, &port);
      ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, RSA_FOLDER);

      /* Listening for connections */
      if(ssh_bind_listen(sshbind)<0){
        printf("Error listening to socket: %s\n", ssh_get_error(sshbind));
        return 1;
      }

      /* Accepting entering connection */
      rc = ssh_bind_accept(sshbind, session);
      if (rc == SSH_ERROR) {
        printf("Error accepting a connection: %s\n", ssh_get_error(sshbind));
        return 1;
      }

      ssh_callbacks_init(&cb);
      ssh_set_server_callbacks(session, &cb);

      /* Setting up encryption */
      if (ssh_handle_key_exchange(session)) {
        printf("ssh_handle_key_exchange: %s\n", ssh_get_error(session));
        return 1;
      }

      /* proceed to authentication */
      ssh_set_auth_methods(session,SSH_AUTH_METHOD_PASSWORD);
      authentication_event = ssh_event_new();
      ssh_event_add_session(authentication_event, session);

      while (!authenticated || !channel || !shell){
        rc = ssh_event_dopoll(authentication_event, 1000);
        if (rc == SSH_ERROR){
          printf("Error : %s\n", ssh_get_error(session));
          ssh_disconnect(session);
          return 1;
        }
      }

      printf("Authenticated & channel created\n");
      printf("Server ending\n");

      ssh_session client_session = ssh_new();

      ssh_options_set(client_session, SSH_OPTIONS_HOST, "localhost");
      ssh_options_set(client_session, SSH_OPTIONS_USER, "aiscky");

      printf("Trying to connect\n");

      rc = ssh_connect(client_session);
      if (rc != SSH_OK){
        printf("Error connecting to localhost: %s", ssh_get_error(client_session));
        return 1;
      }

      printf("Trying to authentify");
      rc = ssh_userauth_password(client_session, "aiscky", "root");
      if (rc != SSH_AUTH_SUCCESS){
        printf("Authentication failed: %s\n",ssh_get_error(client_session));
        return 1;
      }

      printf("Creating channel\n");
      ssh_channel client_channel = ssh_channel_new(client_session);
      if (client_channel == NULL)
      return SSH_ERROR;
      rc = ssh_channel_open_session(client_channel);
      if (rc != SSH_OK)
      {
        ssh_channel_free(client_channel);
        return rc;
      }

      cb.userdata = &client_channel;
      ssh_callbacks_init(&ccb);
      ssh_set_channel_callbacks(client_channel, &ccb);

      ssh_event event = ssh_event_new();

      if(ssh_event_add_session(event, session) != SSH_OK) {
        printf("Couldn't add the session to the event\n");
        return -1;
      }

      printf("Sending ps aux\n");
      rc = ssh_channel_request_exec(client_channel, "ps aux");
      if (rc != SSH_OK)
      {
        printf("error");
        ssh_channel_close(client_channel);
        ssh_channel_free(client_channel);
        return rc;
      }

      do {
        ssh_event_dopoll(event, 1000);
      } while(!ssh_channel_is_closed(client_channel));

      // char buffer[2048];
      // int nbytes = ssh_channel_read(client_channel, buffer, sizeof(buffer), 0);
      //
      // printf("nbytes : %d\n", nbytes);
      //
      // while (nbytes > 0)
      // {
      //   if (write(/*client_channel*/1, buffer, nbytes) != (unsigned int) nbytes)
      //   {
      //     ssh_channel_close(client_channel);
      //     ssh_channel_free(client_channel);
      //     return SSH_ERROR;
      //   }
      //   nbytes = ssh_channel_read(client_channel, buffer, sizeof(buffer), 0);
      // }
      //
      // if (nbytes < 0)
      // {
      //   ssh_channel_close(client_channel);
      //   ssh_channel_free(client_channel);
      //   return SSH_ERROR;
      // }
      ssh_channel_send_eof(client_channel);
      ssh_channel_close(client_channel);
      ssh_channel_free(client_channel);

      ssh_disconnect(session);
      ssh_bind_free(sshbind);
      ssh_finalize();
      return 1;
    }
