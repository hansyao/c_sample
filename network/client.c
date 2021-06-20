#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "chat.h"

int start_client(char *nickname)
{
	char SERVER_PORT[100];
	char SERVER_IP[100];
	int clientSocket, ret, fd[2];

	char sendbuffer[1024], recvbuffer[1024];
	char buf[100], state[64];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket < 0) {
		fprintf(stderr, "%s: Unable to create socket\n", __func__);
		return -ENOMEM;
	}
	
	/* Server IP and port from config file */
	ret = chat_get_config("SERVER_IP", SERVER_IP);
	if (ret) {
		fprintf(stderr, "%s: Error %d to get SERVER_IP\n",
			__func__, ret);
		return ret;
	}

	ret = chat_get_config("SERVER_PORT", SERVER_PORT);
	if (ret) {
		fprintf(stderr, "%s: Error %d to get SERVER_PORT\n",
			__func__, ret);
		return ret;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(SERVER_PORT));
	addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	socklen_t addrlen = sizeof(addr);

	ret = connect(clientSocket,(const struct sockaddr*)(&addr),addrlen);
	if(ret == -1) {
		perror("connect");
		return -1;
	}
	printf("connect server successfully!\n");
	printf("Your nickname: %s\n",nickname);

	send(clientSocket,nickname,strlen(nickname)+1,0);

	int retp=pipe(fd); 
	if(retp==-1) {
		fprintf(stderr, "%s: Unable to create pipe\n", __func__);
		return -ENOMEM;
	}

	pid_t pid = fork();
	if(pid == -1) {
		perror("fork");	
		return -1;
	}

    //send message
	if(pid == 0) {
		while(1) {
            printf("%s: ",nickname);
            fgets(sendbuffer,sizeof(sendbuffer),stdin);
			if (strcmp(sendbuffer,COMMAND_QUIT) == 0) {
				printf("do you wanna quit? (\"y\"->quit; any key continue) :");
				fgets(buf, sizeof(buf), stdin);
				if (strcmp(buf,"y\n") == 0){
					/* define pipe val in parent pid */
					close(fd[0]);
					char *state = COMMAND_QUIT;
					write(fd[1],state,strlen(state));
					wait(NULL);
					break;
				}
			}
			if(send(clientSocket,sendbuffer,strlen(sendbuffer)+1,0)<=0) {
				break;	
			}
		}
	}

    //receive message
    else{
		while(1) {
			/* read pipe status from child pid */
			close(fd[1]);
			ret = read(fd[0],state,sizeof(state));
			write(STDOUT_FILENO,state,ret);			//convert to standard output

			if (strcmp(state,COMMAND_QUIT) == 0)
				break;
			if(recv(clientSocket,recvbuffer,1024,0)<=0)
				break;
			time_t timep;
			time(&timep);
			printf("%s\n",ctime(&timep));
		}
	}
	close(clientSocket);
	return 0;	
}

