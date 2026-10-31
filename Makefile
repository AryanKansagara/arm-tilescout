CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror
.PHONY: all test demo clean
all: build/tilescout
build:
	mkdir -p build
build/tilescout: main.cpp tilescout.hpp | build
	$(CXX) $(CXXFLAGS) main.cpp -o $@
build/tests: tests.cpp tilescout.hpp | build
	$(CXX) $(CXXFLAGS) tests.cpp -o $@
test: build/tests build/tilescout
	./build/tests
	./build/tilescout demo > /dev/null
demo: build/tilescout
	./build/tilescout demo
clean:
	rm -rf build
