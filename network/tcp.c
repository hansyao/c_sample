#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>

#include "csample.h"

static int tcp_handler(int argc, char **argv)
{
	int i;

	fprintf(stdout, "%s: FIXME\n", __func__);
	for (i = 0; i < argc; i++)
		fprintf(stdout, "argv[%d]: %s\n", i, argv[i]);

	return 0;
}

static struct command tcp_command = {
	.name		= "tcp",
	.handler	= tcp_handler,
};

int tcp_init(void)
{
	INIT_LIST_HEAD(&(tcp_command.entry));
	INIT_LIST_HEAD(&(tcp_command.commands));

	return command_add(&tcp_command, &(network_command.commands));
}
