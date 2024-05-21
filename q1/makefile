CXX = g++
CXXFLAGS = -Wall -Wextra -g

SRCS = ttt.cpp 
OBJS = $(SRCS:.cpp=.o)
TARGET = ttt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean

clean:
	rm -f $(OBJS) $(TARGET)
