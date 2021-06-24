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

#define LISTENQ 20

struct server server;
struct client client;
struct client cl[FD_SETSIZE]; 

size_t i;

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
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex);
	for(i=0; i<FD_SETSIZE; i++) {
		if(send(cl[i].cfd, msg, strlen(msg)+1,0)<=0) {
			continue;
		}
	}
	pthread_mutex_unlock(&mutex);
}

int start_server(void)
{
	int i, ttl_conn, maxfd, cfd, sockfd, recvlen;
	int nready;
	int n, ret;
	fd_set rset , allset;
	char buffer[MAX_LINE];
	socklen_t len;
	struct sockaddr_in servaddr, caddr;

 
	/* Server IP and port from config file */
	ret=chat_server(&server);

	/*(1) open socket*/
	server.sock = socket(AF_INET , SOCK_STREAM , 0);
 
	/*(2) bind port*/
	bzero(&servaddr , sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(server.PORT));
	servaddr.sin_addr.s_addr = inet_addr(server.IP);
 
	bind(server.sock , (struct sockaddr *)&servaddr , sizeof(servaddr));
 
	/*(3) listen*/
	listen(server.sock , LISTENQ);
 
	/*(4) initiate select*/
	maxfd = server.sock;
	ttl_conn = -1;
	for(i=0 ; i<FD_SETSIZE ; ++i) {
		cl[i].cfd = -1;
	}
	FD_ZERO(&allset);
	FD_SET(server.sock , &allset);
 
	/*(5) loop in server*/

	while(1) {
		rset = allset;
		nready = select(maxfd+1 , &rset, NULL, NULL, NULL);
		
		if(FD_ISSET(server.sock , &rset)) {
			/* receive client requests*/
			len = sizeof(caddr);
 
			printf("\naccpet connection~\n");
 
			if((cfd = accept(server.sock , (struct sockaddr *)&caddr , &len)) < 0) {
				perror("accept error.\n");
				exit(1);
			}

			printf("accpet a new client: %s:%d\n", inet_ntoa(caddr.sin_addr) , caddr.sin_port);
 
			/* add cfd to arrary */
			for(i=0 ; i<FD_SETSIZE ; ++i) {
				if(cl[i].cfd < 0) {
					/* get nickname */
					recvlen = recv(cfd, buffer, sizeof(buffer), 0);
					if(recvlen <= 0 || buffer == "\n")
						break;
					
					/* verify nickname */
					ret = verify_nickname(buffer);
					if (ret == 1) {
						strcpy(buffer, "Duplicated name ");
						strcat(buffer, buffer);
						strcat(buffer, "\n");
						send(cfd, buffer, strlen(buffer),0);
						close(cfd);
						break;
					}
					else {
					
						/* save nickname to arrary if not exist */
						strcpy(cl[i].nickname, buffer);
						memset(buffer, 0, sizeof(buffer));

						fprintf(stdout, "%s %s is online!\n", __func__, cl[i].nickname); 

						cl[i].cfd = cfd;
						client.cfd = cfd;

						/* broadcast oneline status */
						strcpy(buffer, cl[i].nickname);
						strcat(buffer, " is online!\n");
						broadcast(buffer, &server);
						memset(buffer, 0, sizeof(buffer));

						break;
					}
				}
			}
 
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

		for(i=0; i<=ttl_conn ; ++i) {
			if((sockfd = cl[i].cfd) < 0)
				continue;
			if(FD_ISSET(sockfd , &rset)) {
				
				/*process client request*/
				printf("\nreading the socket~~~ \n");
				
				bzero(buffer , MAX_LINE);
				if((n = read(sockfd , buffer, MAX_LINE)) <= 0) {
					/* broadcast offline info */
					printf("%s is offiline!\n", cl[i].nickname);
					broadcast(strcat(cl[i].nickname, " is offline!\n"), &server);

					/* reset nickname buffer after offline*/
					memset(cl[i].nickname, 0, sizeof(cl[i].nickname));

					close(sockfd);
					FD_CLR(sockfd , &allset);
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
					memset(buffer, 0, sizeof(buffer));

					if((ret = write(sockfd , buffer , n)) != n) {
						printf("error writing to the sockfd!\n");
						break;
					}
				}
				if(--nready <= 0)
					break;
			}
		}
	}
}