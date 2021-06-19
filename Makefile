INCL=-I`pwd`/network/include
BIN_DIR=`pwd`/bin
OBJ=network/chat.c network/getconf.c network/server.c network/client.c

default:
	gcc $(INCL) -lpthread $(OBJ) -o chat