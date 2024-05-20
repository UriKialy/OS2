CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = q1.c
OBJS = $(SRCS:.c=.o)
TARGET = q1

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)