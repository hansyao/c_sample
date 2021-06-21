#include <stdio.h>  
#include <stdlib.h>
#include <unistd.h>
#include <string.h>        
#include <errno.h>

#include "chat.h"

static char *trim(char *str)
{
	int i;

	/* Remove tail blanks */
	for (i = (strlen(str) - 1); i >= 0; i--) {
		if (str[i] != ' ')
			break;

		str[i]='\0';
	}

	/* Remove head blanks */
	for (i = 0; i < strlen(str); i++) {
		if (str[i] != ' ')
			return &str[i];
	}

	return NULL;
}

int chat_get_config(char *key, char *val)
{
	FILE *f;
	char line[256], *pkey, *pval;
	int ret = -ENOENT;

	f = fopen(CHAT_CONFIG_FILE, "r");
	if (!f) {
		fprintf(stderr, "%s: Unable to open <%s>\n",
			__func__, CHAT_CONFIG_FILE);
		return -ENOENT;
	}

	while (fgets(line, sizeof(line), f)) {
		/* Remove the carrier-return character */
		line[strlen(line) - 1] = '\0';

		/* Skip the comments */
		if (!strncmp(line, "//", 2) || line[0] == '#')
			continue;

		/* Search the separator */
		pval = strchr(line, '=');
		if (!pval)
			continue;

		/* Split line and trim the unnecessary blanks */
		*pval++ = '\0';
		pkey = trim(line);
		pval = trim(pval);

		if (strlen(pkey) == strlen(key) &&
		    !strcmp(pkey, key)) {
			strcpy(val, pval);
			ret = 0;
			break;
		}
	}

	fclose(f);
	return 0;
}

int chat_serverinfo(serverinfo *serverinfo)
{
	int ret;
	char SERVER_PORT[100];
	char SERVER_IP[100];

	/* Server IP and port from config file */
	ret = chat_get_config("SERVER_IP", SERVER_IP);
	if (ret) {
		fprintf(stderr, "%s: Error %d to get SERVER_IP\n",
			__func__, ret);
		return ret;
	}

	ret = chat_get_config("SERVER_PORT", SERVER_PORT);
	if (ret) {
		fprintf(stderr, "%s: Error %d to get SERVER_PORT\n",
			__func__, ret);
		return ret;
	}

	strcpy(serverinfo->SERVER_IP, SERVER_IP);
	strcpy(serverinfo->SERVER_PORT, SERVER_PORT);
	printf("SERVER_IP: %s  BIND_PORT: %s\n", serverinfo->SERVER_IP, serverinfo->SERVER_PORT);

	return 0;

};