CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -O3 -march=native
TARGET = billing_gateway
SRCS = src/mmap_reader.c src/csv_parser.c src/provider_adapters.c src/main.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) data/enterprise_billing.csv

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all run clean