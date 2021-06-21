#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "chat.h"

Client client[MAX];

size_t cnt = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void broadcast(char *msg, Client c)
{
	size_t i;
	pthread_mutex_lock(&mutex);
	for(i=0;i<cnt;i++) {
		if(client[i].cfd != c.cfd) {
			if(send(client[i].cfd, msg, strlen(msg)+1,0)<=0) {
				break;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}

static int verify_nickname(char *nickname)
{
	int i;

	/* nickname duplicate: 1, otherwise: 0 */
	for (i=0; i<cnt; i++) {
		if (strcmp(nickname, client[i].nickname) == 0)
		return 1;
	}
	return 0;
}

static void *pthread_run(void *arg)
{
	char buffer[1024]={};

	Client cl = *(Client*)(arg);
	while(1) {
		strcpy(buffer, cl.nickname);
		strcat(buffer, ": ");
		int ret = recv(cl.cfd, buffer + strlen(buffer), 1024-strlen(buffer), 0);
		if(ret <= 0) {
			size_t i;
			for(i=0;i<cnt;i++) {
				if(client[i].cfd == cl.cfd) {
					client[i] = client[cnt - 1];
					--cnt;
					strcpy(buffer, "user->");
					strcat(buffer, cl.nickname);
					strcat(buffer," exited!");

					time_t timep;
					time(&timep);
					printf("%s at %s", buffer, ctime(&timep));
					printf("\n");
					break;
				}
			}
			broadcast(buffer, cl);
			close(cl.cfd);
			return NULL;
		}
		else {
			broadcast(buffer, cl);
			printf("%s", buffer);
			// send(cl.cfd,buffer,strlen(buffer)+1,0);
		}
	}
}


int start_server(void)
{
	char buffer[100];
	int serverSocket, ret;

	serverinfo serverinfo;

	serverSocket = socket(AF_INET,SOCK_STREAM, 0);
	if(serverSocket < 0) {
		fprintf(stderr, "%s: Unable to create socket\n", __func__);
		return -ENOMEM;
	}

	/* Server IP and port from config file */
	ret=chat_serverinfo(&serverinfo);
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(serverinfo.SERVER_PORT));
	addr.sin_addr.s_addr = inet_addr(serverinfo.SERVER_IP);
	socklen_t addrlen = sizeof(addr);

	ret = bind(serverSocket, (struct sockaddr*)(&addr), addrlen);
	if(ret == -1) {
		perror("bind");
		return -1;
	}
	if(listen(serverSocket,10) == -1) {
		perror("listen");
		return -1;
	}
	while(1) {

		struct sockaddr_in caddr;
		socklen_t len = sizeof(caddr);
		char *nickname;

		printf("waiting for connecting....\n");
		int cfd = accept(serverSocket, (struct sockaddr*)(&caddr), &len);
		if(cfd == -1) {
			perror("accept");
			return -1;
		}
		
		recv(cfd, &client[cnt].nickname, 40, 0);
		ret = verify_nickname(client[cnt].nickname);
		if (ret == 1) {
			strcpy(buffer, "Duplicated name ");
			strcat(buffer, client[cnt].nickname);
			strcat(buffer, "\n");
			send(cfd, buffer, strlen(buffer),0);
			close(cfd);
			continue;
		}

		client[cnt].cfd = cfd;
		pthread_t id;
		strcpy(buffer,"user->");
		strcat(buffer, client[cnt].nickname);
		strcat(buffer," is online! ");

		printf("%s(ip <%s> port [%hu])\n", buffer,inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port));
		broadcast(buffer,client[cnt]);
		ret = pthread_create(&id, NULL, pthread_run, (void*)(&client[cnt]));
		cnt++;
		if(ret != 0) {
			printf("pthread_create:%s\n",strerror(ret));
			continue;
		}

	}
	return 0;
}
