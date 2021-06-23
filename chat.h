#define CHAT_CONFIG_FILE "config"
#define COMMAND_QUIT "/quit\n"
#define MAX 100

typedef struct serverinfo {
	int	sock;
	size_t 	cnt;
	int 	cfd;
	char	*cIP;
	int		cPORT;
	char	nickname[40];
	char	SERVER_IP[100];
	char	SERVER_PORT[100];
} Serverinfo;


int chat_serverinfo(Serverinfo *serverinfo);
int start_server(void);
int start_client(char *nickname);


