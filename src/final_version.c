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

static int auth_password(const char *user, const char *password) {
  printf("%s\n", user);
  printf("%s\n", password);

  if(strcmp(user, PROXY_USER)) {
    return SSH_AUTH_DENIED;
  }
  if(strcmp(password, PROXY_PWD)) {
    return SSH_AUTH_DENIED;
  }
  return SSH_AUTH_SUCCESS;;
}

static int authenticate(ssh_session session) {
  ssh_message	message;
  do {
    message=ssh_message_get(session);
    if(!message) {
      break;
    }
    if (ssh_message_type(message) == SSH_REQUEST_AUTH && ssh_message_subtype(message) == SSH_AUTH_METHOD_PASSWORD) {
      if(auth_password(ssh_message_auth_user(message), ssh_message_auth_password(message)) == SSH_AUTH_SUCCESS) {
        ssh_message_auth_reply_success(message,0);
        ssh_message_free(message);
        return 1;
      }
      ssh_message_auth_set_methods(message, SSH_AUTH_METHOD_PASSWORD);
      ssh_message_reply_default(message);
    }
    else {
      ssh_message_auth_set_methods(message, SSH_AUTH_METHOD_PASSWORD);
      ssh_message_reply_default(message);
    }
    ssh_message_free(message);
  } while (1);
  return 0;
}

static int copy_client_to_endpoint(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata) {
  ssh_channel endpoint_channel = *(ssh_channel*)userdata;
  int sz;
  (void)session;
  (void)channel;
  (void)is_stderr;

  // printf("WRITING ON ENDPOINT\n");
  sz = ssh_channel_write(endpoint_channel, data, len);
  return sz;
}

static int copy_endpoint_to_client(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata) {
  ssh_channel client_channel = *(ssh_channel*)userdata;
  int sz;
  (void)session;
  (void)channel;
  (void)is_stderr;

  // printf("WRITING ON CLIENT\n");
  sz = ssh_channel_write(client_channel, data, len);
  return sz;
}

static void chan_close(ssh_session session, ssh_channel channel, void *userdata) {
  // ssh_channel endpoint_channel = *(ssh_channel*)userdata;
  (void)session;
  (void)channel;

  ssh_channel_close(channel);
}

struct ssh_channel_callbacks_struct cb = {
  .channel_data_function = copy_client_to_endpoint,
  .channel_eof_function = chan_close,
  .channel_close_function = chan_close,
  .userdata = NULL
};

struct ssh_channel_callbacks_struct cc = {
  .channel_data_function = copy_endpoint_to_client,
  .channel_eof_function = chan_close,
  .channel_close_function = chan_close,
  .userdata = NULL
};

static int port = 55;

int main() {
  ssh_session session;
  ssh_bind sshbind;
  ssh_message message;
  ssh_channel chan=0;
  int rc = 0, auth = 0, shell = 0, sftp = 0;

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

  if (ssh_handle_key_exchange(session)) {
    printf("ssh_handle_key_exchange: %s\n", ssh_get_error(session));
    return 1;
  }

  /* proceed to authentication */

  auth = authenticate(session);

  if(!auth) {
    printf("Authentication error: %s\n", ssh_get_error(session));
    ssh_disconnect(session);
    return 1;
  }

  /* wait for a channel session */
  do {
    message = ssh_message_get(session);
    if(message){
      if(ssh_message_type(message) == SSH_REQUEST_CHANNEL_OPEN && ssh_message_subtype(message) == SSH_CHANNEL_SESSION) {
        chan = ssh_message_channel_request_open_reply_accept(message);
        ssh_message_free(message);
        break;
      }
      else {
        ssh_message_reply_default(message);
        ssh_message_free(message);
      }
    }
    else {
      break;
    }
  } while(!chan);

  if(!chan) {
    printf("Error: client did not ask for a channel session (%s)\n", ssh_get_error(session));
    ssh_finalize();
    return 1;
  }

  /* wait for a shell */
  do {
    message = ssh_message_get(session);
    if(message != NULL) {
      if(ssh_message_type(message) == SSH_REQUEST_CHANNEL) {
        if(ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_SHELL) {
          shell = 1;
          ssh_message_channel_request_reply_success(message);
          ssh_message_free(message);
          break;
        } else if(ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_PTY) {
          ssh_message_channel_request_reply_success(message);
          ssh_message_free(message);
          continue;
        }
      }
      ssh_message_reply_default(message);
      ssh_message_free(message);
    } else {
      break;
    }
  } while(!shell);

  if(!shell) {
    printf("Error: No shell requested (%s)\n", ssh_get_error(session));
    return 1;
  }

  //
  // ssh_event event = ssh_event_new();
  //
  // if(ssh_event_add_session(event, session) != SSH_OK) {
  //   printf("Couldn't add the session to the event\n");
  //   return -1;
  // }
  //
  // do {
  //   ssh_event_dopoll(event, 1000);
  // } while(!ssh_channel_is_closed(chan));

  ssh_session client_session = ssh_new();

  ssh_options_set(client_session, SSH_OPTIONS_HOST, "localhost");
  ssh_options_set(client_session, SSH_OPTIONS_USER, "aiscky");

  printf("CONNECT\n");

  rc = ssh_connect(client_session);
  if (rc != SSH_OK){
    printf("Error connecting to localhost: %s", ssh_get_error(client_session));
    return 1;
  }

  printf("AUTH\n");
  rc = ssh_userauth_password(client_session, "aiscky", "root");
  if (rc != SSH_AUTH_SUCCESS){
    printf("Authentication failed: %s\n",ssh_get_error(client_session));
    return 1;
  }

  printf("CHANNEL\n");
  ssh_channel client_channel = ssh_channel_new(client_session);
  if (client_channel == NULL)
  return SSH_ERROR;
  rc = ssh_channel_open_session(client_channel);
  if (rc != SSH_OK)
  {
    ssh_channel_free(client_channel);
    return rc;
  }

  rc = ssh_channel_request_pty(client_channel);
  if (rc != SSH_OK) return rc;

  // rc = ssh_channel_change_pty_size(channel, 80, 24);
  // if (rc != SSH_OK) return rc;

  rc = ssh_channel_request_shell(client_channel);
  if (rc != SSH_OK) return rc;

  cb.userdata = &client_channel;
  ssh_callbacks_init(&cb);
  ssh_set_channel_callbacks(chan, &cb);

  cc.userdata = &chan;
  ssh_callbacks_init(&cc);
  ssh_set_channel_callbacks(client_channel, &cc);

  ssh_event event = ssh_event_new();

  if(ssh_event_add_session(event, session) != SSH_OK) {
    printf("Couldn't add the session to the event\n");
    return -1;
  }

  do {
    ssh_event_dopoll(event, 1000);
  } while(!ssh_channel_is_closed(chan));

  printf("ENDING\n");

  ssh_channel_close(chan);
  ssh_disconnect(session);
  ssh_bind_free(sshbind);

  ssh_finalize();
}
