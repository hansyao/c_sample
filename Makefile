INCL=-I`pwd`/network/include
BIN_DIR=`pwd`/bin
OBJ=network/chat.c cjson/cJSON.c

default:
	gcc $(INCL) -pthread $(OBJ) -o chat