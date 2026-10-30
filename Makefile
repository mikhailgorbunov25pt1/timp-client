CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
LDLIBS = -lboost_program_options -lcryptopp

SOURCES = src/user_interface.cpp src/credentials.cpp src/data_reader.cpp src/data_writer.cpp src/communicator.cpp
HEADERS = $(wildcard src/*.h)

client: src/main.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) src/main.cpp $(SOURCES) -o $@ $(LDLIBS)

clean:
	rm -f client

.PHONY: clean
