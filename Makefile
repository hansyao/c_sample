BUILD_DIR=`pwd`/build
BIN_DIR=`pwd`/bin
GCC=`which gcc`


.PHONY: default
default: dircreate
	$(GCC) ./network/server.c -o $(BUILD_DIR)/server
	$(GCC) ./network/client.c -o $(BUILD_DIR)/client
	cp -f $(BUILD_DIR)/server $(BIN_DIR)/
	cp -f $(BUILD_DIR)/client $(BIN_DIR)/

.PHONY: server
server: dircreate
	$(GCC) ./network/server.c -o $(BUILD_DIR)/server
	cp -f $(BUILD_DIR)/server $(BIN_DIR)/

.PHONY: client
client:dircreate
	$(GCC) ./network/client.c -o $(BUILD_DIR)/client
	cp -f $(BUILD_DIR)/client $(BIN_DIR)/

.PHONY: clean
clean:
	rm -rf build
	@echo cleaned all build files

dircreate:
	if [ ! -e $(BUILD_DIR) ]; then mkdir build; fi
	if [ ! -e $(BIN_DIR) ]; then mkdir bin; fi