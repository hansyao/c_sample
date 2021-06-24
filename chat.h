#define CHAT_CONFIG_FILE "config"			/* chat config file		*/
#define CHAT_COMMAND_QUIT "/quit\n"			/* quit command			*/
#define CHAT_IP_LEN 100						/* ip length			*/
#define CHAT_PORT_LEN 16					/* ip port lenth		*/
#define CHAT_NICKNAME_LEN 48				/* nickname lenth		*/
#define MAX_LINE 1024						/* msg max length 		*/


struct client {
	int		sock;							/* client socket		*/	
	int		cfd;							/* client file descr	*/
	char	nickname[CHAT_NICKNAME_LEN];	/* client nickname		*/
#define 	client_state_none	0			/* client thread none	*/
#define 	client_state_quit	1			/* client thread quit 	*/
	int		state;							/* thread state			*/
	int		val;							/* thread value			*/
};

struct server {
	int		sock;							/* server socket 		*/
	char	IP[CHAT_IP_LEN];				/* server IP address 	*/
	char	PORT[CHAT_PORT_LEN];			/* server listen port	*/
	struct	client client;					/* client info accept	*/
};


int chat_server(struct server *server);		/* main app				*/
int start_server(void);						/* server start			*/
int start_client(char *nickname);			/* client start			*/
