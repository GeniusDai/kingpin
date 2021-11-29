BIN = bin
SRC = src
CXX = g++
CXXFLAGS = -O0 -Wall -std=c++11 -g -I ../../include
LDFLAGS = -lpthread

export BIN SRC CXX CXXFLAGS LDFLAGS

SUBDIRS=$(shell find . -maxdepth 1 -mindepth 1 -type d)

.PHONY: all build clean

all: $(SUBDIRS)

$(SUBDIRS): build
	make -C $@ $^
