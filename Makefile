CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++20
TARGET = binder_example.e
SOURCES = binder_example.cpp
HEADERS = binder.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: all clean