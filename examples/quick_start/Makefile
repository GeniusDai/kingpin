BIN = bin
SRC = src

CXX = g++
CXXFLAGS = -O0 -Wall -std=c++11 -g -I ../../include

LDFLAGS = -lpthread

.PHONY: all build clean run-server

all: build

build:
	mkdir $(BIN)
	$(CXX) $(CXXFLAGS) -o $(BIN)/test_logger $(SRC)/test_logger.cc $(LDFLAGS)

clean:
	rm -rf $(BIN)