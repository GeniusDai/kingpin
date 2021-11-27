BIN = bin
SERVER = StartServer

CXX = g++
CXXFLAGS = -O0 -Wall -std=c++11 -g -I ../../include

LDFLAGS = -lpthread

.PHONY: all build clean run-server

all: build

build: clean
	mkdir $(BIN)
	$(CXX) $(CXXFLAGS) -o $(BIN)/$(SERVER) $(SERVER).cc $(LDFLAGS)

clean:
	rm -rf $(BIN)

run-server:
	$(BIN)/StartServer
