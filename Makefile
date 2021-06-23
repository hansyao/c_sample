OBJ=chat
SRC_FILES=config.c server.c client.c main.c

default:
	gcc -D__GNU_SOURCE -lpthread $(SRC_FILES) -o $(OBJ)
clean:
	@rm -fr $(OBJ)
