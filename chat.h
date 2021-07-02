#define CHAT_CONFIG_FILE "config"		/* chat config file	*/
#define CHAT_COMMAND_QUIT "/quit\n"		/* quit command		*/
#define CHAT_IP_LEN 100				/* ip length		*/
#define CHAT_PORT_LEN 16			/* ip port lenth	*/
#define CHAT_NICKNAME_LEN 100			/* nickname lenth	*/
#define CHAT_BUFFER_LEN 1024			/* msg max length	*/
#define CHAT_LISTENQ 200			/* listen limits	*/

struct client {
	int  cfd;				/* client file descr	*/
	char *nickname;				/* client nickname	*/
	struct client *next;			/* point to next	*/
	struct client *prev;			/* point to prev	*/
};

struct server {
	int	sock;				/* server socket	*/
	char	IP[CHAT_IP_LEN];		/* server IP addr	*/
	char	PORT[CHAT_PORT_LEN];		/* server listen port	*/
};

struct server_thread {
#define		server_state_none	0	/* client thread none	*/
#define		server_state_quit	1	/* client thread quit	*/
	int	state;				/* thread state		*/
	int	val;				/* thread value		*/
};

struct client_thread {
	int	sock;				/* client socket	*/	
	char	*nickname;			/* client nickname	*/
#define		client_state_none	0	/* client thread none	*/
#define		client_state_quit	1	/* client thread quit	*/
	int	state;				/* thread state		*/
	int	val;				/* thread value		*/
};

int chat_server(struct server *server);	
int start_server(void);	
int start_client(char *nickname);
struct client *add_client(int cfd, char *nickname);
struct client *add_tail(struct client **head, struct client *new);
int del_client(struct client *head, struct client *delete, int n);
void print_list(struct client *head);


