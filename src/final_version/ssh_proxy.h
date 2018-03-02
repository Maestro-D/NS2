#ifndef SSHPROXY_SSH_PROXY_H
#define SSHPROXY_SSH_PROXY_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct	userStruct {
	char	userName[15];
	char	passWord[15];
	char	endpointList[10][10];
};

struct	endpointStruct {
	char	endpointAddr[10][30];
	char	endpointPort[10][10];
};

struct endpointStruct fillEndpointAddr(struct endpointStruct myStruct);
struct endpointStruct fillTabAddr(struct endpointStruct myStruct, int cpt, char *tab2);
struct endpointStruct fillTabPort(struct endpointStruct myStruct, int cpt, char *tab2);
struct userStruct fillUserCredential(struct userStruct myStruct);
struct userStruct fillUserEndpoint(struct userStruct myStruct);
struct userStruct fillList(struct userStruct myStruct, int cpt, char *tab2);

#endif //SSHPROXY_SSH_PROXY_H
