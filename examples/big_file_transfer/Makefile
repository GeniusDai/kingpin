BIN = bin
SRC = src
SERVER = StartServer
CLIENT = StartClient

CXX = g++
CXXFLAGS = -O0 -Wall -std=c++11 -g -I ../../include

LDFLAGS = -lpthread

.PHONY: all build clean run-server

all: build

build: clean
	mkdir $(BIN)
	$(CXX) $(CXXFLAGS) -o $(BIN)/$(SERVER) $(SRC)/$(SERVER).cc $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -o $(BIN)/$(CLIENT) $(SRC)/$(CLIENT).cc $(LDFLAGS)

clean:
	rm -rf $(BIN)

run-server:
	$(BIN)/StartServer

run-client:
	$(BIN)/StartClient