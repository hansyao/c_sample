/*
 * Copyright 2021 Hans Yao <hansyow@gmail.com>
 * Copyright 2021 Gavin Shan <gshan@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __CSAMPLE_H
#define __CSAMPLE_H

#include "list.h"
#include "network.h"

#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))

struct command {
	const char 		*name;
	int 			(*handler)(int argc, char **argv);
	struct list_head	entry;
	struct list_head	commands;
};

/* Data */
extern struct list_head commands;

/* APIs */
struct command *command_find(const char *name, struct list_head *list);
int command_add(struct command *cmd, struct list_head *list);

#endif /* __CSAMPLE_H */
