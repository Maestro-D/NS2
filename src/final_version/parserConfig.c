#include "ssh_proxy.h"

struct endpointStruct fillTabAddr(struct endpointStruct myStruct, int cpt, char *tab2) {
	int i = 0;
	while (tab2[i] != 0) {
                myStruct.endpointAddr[cpt][i] = tab2[i];
        	i++;
        }
	return myStruct;
	
}

struct endpointStruct fillTabPort(struct endpointStruct myStruct, int cpt, char *tab2) {
        int i = 0;
        while (tab2[i] != 0) {
                myStruct.endpointPort[cpt][i] = tab2[i];
                i++;
        }
        return myStruct;

}

struct	endpointStruct fillEndpointAddr(struct endpointStruct myStruct) {
	FILE *fp;
	char * line = NULL;
    	size_t len = 0;
    	ssize_t read;
	int cpt = 0;
	char *p;
	char *l;

	fp = fopen("config.ini", "r");
    	if (fp == NULL)
        	exit(EXIT_FAILURE);
    	while ((read = getline(&line, &len, fp)) != -1) {
		//printf("la ligne actuelle est : %s\n", line);
		if (strcmp(line, "# endpoint_name=ip_address|port\n") == 0) {
			while ((read = getline(&line, &len, fp)) != -1) {
				if (strcmp(line, "\n") == 0) {
					fclose(fp);
					if (line)
						free(line);
					return myStruct;
				}
				p = strtok(line, "=");
				p = strtok(NULL, "=");
				l = strtok(p, "|");
				myStruct = fillTabAddr(myStruct, cpt, l);
				l = strtok(NULL, "|");
				myStruct = fillTabPort(myStruct, cpt, l);
				cpt++;
			}
		}
	}

    	fclose(fp);
    	if (line)
        	free(line);
    	exit(EXIT_SUCCESS);
}

struct userStruct fillList(struct userStruct myStruct, int cpt, char *tab2) {
        int i = 0;
        while (tab2[i] != 0) {
                myStruct.endpointList[cpt][i] = tab2[i];
                i++;
        }
        return myStruct;
}


struct userStruct fillUserEndpoint(struct userStruct myStruct) {
	FILE *fp;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        int cpt = 0;
        char *p;
        char *l;

        fp = fopen("config.ini", "r");
        if (fp == NULL)
                exit(EXIT_FAILURE);
        while ((read = getline(&line, &len, fp)) != -1) {
                //printf("la ligne actuelle est : %s\n", line);
                if (strcmp(line, "# local_username=available_endpoint_1|available_endpoint_2|...\n") == 0) {
                        while ((read = getline(&line, &len, fp)) != -1) {
                                if (strcmp(line, "\n") == 0) {
                                        fclose(fp);
                                        if (line)
                                                free(line);
                                        return myStruct;
                                }
                                p = strtok(line, "=");
				if (strcmp(p, myStruct.userName) == 0) {
                                	p = strtok(NULL, "=");
                                	l = strtok(p, "|");
					while (l != NULL) {
                                		myStruct = fillList(myStruct, cpt, l);
                                		cpt++;
                                		l = strtok(NULL, "|");
					}
				myStruct.endpointList[cpt][0] = "";
				fclose(fp);
        			if (line)
                			free(line);
				return myStruct;
				}
                        }
                }
        }

        fclose(fp);
        if (line)
                free(line);
        exit(EXIT_SUCCESS);
}

struct  userStruct fillUserCredential(struct userStruct myStruct) {
        FILE *fp;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        char *p;
	int i = 0;

        fp = fopen("config.ini", "r");
        if (fp == NULL)
                exit(EXIT_FAILURE);
        while ((read = getline(&line, &len, fp)) != -1) {
                if (strcmp(line, "# username=password\n") == 0) {
                        while ((read = getline(&line, &len, fp)) != -1) {
                                p = strtok(line, "=");
                                while (p[i] != 0) {
					myStruct.userName[i] = p[i];
					i++;
				}
				i = 0;
                                p = strtok(NULL, "=");
				while (p[i] != 0) {
					myStruct.passWord[i] = p[i];
					i++;
				}
				fclose(fp);
				myStruct = fillUserEndpoint(myStruct);
                                if (line)
                                	free(line);
                               	return myStruct;
                        }
                }
        }
        fclose(fp);
        if (line)
                free(line);
        exit(EXIT_SUCCESS);
}
