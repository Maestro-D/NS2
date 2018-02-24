#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include "ssh_proxy.h"

#define SSHD_USER "toto"
#define SSHD_PASSWORD "toto"
#define FWD_USER "qbayle"
#define FWD_PWD "kant124233"


static const char *name;
static const char *instruction;
static const char *prompts[2];
static char echo[] = { 1, 0 };

static int auth_password(const char *user, const char *password){
    if(strcmp(user, SSHD_USER))
	return 0;
    if(strcmp(password, SSHD_PASSWORD))
        return 0;
    return 1; // authenticated
}

static int 	authenticate(ssh_session session) {
	ssh_message	message;

    	name = "\n\nKeyboard-Interactive Fancy Authentication\n";
    	instruction = "Please enter your real name and your password";
    	prompts[0] = "Real name: ";
    	prompts[1] = "Password: ";
	
	do {
		message=ssh_message_get(session);
        	if(!message)
            		break;
        	switch(ssh_message_type(message)){
            		case SSH_REQUEST_AUTH:
                		switch(ssh_message_subtype(message)){
                    			case SSH_AUTH_METHOD_PASSWORD:
                        			printf("User %s wants to auth with pass %s\n",
                               				ssh_message_auth_user(message),
                               				ssh_message_auth_password(message));
						if(auth_password(ssh_message_auth_user(message),
                           				ssh_message_auth_password(message))){
                               					ssh_message_auth_reply_success(message,0);
                               					ssh_message_free(message);
                               					return 1;
                           			}
		
                        			ssh_message_auth_set_methods(message, SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_INTERACTIVE);
                        			// not authenticated, send default message
                        			ssh_message_reply_default(message);
                        			break;
					case SSH_AUTH_METHOD_INTERACTIVE:

						ssh_message_auth_interactive_request(message, name, instruction, 2, prompts, echo);
                                		ssh_message_auth_reply_success(message,0);
                                		ssh_message_free(message);
                                		return 1;
                    			case SSH_AUTH_METHOD_NONE:
                    			default:
                        			printf("User %s wants to auth with unknown auth %d\n",
                               				ssh_message_auth_user(message),
                               				ssh_message_subtype(message));
                        			ssh_message_auth_set_methods(message,
                                                	SSH_AUTH_METHOD_PASSWORD |
                                                	SSH_AUTH_METHOD_INTERACTIVE);
                        			ssh_message_reply_default(message);
                        			break;
                		}
                		break;
            			default:
                		ssh_message_auth_set_methods(message,
                                	SSH_AUTH_METHOD_PASSWORD |
                                        SSH_AUTH_METHOD_INTERACTIVE);
                		ssh_message_reply_default(message);
        	}
        	ssh_message_free(message);
    	} while (1);
	return 0;
}

static int	print_chan(ssh_channel chan, char *str) {
	int	count;

	count = strlen(str);
	if (ssh_channel_write(chan, str, count) != count)
		printf("Error : Fail to write to the channel : %s\n", str);
		return 0;
	return 1;
}

static int	menu(ssh_channel chan) {
        print_chan(chan, "Voici les actions disponibles :\n");
        print_chan(chan, "List - Affiche la liste des serveurs dispobibles\n");
        print_chan(chan, "Connect [server] - Forwarding vers le serveur choisi\n");
        print_chan(chan, "Help - Affiche ces commandes\n");
        print_chan(chan, "Exit - Quitter le programme\n");
	return 0;
}

static int	port = 55;

int 	main() {

	ssh_session session;
    	ssh_bind sshbind;
    	ssh_message message;
    	ssh_channel chan=0;
    	char buf[2048];
    	int auth=0;
    	int shell=0;
    	int i;
    	int r;
	
	// Open session and set options
  	sshbind = ssh_bind_new();
    	session = ssh_new();
	if (session == NULL)
    		exit(-1);
  	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDADDR, "localhost");
  	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT, &port);
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, "/etc/ssh/ssh_host_rsa_key");

	if(ssh_bind_listen(sshbind) < 0) {
        	printf("Error listening to socket: %s\n", ssh_get_error(sshbind));
        	return 1;
    	}
    	printf("Started sample libssh sshd on port %d\n", port);

    	r = ssh_bind_accept(sshbind, session);
    	if(r == SSH_ERROR) {
      		printf("Error accepting a connection: %s\n", ssh_get_error(sshbind));
      		return 1;
    	}
    	if (ssh_handle_key_exchange(session)) {
        	printf("ssh_handle_key_exchange: %s\n", ssh_get_error(session));
        	return 1;
    	}

	
    	/* proceed to authentication */
    	auth = authenticate(session);
	printf("VALEUR DE AUTH : %i\n", auth); 
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
            		if(ssh_message_type(message) == SSH_REQUEST_CHANNEL && ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_SHELL) {
                		shell = 1;
                		ssh_message_channel_request_reply_success(message);
                		ssh_message_free(message);
                		break;
            		}
            		ssh_message_reply_default(message);
            		ssh_message_free(message);
        	}
		else {
            		break;
        	}
    	} while(!shell);

    	if(!shell) {
        	printf("Error: No shell requested (%s)\n", ssh_get_error(session));
        	return 1;
    	}
	
	print_chan(chan, "\n\n\n\n\n\n\n\nBienvenu sur le proxy spatch !\n");
	menu(chan);
    	do {
		memset(buf, 0, strlen(buf));
        	i = ssh_channel_read(chan,buf, 2048, 0);
        	if (i > 0) {
            		if(*buf == '^C' || *buf == '^D')
                    		break;
            		if (i == 1 && *buf == '\r')
                		ssh_channel_write(chan, "\r\n", 2);
            		else if (strcasecmp(buf, "List\n") == 0)
                                print_chan(chan, "Serveurs diponibles :\n192.168.1.15\n192.174.0.3\n12.12.12.12\n");
			else if (strcasecmp(buf, "Help\n") == 0)
				menu(chan);
			else if (strcasecmp(buf, "Exit\n") == 0)
				break;
			else if (strcasecmp(buf, "Connect\n") == 0)
				forwarding_client(FWD_USER, FWD_PWD, "localhost", 22);
			else
                		ssh_channel_write(chan, buf, i);
			memset(buf, 0, strlen(buf));
            	}
    	} while (i>0);
    	ssh_channel_close(chan);
    	ssh_disconnect(session);
    	ssh_bind_free(sshbind);

	ssh_finalize();

	return 0;
}
