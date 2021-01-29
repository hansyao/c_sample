#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>

#include "csample.h"

static void usage(void)
{
	struct command *cmd;

	fprintf(stdout, "Supported commands:\n\n");
	list_for_each_entry(cmd, &(network_command.commands), entry)
		fprintf(stdout, "   %s\n", cmd->name);

	if (!list_empty(&(network_command.commands)))
		fprintf(stdout, "\n");
}

static int network_handler(int argc, char **argv)
{
	struct command *cmd;

	if (argc < 2) {
		usage();
		return -EINVAL;
	}

	cmd = command_find(argv[1], &(network_command.commands));
	if (!cmd) {
		usage();
		return -ENOENT;
	}

	return cmd->handler(argc - 1, &argv[1]);
}

struct command network_command = {
	.name		= "network",
	.handler	= network_handler,
};

int network_init(void)
{
	int ret = 0;

	INIT_LIST_HEAD(&(network_command.entry));
	INIT_LIST_HEAD(&(network_command.commands));

	ret |= tcp_init();
	ret |= command_add(&network_command, &commands);

	return ret;
}
