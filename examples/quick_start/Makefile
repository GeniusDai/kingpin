include ../Makefile.inc

.PHONY: all build clean run-server

all: build

build: clean
	mkdir $(BIN)
	$(CXX) $(CXXFLAGS) -o $(BIN)/test_logger $(SRC)/test_logger.cc $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -o $(BIN)/test_server $(SRC)/test_server.cc $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -o $(BIN)/test_buffer $(SRC)/test_buffer.cc $(LDFLAGS)

clean:
	rm -rf $(BIN)