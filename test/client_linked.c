#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAT_NICKNAME_LEN 100

struct client {
	int cfd;
	char *nickname;
	struct client *next;
};

char *del_nickname_test;

struct client *create_client_nodes(int cfd, char *nickname)
{
	
	/* head point */
	struct client *client_node = NULL;

	/* allocate memory */
	client_node = (struct client *)malloc(sizeof(client_node)); 
	if (client_node == NULL) {
		fprintf (stdout, "%s malloc failed!\n", __func__);
	}

	memset(client_node, 0, sizeof(client_node));	/* mem space initial */
	client_node->cfd = cfd;
	client_node->nickname = nickname;
	client_node->next = NULL;
	
	return client_node;

}

struct client *client_linklist(int n)
{
	struct client *head, *client_node, *end;
	int cfd;
	char *nickname;

	del_nickname_test = malloc(sizeof(del_nickname_test));

	/* allocate memory space */
	head = (struct client *)malloc(sizeof(head));
	end = head;
	for (int i = 0; i < n; i++) {
		cfd = i;
		nickname = malloc(sizeof(nickname));
		sprintf(nickname, "%d", rand());
		client_node = create_client_nodes(cfd, nickname);
		end->next = client_node;
		end = client_node;
		
		fprintf(stdout, "%s cfd - %d nickname: %s\n", 
			__func__, client_node->cfd, client_node->nickname);

		if (i == 3)
			del_nickname_test = nickname;
	}

	end->next = NULL; 
	
	return head;
}

/* insert last */
void tail_insert(struct client *head, struct client *new)
{
	struct client *p = head;

	while(NULL != p->next) {
		p = p->next;
	}

	p->next = new;

}


/* insert head */
void head_insert(struct client *head, struct client *new)
{
	struct client *p = head;

	new->next = p->next;
	p->next = new;

}

void print_client_linklist(struct client *client_linklist)
{
	struct client *p = client_linklist;
	p = p->next;
	while (NULL != p->next) {
		fprintf(stdout, "%s cfd - %d nickname:%s\n", 
					__func__, p->cfd, p->nickname);
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

