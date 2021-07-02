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
struct client *head;

int ttl_conn = -1;


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
	struct client *p;

	p = head;
	p = p->next;
	while (p) {
		fprintf(stdout, "%s nickname: %s p->nickname: %s\n", 
			__func__, nickname, p->nickname);
		if(strcmp(p->nickname, nickname) == 0)
			return 1;
		p = p->next;
	}

	return 0;	 /* 0 not found; 1 nickname found */
}

static void broadcast(char *msg, int *cfd)
{
	struct client *p = head;
	
	p = p->next;
	while (p) {
		/* not broadcast for itself */
		if (p->cfd == *cfd)  goto next;

		/* boardcoast all others */
		send(p->cfd, msg, strlen(msg), 0);
next:
		p = p->next;
	}
}

static int accept_conn_func(int *cfd, fd_set *allset)
{
	int recvlen, ret, n;
	struct sockaddr_in caddr;
	socklen_t len;
	char buffer[CHAT_NICKNAME_LEN];

	struct client *p = head;
	struct client *client;

	/* receive client requests*/
	len = sizeof(caddr);

	printf("\naccpet connection~\n");

	*cfd = accept(server.sock, (struct sockaddr *)&caddr, &len);
	if(*cfd < 0) {
		perror("accept error.\n");
		return ttl_conn;
	}

	printf("accpet a new client: %s:%d\n", inet_ntoa(caddr.sin_addr), 
			caddr.sin_port);

	/* add cfd to single-link */
	fprintf(stdout, "%s FD_SETSIZE before: %d \n", __func__, FD_SETSIZE);
		
	n = FD_SETSIZE;
	p = head;
	while (ttl_conn < n) {
		bzero(buffer,sizeof(buffer));
		/* get nickname */
		recvlen = recv(*cfd, buffer, sizeof(buffer), 0);
		if(recvlen <= 0 || (strcmp(buffer, "\n") == 0))
			break;
		
		ttl_conn++;
		/* verify nickname */
		fprintf(stdout, "%s start verify_nickname\n", __func__);
		ret = verify_nickname(buffer);
		fprintf(stdout, "%s verifynickname ret: %d\n", __func__, ret);
		if (ret == 1) {
			strcpy(buffer, "Duplicated name ");
			strcat(buffer, "\n");
			send(*cfd, buffer, strlen(buffer),0);
			
			close(*cfd);
			FD_CLR(*cfd, allset);
			*cfd = head->cfd;	/* select initiate cfd = -1 */

			ttl_conn--;
			fprintf(stdout, "%s del cfd: %d done\n", 
				__func__, *cfd);
			break;
		}
		else if (ret == 0) {
			/* append nickname to link if not exist */
			fprintf(stdout, "%s %s is online!\n", 
					__func__, buffer);

			client  = add_client(*cfd, buffer);
			add_tail(&head, client);	

			print_list(head);

			/* broadcast oneline status */
			strcat(buffer, " is online!\n");
			broadcast(buffer, cfd);

			break;
		}

		n = FD_SETSIZE;
	}

	fprintf(stdout, "%s ttl_conn=%d, n=%d\n", __func__, ttl_conn, n);

	return ttl_conn;
}

static void client_request_func(fd_set *rset, fd_set *allset, int *nready)
{

	int sockfd, ret, recvlen;
	char buffer[CHAT_BUFFER_LEN];
	struct client *p;
	struct client d;	/* delete */

	p = head;
	p = p->next;
	while (p) {
		if((sockfd = p->cfd) < 0) goto next;
		if (!FD_ISSET(sockfd, rset)) goto next;

		/*process client request*/
		printf("\nreading the socket~~~ \n");

		if((recvlen = recv(sockfd, buffer, sizeof(buffer), 0)) <= 0) {
			fprintf (stdout, "%s recvlen: %d\n", 
					__func__, recvlen);
			
			/* broadcast offline info */	
			if ((strlen(p->nickname) == 0) || 
				(strcmp(p->nickname, "\n") == 0) ) 
				break;

			fprintf(stdout, "%s %s is offline!\n", 
				__func__, p->nickname );

			broadcast(strcat(p->nickname, " is offline!\n"), 
					&sockfd);

			/* reset nickname buffer after offline*/
			d.cfd = sockfd;
			d.nickname = p->nickname;
			ret = del_client(head, &d, 1);
			if (ret == 0) goto next;

			close(sockfd);
			FD_CLR(sockfd, allset);

			ttl_conn--;
			fprintf(stdout, "%s ttl_conn:%d\n", __func__, ttl_conn);
		}
		else {
			if(strcmp(buffer, "\n") == 0 || strlen(buffer) == 0)
				goto next;
			fprintf(stdout, "%s  %s send mesg: %s\n", 
				__func__ , p->nickname, buffer);

			strcat(buffer, p->nickname);
			strcat(buffer, ": ");
			broadcast(buffer, &p->cfd);

			ret = send(sockfd, buffer, recvlen, 0);
			if(ret != recvlen) {
				fprintf(stderr, "error write sockfd!\n");
				break;
			}
		}

		if(--*nready <= 0) {
			fprintf(stdout, "%s nready: %d\n", __func__, *nready);
			break;
		}

next:
		p = p->next;
	}
} 


/* server thread callback*/
static void *server_func(void *data)
{

	struct server_thread *t = (struct server_thread *)data;
	struct client *p;

	fd_set rset, allset;
	int i, nready, cfd, maxfd;

	
	head = (struct client *)malloc(sizeof(*head));
	if (!head)
		fprintf(stderr, "\n %s mem alloc fails", __func__);

	/* initiate select */
	maxfd = server.sock;

	p = head;
	p->cfd = -1;

	FD_ZERO(&allset);
	FD_SET(server.sock , &allset);


	fprintf(stdout, "%s maxfd %d\n", __func__, maxfd);

	while(1) {
		rset = allset;
		nready = select(maxfd + 1 , &rset, NULL, NULL, NULL);

		fprintf(stdout, "%s nready-before: %d\n", 
				__func__, nready);

		/* accept connections */
		if(!FD_ISSET(server.sock , &rset)) {
			/* process clients requests */
			client_request_func(&rset, &allset, &nready);
			usleep(10000);		
			continue;
		}

		i = accept_conn_func(&cfd, &allset);
		fprintf(stdout, "%s loop i = %d\n", __func__, i);
		if(FD_SETSIZE == i) {
			perror("too many connection.\n");
			continue;
		}
		
		FD_SET(cfd , &allset);	
		if(cfd > maxfd)
			maxfd = cfd;
		if(--nready < 0)
			break;

		/* process clients requests */
		client_request_func(&rset, &allset, &nready);
		usleep(10000);

	}

	/* release memory */
	while(head) {
		p = head;
		head = head->next;
		free(p->nickname);
		free(p);
	}

	/* Tell the main thread to quit */
	set_state(t, server_state_quit);
	set_val(t);
	wait_state(t, server_state_quit, 3);

	return 0;
}
int start_server(void)
{
	int ret;
	struct sockaddr_in servaddr;
	struct server_thread t;
	pthread_t id;

	/* Server IP and port from config file */
	ret = chat_server(&server);

	/*(1) open socket*/
	server.sock = socket(AF_INET , SOCK_STREAM , 0);

	/*(2) bind port*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(server.PORT));
	servaddr.sin_addr.s_addr = inet_addr(server.IP);

	bind(server.sock, (struct sockaddr *)&servaddr, sizeof(servaddr));

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

