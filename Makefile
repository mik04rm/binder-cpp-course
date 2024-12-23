CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++20
TARGETS = binder_example.e binder_example_1.e
SOURCES = binder_example.cpp binder_example_1.cpp
HEADERS = binder.h

all: $(TARGETS)

%.e: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean