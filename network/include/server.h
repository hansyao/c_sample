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

int start_server()
{

    int serverSocket; //socket句柄
    struct sockaddr_in server_addr;
    struct sockaddr_in clientAddr;
    int addr_len = sizeof(clientAddr);
    int client;
    char buffer[200];
    int iDataNum;
    int isusrinfo;
    char *dest_str;
    char *authorized;

    // open socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return 1;
    }
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        return 1;
    }

    //设置服务器上的socket为监听状态
    if (listen(serverSocket, 5) < 0)
    {
        perror("listen");
        return 1;
    }
    while (1)
    {
        printf("监听端口: %d\n", SERVER_PORT);
        client = accept(serverSocket, (struct sockaddr *)&clientAddr, (socklen_t *)&addr_len);
        if (client < 0)
        {
            perror("accept");
            continue;
        }
        printf("等待消息...\n");
        printf("IP is %s\n", inet_ntoa(clientAddr.sin_addr));
        printf("Port is %d\n", htons(clientAddr.sin_port));

        printf("验证用户合法性:\n");
        buffer[0] = '\0';
        iDataNum = recv(client, buffer, 1024, 0);
        if (iDataNum < 0)
        {
            perror("recv null");
            continue;
        }
        buffer[iDataNum] = '\0';

        //print usre info
        printf("%s\n", buffer);

        isusrinfo=userinfo_check(buffer);
        if (isusrinfo=0)
        {
            authorized="user authorized failed!";
            close(serverSocket);
        }
        else
        {    
            
            authorized=" user authorized successfully!";
            printf ("%s \n", authorized);
            send(client, authorized, strlen(authorized), 0);
            
        }

        while (1)
        {
            printf("读取消息:");
            buffer[0] = '\0';
            iDataNum = recv(client, buffer, 1024, 0);
            if (iDataNum < 0)
            {
                perror("recv null");
                continue;
            }
            buffer[iDataNum] = '\0';

            if (strcmp(buffer, "quit") == 0)
                break;
            printf("%s\n", buffer);
            printf("发送消息:");
            scanf("%s", buffer);
            printf("\n");
            send(client, buffer, strlen(buffer), 0);
            if (strcmp(buffer, "quit") == 0)
                break;
        }
    }
    close(serverSocket);
}


int userinfo_check(char *text)
{
    int len = 0;
    cJSON *json;
    cJSON *nickname;
    cJSON *password;

    json = cJSON_Parse(text);
    if (!json) //JSON check
        return 0;
    nickname = cJSON_GetObjectItem(json, "nickname");
    if (!nickname) //nickname check
        return 0;
    password = cJSON_GetObjectItem(json, "password");
    if (!password) //password check
        return 0;

    // get nickname 
    len = strlen(nickname->valuestring);
    memcpy(nickname, nickname->valuestring, len);
    printf("nickname is %s \n", nickname);

    // get password 
    len = strlen(password->valuestring);
    memcpy(password, password->valuestring, len);
    printf("password is %s \n", password);

    return 1;
}


int userinfolist_get(char *text)
{
    int len = 0;
    cJSON *json;
    cJSON *nickname;
    cJSON *password;

    json = cJSON_Parse(text);
    if (!json) //JSON check
        return 0;
    nickname = cJSON_GetObjectItem(json, "nickname");
    if (!nickname) //nickname check
        return 0;
    password = cJSON_GetObjectItem(json, "password");
    if (!password) //password check
        return 0;

    // get nickname 
    len = strlen(nickname->valuestring);
    memcpy(nickname, nickname->valuestring, len);
    printf("nickname is %s \n", nickname);

    // get password 
    len = strlen(password->valuestring);
    memcpy(password, password->valuestring, len);
    printf("password is %s \n", password);

    return 1;
}


typedef struct
{
    int id;
    char nickname[32];
    char password[32];
} userinfo;