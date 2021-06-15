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
#include "server.h"
#include "client.h"

int main(int argc, char* argv[], char* envp[])
{

    char *optstr = "snh";
    int opt;
    char *nickname;
    char password[32];

    while((opt = getopt(argc, argv, optstr)) != -1)
    {
        switch(opt)
        {
            case 's':
                printf("start server\n");
                start_server();

                break;

            case 'n':
                if (argv[2] != NULL)
                {
                    nickname=argv[2];
                    printf("input your password:");
                    scanf("%s", password);
                    printf("start client with %s \n", argv[2]);
                    printf("your password: %s \n", password);
                    start_client(nickname, password);

                    break;
                }

                printf("lack of nickname\n");
                break;

            case 'h':
                printf("help\n");
                chat_help();

                break;
        }
    }

    return 0;
}

int chat_help()
{
    printf("this is the help\n");
}

