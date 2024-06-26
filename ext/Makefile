CC = clang
CFLAGS = -Wall -O2
LDFLAGS = -framework CoreServices
TARGET = fsevent
SRC = io-fsevent.c
OBJ = $(SRC:.c=.o)

# Default target
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# To create object files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Clean Target
clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
