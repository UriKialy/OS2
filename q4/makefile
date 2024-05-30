CXX = g++
CXXFLAGS = -Wall -Wextra -g

SRCS = ttt.cpp mync.cpp
OBJS = $(SRCS:.cpp=.o)
TARGETS = ttt mync

all: $(TARGETS)

ttt: ttt.o
	$(CXX) $(CXXFLAGS) $^ -o $@

mync: mync.o
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean

clean:
	rm -f $(OBJS) $(TARGETS)
