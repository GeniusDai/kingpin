SRC = src
BIN = bin
SERVER = StartServer
CLIENT = StartClient
TEST = StartTest

CXX = g++
CXXFLAGS = -O0 -Wall -Wno-write-strings -Wno-sign-compare -std=c++11 -g -I ../../include

LDFLAGS = -lpthread

.PHONY: all build clean run-server run-client

all: build

build: clean
	mkdir $(BIN)
	$(CXX) $(CXXFLAGS) -o $(BIN)/$(TEST) $(SRC)/$(TEST).cc $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -o $(BIN)/$(SERVER) $(SRC)/$(SERVER).cc $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -o $(BIN)/$(CLIENT) $(SRC)/$(CLIENT).cc $(LDFLAGS)

clean:
	rm -rf $(BIN)

run-server:
	$(BIN)/StartServer

run-client:
	$(BIN)/StartClient
