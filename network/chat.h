
#define CHAT_CONFIG_FILE "config"
#define COMMAND_QUIT "/quit\n"
#define MAX 100

typedef struct serverinfo {
	char SERVER_IP[100];
	char SERVER_PORT[100];
} serverinfo;


typedef struct Client {
	int cfd;
	char nickname[40];
} Client;

int chat_get_config(char *key, char *val);
int start_server(void);
int start_client(char *nickname);

