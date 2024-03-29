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
#include <pthread.h>

#include "chat.h"

struct thread_data {
	int	sock;
	char	*nickname;
#define client_state_none	0
#define client_state_quit	1
	int	state;
	int	val;
};

static inline void set_state(struct thread_data *t, int state)
{
	t->state = state;
}

static inline void set_val(struct thread_data *t)
{
	t->val++;
}

static inline int check_state(struct thread_data *t, int state)
{
	return (t->state == state) ? 1 : 0;
}

static inline int check_val(struct thread_data *t, int val)
{
	return (t->val == val) ? 1 : 0;
}

static inline void wait_state(struct thread_data *t, int state, int val)
{
	while (1) {
		if (!check_state(t, state))
			continue;

		/* We don't care the value if -1 is given */
		if (val == -1 || check_val(t, val))
			break;
	}
}

static void *recv_func(void *data)
{
	struct thread_data *t = (struct thread_data *)data;
	int ret, clientSocket = t->sock;
	char recvbuffer[1024];
	time_t timep;

	while(1) {
		/* Check if send thread is quiting */
		if (check_state(t, client_state_quit)) {
			set_val(t);
			wait_state(t, client_state_quit, 3);
			break;			
		}

		ret = recv(clientSocket, recvbuffer,
			   sizeof(recvbuffer), MSG_DONTWAIT);
		if (ret <= 0)
			continue;

		time(&timep);
		printf("%s <<<< %s\n", recvbuffer, ctime(&timep));
	}	
}

static void *send_func(void *data)
{
	struct thread_data *t = (struct thread_data *)data; 
	int clientSocket = t->sock;
	char *nickname = t->nickname; 
	char sendbuffer[1024], buf[100];

	while(1) {
		printf("%s: ", nickname);
		fgets(sendbuffer, sizeof(sendbuffer), stdin);
		if (strcmp(sendbuffer, COMMAND_QUIT) == 0) {
			printf("do you wanna quit? (\"y\"->quit; any key continue) :");
			fgets(buf, sizeof(buf), stdin);
			if (strcmp(buf,"y\n") == 0) {
				break;
			}
		}
		if(send(clientSocket, sendbuffer, strlen(sendbuffer)+1, 0) <= 0) {
			break;	
		}
	}

	/* Tell the main thread to quit */
	set_state(t, client_state_quit);
	set_val(t);
	wait_state(t, client_state_quit, 3);
}

int start_client(char *nickname)
{
	struct thread_data t = {
		.state = client_state_none,
		.nickname = nickname,
	};
	int clientSocket, ret, fd[2], ret_fd;
	serverinfo serverinfo;
	struct sockaddr_in addr;
	socklen_t addrlen;

	char sendbuffer[1024], recvbuffer[1024];
	char buf[100];
	char *state = COMMAND_QUIT;

	pthread_t id[2] = { 0, 0 };

	/* Server IP and port from config file */
 	ret = chat_serverinfo(&serverinfo);
	if (ret) {
		fprintf(stderr, "%s: Error %d to get config\n",
			__func__, ret);
		goto error;
	}

	/* Create socket */
	t.sock = socket(AF_INET, SOCK_STREAM, 0);
	if (t.sock < 0) {
		fprintf(stderr, "%s: Error %d to create socket\n",
			__func__, t.sock);
		goto error;
	}
	
	/* Connect to server */
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(serverinfo.SERVER_PORT));
	addr.sin_addr.s_addr = inet_addr(serverinfo.SERVER_IP);
	addrlen = sizeof(addr);
	ret = connect(t.sock, (const struct sockaddr*)(&addr), addrlen);
	if(ret == -1) {
		fprintf(stderr, "%s: Error %d to connect server\n",
			__func__, ret);
		goto error;
	}

	printf("connect server successfully!\n");
	printf("Your nickname: %s\n", nickname);
	send(t.sock, nickname, strlen(nickname)+1, 0);

	/* Thread to send messages */
	ret = pthread_create(&id[1], NULL, send_func, &t);
	if (ret) {
		fprintf(stderr, "%s: Error %d to create send thread\n",
			__func__, ret);
		goto error;
	}

	/* Thread to receive messages */
	ret = pthread_create(&id[2], NULL, recv_func, &t);
	if (ret != 0) {
		fprintf(stderr, "%s: Error %d to create recv thread\n",
			__func__, ret);
		goto error;
	}

	/* Sleep here and waiting some one to quit */
	while (1) {
		if (check_state(&t, client_state_quit)) {
			set_val(&t);
			wait_state(&t, client_state_quit, 3);
			break;
		}

		sleep(1);
	}
	
	return 0;	

error:
	if (id[1])
		pthread_detach(id[1]);
	if (id[0])
		pthread_detach(id[0]);
	if (t.sock> 0)
		close(t.sock);
	return -EIO;
}

