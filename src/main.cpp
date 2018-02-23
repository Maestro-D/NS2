#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <libssh/libssh.h>
#include <libssh/server.h>

static const char *name;
static const char *instruction;
static const char *prompts[2];
static char echo[] = { 1, 0 };

/*static int	authenticate(ssh_session session) {
	ssh_message message;

	do {
		message = ssh_message_get(session);
		if(!message) {
			printf("TA MERE LA PUTE MES COUILLES");
			break;
		}
		switch(ssh_message_type(message)) {
			case SSH_REQUEST_AUTH:
				printf("THE USER WANT TO CONNECT YOU MORRON");
				break;
			default:
				printf("MAIS TU VEUX QUOI BORDEL DE MERDE");
		}
		ssh_message_free(message);	
	} while (42);
	return 0;
}*/

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
						printf("METHODE PASSWORDDDDDDDDDDDDD\n");
                        			printf("User %s wants to auth with pass %s\n",
                               				ssh_message_auth_user(message),
                               				ssh_message_auth_password(message));
                        			if(ssh_userauth_password(session, NULL, ssh_message_auth_password(message))==SSH_AUTH_SUCCESS) {
                               				printf("FUCCCCCCCCKKKKKKKKKKKK CAAAAAA MARCHEEEEEE");
							ssh_message_auth_reply_success(message,0);
                               				ssh_message_free(message);
                               				return 1;
                           			}		
                        			ssh_message_auth_set_methods(message, SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_INTERACTIVE);
                        			// not authenticated, send default message
                        			ssh_message_reply_default(message);
                        			break;

                    			case SSH_AUTH_METHOD_NONE:
                    			default:
						printf("METHODE AUTREEEEEEEE\n");
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
	char *password;
	
	printf("DEBUT");
	
	// Open session and set options
  	sshbind = ssh_bind_new();
    	session = ssh_new();
	if (session == NULL)
    		exit(-1);
  	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDADDR, "localhost");
  	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT, &port);
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, "/etc/ssh/ssh_host_rsa_key");

	printf("APRES_OPEN_SESSION");

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

	
	//r = ssh_userauth_password(session, NULL, password);
    	/* proceed to authentication */
    	//auth = authenticate(session);
    	/*if(!auth) {
        	printf("Authentication error: %s\n", ssh_get_error(session));
        	ssh_disconnect(session);
        	return 1;
    	}*/

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
	
	printf("it works !\n");
    	do {
        	i = ssh_channel_read(chan,buf, 2048, 0);
        	if (i > 0) {
            		if(*buf == '^C' || *buf == '^D')
                    	break;
            		if (i == 1 && *buf == '\r')
                		ssh_channel_write(chan, "\r\n", 2);
            		else
                		ssh_channel_write(chan, buf, i);
            		if (write(1,buf,i) < 0) {
                		printf("error writing to buffer\n");
                		return 1;
            		}
        	}
    	} while (i>0);
    	ssh_channel_close(chan);
    	ssh_disconnect(session);
    	ssh_bind_free(sshbind);

	ssh_finalize();

	return 0;
}
