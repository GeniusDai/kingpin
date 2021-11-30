include Makefile.inc

SUBDIRS=$(shell find . -maxdepth 1 -mindepth 1 -type d)

.PHONY: all build clean

all: $(SUBDIRS)

$(SUBDIRS): build
	make -C $@ $^
