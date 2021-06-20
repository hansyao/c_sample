#define CHAT_CONFIG_FILE "config"
#define COMMAND_QUIT "/quit\n"

int chat_get_config(char *key, char *val);
int start_server();
int start_client(char *nickname);
