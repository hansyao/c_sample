/*
 * ==========================================================================
 *
 *       Filename:  link.c
 *
 *    Description: single-linked for client 
 *
 *        Version:  1.0
 *        Created:  06/30/2021 01:15:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Hans Yao (YH), hansyow@gmail.com
 *   Organization:  
 *
 * ==========================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat.h"



struct client *add_client(int cfd, char *nickname) 
{
	
	struct client *p;

	p = (struct client *)malloc(sizeof(p));
	if(!p) {
		fprintf (stderr, 
			"\n %s mem alloc failed cfd:%d, nickname:%s\n",
			__func__, cfd, nickname);
	}

	p->cfd = cfd;
	p->nickname = malloc(sizeof(*p->nickname));
	strcpy(p->nickname,  nickname);
	p->next = NULL;

	return p;
}

struct client *add_tail(struct client **head, struct client *new)
{
	struct client **p = head;
	
	while (*p)
		p = &((*p)->next);
	
	*p = new;

	return *p;
}


int del_client(struct client *head, struct client *delete, int n)
{
	/* return 0 - not fund, 1 - del nickname done, 2 - del cfd done */

	struct client *p = head;
	struct client *prev = NULL;
	struct client *d = delete;

	while (p) {
		prev = p;
		p = p->next;

		/* del by cfd */
		if (p->cfd == d->cfd && n == 1)
			goto del;
		/* del by nickname */
		else if (strcmp(p->nickname, d->nickname) == 0 && n == 0)
			goto del;
	}

	fprintf(stderr, "%s not found\n", __func__);
	return 0;
	
del:
	if (p->next) {
		prev->next = p->next;
	}
	else {
		prev->next = NULL;
	}
	
	free(p->nickname);
	free(p);

	return n == 0 ? 1 : 2;
}

void print_list(struct client *head)
{
	struct client *p = head;

	p = p->next;
	while (p) {
		fprintf(stdout, "%s cfd: %d nickname: %s\n", __func__, 
			p->cfd, p->nickname);
		p = p->next;
	}

}



