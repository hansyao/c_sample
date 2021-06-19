#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "config.h"

#define MAX 100
typedef struct Client {
	int cfd;
	char nickname[40];
}Client;
Client client[MAX] = {};

static void *pthread_run(void *arg);
static void broadcast(char *msg,Client c);

size_t cnt = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int start_server()
{
	char SERVER_PORT[100];
	char SERVER_IP[100];

	int serverSocket = socket(AF_INET,SOCK_STREAM,0);
	if(serverSocket == -1) {
		perror("socket");
		return -1;
	}

	// get server config from configfile 
    GetIniKeyString("server","SERVER_IP",CONFIGFILE, SERVER_IP);
    GetIniKeyString("server","SERVER_PORT",CONFIGFILE, SERVER_PORT);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(SERVER_PORT));
	addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	socklen_t addrlen = sizeof(addr);

	int ret = bind(serverSocket,(struct sockaddr*)(&addr),addrlen);
	if(ret == -1) {
		perror("bind");
		return -1;
	}
	if(listen(serverSocket,10)==-1) {
		perror("listen");
		return -1;
	}
	while(1) {
		struct sockaddr_in caddr;
		socklen_t len = sizeof(caddr);
		
		printf("waiting for connecting....\n");
		int cfd = accept(serverSocket,(struct sockaddr*)(&caddr),&len);
		if(cfd == -1){
			perror("accept");
			return -1;
		}
		char buffer[100]={};
		recv(cfd,&client[cnt].nickname,40,0);
		client[cnt].cfd = cfd;
		pthread_t id;
		strcpy(buffer,"user->");
		strcat(buffer,client[cnt].nickname);
		strcat(buffer," is online! ");

		printf("%s(ip <%s> port [%hu])\n", buffer,inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port));
		broadcast(buffer,client[cnt]);
		ret = pthread_create(&id,NULL,pthread_run,(void*)(&client[cnt]));
		cnt++;
		if(ret != 0) {
			printf("pthread_create:%s\n",strerror(ret));
			continue;
		}
		// printf("有一个客户机成功连接:ip <%s> port [%hu] nickname: %s\n",inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port), client[cnt].nickname);
	}
	return 0;
}



void broadcast(char *msg,Client c)
{
	size_t i;
	pthread_mutex_lock(&mutex);
	for(i=0;i<cnt;i++) {
		if(client[i].cfd != c.cfd){
			if(send(client[i].cfd,msg,strlen(msg),0)<=0){
				break;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}

void *pthread_run(void *arg)
{
	Client cl = *(Client*)(arg);
	while(1){
		char buffer[1024]={};
		strcpy(buffer, cl.nickname);
		strcat(buffer, ": ");
		int ret = recv(cl.cfd,buffer+strlen(buffer),1024-strlen(buffer),0);
		if(ret <= 0){
			size_t i;
			for(i=0;i<cnt;i++){
				if(client[i].cfd == cl.cfd){
					client[i] = client[cnt-1];
					--cnt;
					strcpy(buffer,"user->");
					strcat(buffer,cl.nickname);
					strcat(buffer," exited!");

					time_t timep;
					time(&timep);
					printf("%s at %s", buffer, ctime(&timep));
					printf("\n");
					break;
				}
			}
			broadcast(buffer,cl);
			close(cl.cfd);
			return NULL;
		}
		else{
			broadcast(buffer,cl);
			printf("%s", buffer);
			// send(cl.cfd,buffer,strlen(buffer),0);
		}
	}
}