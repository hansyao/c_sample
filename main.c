#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>

#include "csample.h"

LIST_HEAD(commands);

struct command *command_find(const char *name, struct list_head *list)
{
	struct command *cmd;

	list_for_each_entry(cmd, list, entry) {
		if (strlen(cmd->name) == strlen(name) &&
 		    !strcmp(cmd->name, name))
			return cmd;
	}

	return NULL;
}

int command_add(struct command *cmd, struct list_head *list)
{
	struct command *tmp;

	if (!cmd->name) {
		fprintf(stderr, "%s: command name can't be NULL\n",
			__func__);
		return -EINVAL;
        }

	/* Duplicate command isn't allowed */
	tmp = command_find(cmd->name, list);
	if (tmp) {
		fprintf(stderr, "%s: command <%s> already existing\n",
			__func__, cmd->name);
		return -EEXIST;
	}

	/* Put it into the linked list */
	INIT_LIST_HEAD(&cmd->entry);
	list_add(&cmd->entry, list);

	return 0;
}

static void usage(void)
{
	struct command *cmd;

	fprintf(stdout, "Supported commands:\n\n");
	list_for_each_entry(cmd, &commands, entry)
		fprintf(stdout, "   %s\n", cmd->name);

	if (!list_empty(&commands))
		fprintf(stdout, "\n");
}

static int command_init(void)
{
	int ret = 0;

	ret |= network_init();

	return 0;
}

int main(int argc, char **argv)
{
	struct command *cmd;
	int ret;

	/* Load commands */
	ret = command_init();
	if (ret) {
		fprintf(stderr, "%s: Error %d to load commands\n",
			__func__, ret);
		return ret;
	}

	/* The sub-command is mandatory */
	if (argc < 2) {
		usage();
		return -EINVAL;
	}

	/* Find the command */
	cmd = command_find(argv[1], &commands);
	if (!cmd) {
		usage();
		return -ENOENT;
	}

	/* Execute it */
	return cmd->handler(argc - 1, &argv[1]);
}
