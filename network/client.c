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
#include <fcntl.h>

#include "chat.h"

int start_client(char *nickname)
{
	char SERVER_PORT[100];
	char SERVER_IP[100];
	int clientSocket, ret, fd[2], ret_fd;
	serverinfo serverinfo;

	char sendbuffer[1024], recvbuffer[1024];
	char buf[100];
	int retp=pipe(fd); 
	char *state = COMMAND_QUIT;

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket < 0) {
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

	ret = connect(clientSocket, (const struct sockaddr*)(&addr), addrlen);
	if(ret == -1) {
		perror("connect");
		return -1;
	}

	printf("connect server successfully!\n");
	printf("Your nickname: %s\n", nickname);
	send(clientSocket, nickname, strlen(nickname)+1, 0);

	if(retp == -1) {
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
            printf("%s: ", nickname);
            fgets(sendbuffer, sizeof(sendbuffer), stdin);
			if (strcmp(sendbuffer, COMMAND_QUIT) == 0) {
				printf("do you wanna quit? (\"y\"->quit; any key continue) :");
				fgets(buf, sizeof(buf), stdin);
				if (strcmp(buf,"y\n") == 0){
					/* define pipe val in parent process */
					close(fd[0]);
					write(fd[1], state, strlen(state));
					waitpid(pid, NULL, 0);
					close(fd[1]);
					break;
				}
			}
			if(send(clientSocket, sendbuffer, strlen(sendbuffer)+1, 0) <= 0) {
				break;	
			}
		}
	}

    //receive message
    else{

		/* read pipe status from child process */			
		close(fd[1]);
		ret_fd = read(fd[0], buf, sizeof(buf));
		if(ret_fd > 0) {
			if (strcmp(buf, COMMAND_QUIT) == 0) {
				close(clientSocket);
				return 0;
			}
		}

		while(1) {
			if(recv(clientSocket, recvbuffer, sizeof(recvbuffer), 0) <= 0)
				break;
			time_t timep;
			time(&timep);
			printf("%s <<<< %s\n", recvbuffer, ctime(&timep));
		}
	}

	close(clientSocket);
	return 0;	
}

