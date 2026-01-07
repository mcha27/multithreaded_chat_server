CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iheaders

SRC_DIR = src
BIN_DIR = bin

SERVER_SRCS = $(SRC_DIR)/server.cpp
CLIENT_SRCS = $(SRC_DIR)/client.cpp

SERVER = $(BIN_DIR)/server
CLIENT = $(BIN_DIR)/client

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_SRCS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(CLIENT): $(CLIENT_SRCS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -f $(SERVER) $(CLIENT)
