
CC = gcc
CFLAGS = -std=c11
SRC = main.c Shell.c ShellCommands.c
BUILD_DIR = build
OBJ = $(addprefix $(BUILD_DIR)/, $(SRC:.c=.o))
TARGET = $(BUILD_DIR)/main

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
