#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>

#include "chat.h"


struct server server;
struct client client;
struct client cl[FD_SETSIZE]; 


static inline void set_state(struct server_thread *t, int state)
{
	t->state = state;
}

static inline void set_val(struct server_thread *t)
{
	t->val++;
}

static inline int check_state(struct server_thread *t, int state)
{
	return (t->state == state) ? 1 : 0;
}

static inline int check_val(struct server_thread *t, int val)
{
	return (t->val == val) ? 1 : 0;
}

static inline void wait_state(struct server_thread *t, int state, int val)
{
	while (1) {
		if (!check_state(t, state))
			continue;

		/* We don't care the value if -1 is given */
		if (val == -1 || check_val(t, val))
			break;
	}
}

static int verify_nickname(char *nickname)
{
	int i;
	/* nickname duplicate: 1, otherwise: 0 */
	for (i=0; i<FD_SETSIZE; i++) {
		if (strcmp(nickname, cl[i].nickname) == 0)
			return 1;
	}
	return 0;
}

static void broadcast(char *msg, struct server *c)
{
	int i;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex);
	for(i=0; i<FD_SETSIZE; i++) {
		// if(cl[i].cfd != client.cfd) continue;
		if(send(cl[i].cfd, msg, strlen(msg), 0)<=0)
			continue;
	}
	pthread_mutex_unlock(&mutex);
}

static int accept_conn_func(struct sockaddr_in caddr, int *cfd)
{
	int recvlen, ret, i;
	socklen_t len;
	char buffer[CHAT_NICKNAME_LEN];
	// struct sockaddr_in caddr = (struct sockaddr_in *)data;
	
	/* receive client requests*/
	len = sizeof(caddr);

	printf("\naccpet connection~\n");

	if((*cfd = accept(server.sock , (struct sockaddr *)&caddr , &len)) < 0) {
		perror("accept error.\n");
		exit(1);
	}

	printf("accpet a new client: %s:%d\n", inet_ntoa(caddr.sin_addr) , caddr.sin_port);

	/* add cfd to arrary */
	fprintf(stdout, "%s FD_SETSIZE before: %d \n", __func__, FD_SETSIZE);
	for(i=0; i<FD_SETSIZE; ++i) {
		bzero(buffer,sizeof(buffer));
		if(cl[i].cfd < 0) {
			/* get nickname */
			recvlen = recv(*cfd, buffer, sizeof(buffer), 0);
			if(recvlen <= 0 || buffer == "\n")
				break;
			
			/* verify nickname */
			ret = verify_nickname(buffer);
			if (ret == 1) {
				strcpy(buffer, "Duplicated name ");
				strcat(buffer, "\n");
				send(*cfd, buffer, strlen(buffer),0);
				close(*cfd);
				cl[i].cfd = -1;
				client.cfd = -1;
				break;
			}
			else {
			
				/* save nickname to arrary if not exist */
				strcpy(cl[i].nickname, buffer);

				fprintf(stdout, "%s %s is online!\n", __func__, cl[i].nickname); 

				cl[i].cfd = *cfd;
				client.cfd = *cfd;

				/* broadcast oneline status */
				// strcpy(buffer, cl[i].nickname);
				strcat(buffer, " is online!\n");
				broadcast(buffer, &server);
				// bzero(buffer, sizeof(buffer));

				break;
			}	
		fprintf(stdout, "%s loop i = %d\n", __func__, i);
		}
	}

	return i;
}

static void client_request_func(int *ttl_conn, fd_set *rset, fd_set *allset, int *nready)
{

	int i, sockfd, ret, recvlen;
	char buffer[CHAT_BUFFER_LEN];

	for(i=0; i<=*ttl_conn ; ++i) {
		if((sockfd = cl[i].cfd) < 0)
			continue;
		if(FD_ISSET(sockfd , rset)) {
			
			/*process client request*/
			printf("\nreading the socket~~~ \n");
			
			if((recvlen = recv(sockfd , buffer, sizeof(buffer), MSG_DONTWAIT)) <= 0) {

				fprintf (stdout, "%s recvlen: %d\n", __func__, recvlen);

				/* broadcast offline info */	

				if ((strlen(cl[i].nickname) == 0) || (cl[i].nickname == "\n"))
					break;

				fprintf(stdout, "%s %s(HEX: %x) is offline!\n", __func__, cl[i].nickname, cl[i].nickname);

				broadcast(strcat(cl[i].nickname, " is offline!\n"), &server);

				/* reset nickname buffer after offline*/
				bzero(cl[i].nickname, sizeof(cl[i].nickname));

				close(sockfd);
				FD_CLR(sockfd , allset);
				cl[i].cfd = -1;
				client.cfd = -1;
			}
			else {
				if(strcmp(buffer, "\n") == 0)
					continue;
				fprintf(stdout, "%s cl[%d] %s send message: %s\n", __func__, i , cl[i].nickname, buffer);
				// fprintf(stdout, "%s nickname: %s\n", __func__, cl[i].nickname);

				strcat(buffer, cl[i].nickname);
				strcat(buffer, ": ");
				broadcast(buffer, &server);

				if((ret = send(sockfd , buffer , recvlen,0)) != recvlen) {
					printf("error writing to the sockfd!\n");
					break;
				}
			}
			if(--nready <= 0)
				fprintf(stdout, "%s nready: %d\n", __func__, nready);

				break;
		}
	}
} 


/* server thread callback*/
static void *server_func(void *data)
{
	
	struct server_thread *t = (struct server_thread *)data;

	fd_set	rset, allset;
	int		i, nready, cfd, maxfd, ttl_conn;
	struct	sockaddr_in caddr;
	pthread_t id;
	int		ret;

	/* initiate select */
	maxfd = server.sock;
	ttl_conn = -1;
	for(i=0 ; i<FD_SETSIZE ; ++i) {
		cl[i].cfd = -1;
	}

	FD_ZERO(&allset);
	FD_SET(server.sock , &allset);

	while(1) {
		rset = allset;
		nready = select(maxfd + 1 , &rset, NULL, NULL, NULL);
		
		fprintf(stdout, "%s nready-before: %d\n", __func__, nready);

		/* accept connections */
		if(FD_ISSET(server.sock , &rset)) {
			i = accept_conn_func(caddr, &cfd);
			fprintf(stdout, "%s loop i = %d\n", __func__, i);
			
			if(FD_SETSIZE == i) {
				perror("too many connection.\n");
				exit(1);
			}

			FD_SET(cfd , &allset);	

			if(cfd > maxfd)
				maxfd = cfd;
			if(i > ttl_conn)
				ttl_conn = i;
			
			if(--nready < 0)
				continue;
		}
		
		fprintf (stdout, "%s ttl_conn: %d\n", __func__, ttl_conn);

		fprintf(stdout, "%s nready-after: %d\n", __func__, nready);

		/* process clients requests */
		client_request_func(&ttl_conn, &rset, &allset, &nready);
		
	}

	/* Tell the main thread to quit */
	set_state(t, server_state_quit);
	set_val(t);
	wait_state(t, server_state_quit, 3);

}


int start_server(void)
{
	int ret;
	struct sockaddr_in servaddr;
	struct server_thread t;
	pthread_t id;
 
	/* Server IP and port from config file */
	ret=chat_server(&server);

	/*(1) open socket*/
	server.sock = socket(AF_INET , SOCK_STREAM , 0);
 
	/*(2) bind port*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(server.PORT));
	servaddr.sin_addr.s_addr = inet_addr(server.IP);
 
	bind(server.sock , (struct sockaddr *)&servaddr , sizeof(servaddr));
 
	/*(3) listen*/
	listen(server.sock , CHAT_LISTENQ);
 
	/*(4) loop in server*/

	/* Thread to process request */
	ret = pthread_create(&id, NULL, server_func, &t);
	if (ret != 0) {
		fprintf(stderr, "%s: Error %d to create recv thread\n",
			__func__, ret);
		goto error;
	}

	/* Sleep here and waiting child thread to quit */
	while (1) {
		if (check_state(&t, server_state_quit)) {
			set_val(&t);
			wait_state(&t, server_state_quit, 3);
			break;
		}

		sleep(0);
	}

error:
	if (id)
		pthread_detach(id);
	if (server.sock> 0)
		close(server.sock);
	return -EIO;

}