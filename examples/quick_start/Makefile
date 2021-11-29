.PHONY: all build clean run-server

all: build

build: clean
	mkdir $(BIN)
	$(CXX) $(CXXFLAGS) -o $(BIN)/test_logger $(SRC)/test_logger.cc $(LDFLAGS)

clean:
	rm -rf $(BIN)