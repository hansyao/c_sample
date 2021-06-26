/*
 * The program implements TCP server and client to fulfil the chatting
 * function. The program supports the following options:
 *
 * ./chat -s
 *	 Running the program in server mode
 * ./chat -n <nickname>
 *	 Running the program in client mode
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "chat.h"

static void usage(void)
{
	fprintf(stdout, "./chat\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "-s				  Start server\n");
	fprintf(stdout, "-n <nickname>	  Start client\n");
	fprintf(stdout, "\n");
}

int main(int argc, char **argv)
{
	char *nickname;
	char password[32];
	int opt, ret = 0;

	while ((opt = getopt(argc, argv, "sn:h")) != -1) {
		switch(opt) {
		case 's':
			ret = start_server();
					break;
		case 'n':
			nickname = optarg;
			// printf("input your password:");
			// scanf("%s", password);
			// printf("your password: %s \n", password);
			fprintf(stdout, "%s nickname: %s\n", __func__, nickname);
			ret = start_client(nickname);
			break;
		case 'h':
		default:
			usage();
		}
	}

	return ret;
}


