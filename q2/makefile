CXX = g++
CXXFLAGS = -Wall -Wextra -g

# Source files
SRCS_TTT = ttt.cpp
SRCS_MYNC = mync.cpp

# Object files
OBJS_TTT = $(SRCS_TTT:.cpp=.o)
OBJS_MYNC = $(SRCS_MYNC:.cpp=.o)

# Executables
TARGET_TTT = ttt
TARGET_MYNC = mync

# Default target
all: $(TARGET_TTT) $(TARGET_MYNC)

# Rule to build each target executable
$(TARGET_TTT): $(OBJS_TTT)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TARGET_MYNC): $(OBJS_MYNC)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Rule to compile each source file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean

# Clean up build files
clean:
	rm -f $(OBJS_TTT) $(OBJS_MYNC) $(TARGET_TTT) $(TARGET_MYNC)
