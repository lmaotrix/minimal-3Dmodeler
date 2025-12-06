CC = gcc
CFLAGS = -Wall -Wextra -std=c17
TARGET = main
SOURCE = main.c

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET) $(TARGET).o

.PHONY: clean

