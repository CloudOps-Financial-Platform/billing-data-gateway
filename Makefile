CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -O3 -march=native
SRC = src/mmap_reader.c src/csv_parser.c src/provider_adapters.c src/main.c
OBJ = $(SRC:.c=.o)
TARGET = billing_gateway

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	@bash tests/run_tests.sh

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean test