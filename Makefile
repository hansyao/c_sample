BUILD_DIR=`pwd`/build
BIN_DIR=`pwd`/bin
GCC=`which gcc`

dircreate:
	if [ ! -e $(BUILD_DIR) ]; then mkdir build; fi
	if [ ! -e $(BIN_DIR) ]; then mkdir bin; fi

default:dircreate
	$(GCC) ./network/server.c -o $(BUILD_DIR)/server
	$(GCC) ./network/client.c -o $(BUILD_DIR)/client
	cp -f $(BUILD_DIR)/server $(BIN_DIR)/
	cp -f $(BUILD_DIR)/client $(BIN_DIR)/

server:dircreate
	$(GCC) ./network/server.c -o $(BUILD_DIR)/server
	cp -f $(BUILD_DIR)/server $(BIN_DIR)/

client:dircreate
	$(GCC) ./network/client.c -o $(BUILD_DIR)/client
	cp -f $(BUILD_DIR)/client $(BIN_DIR)/

clean:
	rm -rf build
	@echo cleaned all build files
