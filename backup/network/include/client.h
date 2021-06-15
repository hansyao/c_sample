#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "cJSON.h"
#define SERVER_PORT 6666


int start_client(char *nickname, char *password)
{

    int clientSocket;
    struct sockaddr_in serverAddr;
    char sendbuf[200];
    char recvbuf[200];
    int iDataNum;
    
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("connect");
        return 1;
    }
    printf("连接到主机...\n");


     /******************************************
	     generate json from nickname & password
	 *******************************************/
	cJSON * jsonroot=0;
	char * jsonout=0;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "nickname", nickname);
	cJSON_AddStringToObject(jsonroot, "password", password);
	//phaser json
	jsonout=cJSON_Print(jsonroot);

    //send json strings to server for authorization
    send(clientSocket, jsonout, strlen(jsonout), 0);

	//releasee json object
	cJSON_Delete(jsonroot);
	//release jsonout
	free(jsonout);

    // receive server response for authorization
    recvbuf[0] = recv(clientSocket, recvbuf, 200, 0);
    if (recvbuf != NULL)
    {
        printf("验证成功:%s \n", recvbuf);
    }
    else
    {
        close(clientSocket);
    }

    while (1)
    {
        printf("发送消息:");
        scanf("%s", sendbuf);
        printf("\n");
        send(clientSocket, sendbuf, strlen(sendbuf), 0);
        if (strcmp(sendbuf, "quit") == 0)
            break;
        printf("读取消息:");
        recvbuf[0] = '\0';
        iDataNum = recv(clientSocket, recvbuf, 200, 0);
        recvbuf[iDataNum] = '\0';
        printf("%s\n", recvbuf);
    }
    close(clientSocket);
}
