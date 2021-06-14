/*
 * The program implements TCP server and client to fulfil the chatting
 * function. The program supports the following options:
 *
 * ./chat -s
 *   Running the program in server mode
 * ./chat -n <nickname>
 *   Running the program in client mode
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#define NUM_THREADS 5           //define thread number

extern int start_server();
extern int start_client(char *nickname, char *password);

static int chat_help(void)
{
    printf("this is the help\n");
}


int main(int argc, char* argv[], char* envp[])
{

    int opt;
    char *nickname;
    char password[32];

    while ((opt = getopt(argc, argv, "sn:h")) != -1) {
        fprintf(stdout, "opt=%c\n", opt);
        switch(opt) {
            case 's':
                printf("start server\n");
                start_server();

                break;
            case 'n':
                nickname=optarg;
                // printf("input your password:");
                // scanf("%s", password);
                printf("start client with %s \n", argv[2]);
                // printf("your password: %s \n", password);
                start_client(nickname, password);

                break;
            case 'h':
            default:
                printf("help\n");
                chat_help();

            break;
        }
    }

    return 0;
}


