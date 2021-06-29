#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAT_NICKNAME_LEN 100

struct client {
	int		cfd;
	char		*nickname;
	struct client	*next;
};


char *del_nickname_test;

struct client *add_client_node(int cfd, char *nickname)
{
	struct client *client;

	client = (struct client *)malloc(sizeof(*client));
	if (!client) {
		fprintf(stderr, "%s: Memory allocation fails (%d, %s)\n",
			cfd, nickname);
		return NULL;
	}

	memset(client, 0, sizeof(*client));
	client->cfd = cfd;
	client->nickname = nickname;
	client->next = NULL;

	return client;
}

struct client *populate_list(int n, char **delete)
{
	struct client *head, *client, *end;
	int cfd, i;
	char *nickname;

	*delete = NULL;
	head = NULL;

	for (i = 0; i < n; i++) {
		nickname = (char *)malloc(sizeof(int));
		if (!nickname) {
			fprintf(stderr, "%s: Alloc nickname (%d)\n",
				__func__, i);
			continue;
		}

		sprintf(nickname, "%d", rand());
		client = add_client_node(i, nickname);
		if (!cient) {
			free(nickname);
			continue;
		}

		/*  Nickname to be deleted */
		if (*delete == NULL)
			*delete = nickname;
		else if (i == 3)
			*delte = nickname;

		/*  Head node */
		head = head ? head : client;
	}

	return head;
}

void add_tail(struct client **head, struct client *new)
{
	struct client **p = head;

	while (*p)
		p = &((*p)->next);

	*p = new;
}

void add_head(struct client **head, struct client *new)
{
	new->next = *head;
	*head = new;
}

void print_list(struct client *head)
{
	struct client *p = head;

	while (p) {
		fprintf(stdout, "[%04d] [%s]\n", p->cfd, p->nickname);

		p = p->next;
	}
}

int delete_by_cfd(struct client *head, int cfd)
{
	struct client *p = head;
	struct client *prev = NULL;
	while (NULL != p->next) {
		prev = p;
		p = p->next;
		if ( p->cfd == cfd ) {
			if (p->next != NULL) {
				prev->next = p->next;
				free(p);
			}
			else {
				prev->next = NULL;
				free(p);
			}
			return 0;
			fprintf (stdout, "%s delete %d successfully!\n", 
				__func__, cfd);
		}
	}
	fprintf (stdout, "%s not found: %d\n", __func__, cfd);
	return -1;
}


int delete_by_nickname(struct client *head, char *nickname)
{
	struct client *p = head;
	struct client *prev = NULL;
	nickname = del_nickname_test;

	while (NULL != p->next) {
		prev = p;
		p = p->next;
		if ( p->nickname == nickname ) {
			if (p->next != NULL) {
				prev->next = p->next;
				free(p);
			}
			else {
				prev->next = NULL;
				free(p);
			}
			fprintf (stdout, "%s delete %s successfully!\n", 
						__func__, nickname);
			free(del_nickname_test);
			return 0;
		}
	}
	fprintf (stdout, "%s not found: %d\n", __func__, nickname);
	return -1;
}

/* main app */

int main (int argc, char **argv[])
{
	int cfd, ret;

	int n = 10;

	fprintf(stdout, "%s initiate client linklist \n", __func__);
	struct client *client_node = client_linklist(n);

	fprintf(stdout, "%s check head: cfd - %d nickname: %s\n", 
		__func__, client_node->cfd, client_node->nickname);

	/* print whole list */	
	fprintf(stdout, "\n%s print whole list \n", __func__);
	print_client_linklist(client_node);
	
	/* delete by nickname */
	fprintf(stdout, "\n%s start delete nickname %s\n", __func__, 
		del_nickname_test);
	delete_by_nickname(client_node, del_nickname_test);

	/* print whole list to see whether delete successfully */
	fprintf(stdout, "\n%s start print after delete\n", __func__);
	print_client_linklist(client_node);
	free(client_node);
	
	return 0;

}

