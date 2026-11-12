CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
LDLIBS = -lboost_program_options -lcryptopp

SOURCES = src/user_interface.cpp src/credentials.cpp src/data_reader.cpp src/data_writer.cpp src/communicator.cpp
HEADERS = $(wildcard src/*.h)
TEST_SOURCES = $(wildcard tests/*.cpp)
TEST_HEADERS = $(wildcard tests/*.h)

client: src/main.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) src/main.cpp $(SOURCES) -o $@ $(LDLIBS)

unit_tests: $(TEST_SOURCES) $(TEST_HEADERS) $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -Isrc $(TEST_SOURCES) $(SOURCES) -o $@ $(LDLIBS) -lUnitTest++ -pthread

test: unit_tests
	./unit_tests

clean:
	rm -f client unit_tests

.PHONY: test clean
