/*
 * Copyright 2021 Hans Yao <hansyow@gmail.com>
 * Copyright 2021 Gavin Shan <gshan@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __CSAMPLE_NETWORK_H
#define __CSAMPLE_NETWORK_H

/* Data */
extern struct command network_command;

/* APIs */
int tcp_init(void);
int network_init(void);

#endif /* __CSAMPLE_NETWORK_H */
