CC = gcc
CFLAGS = -g -O0 -Wall -Wextra -Wno-unused-parameter -pedantic -Werror -ICli
SRC = main.c Cli/Cli.c Cli/CliBindings.c
BUILD_DIR = build
OBJ = $(BUILD_DIR)/main.o $(BUILD_DIR)/Cli.o $(BUILD_DIR)/CliBindings.o
TARGET = $(BUILD_DIR)/firmware-cli

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/main.o: main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/Cli.o: Cli/Cli.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/CliBindings.o: Cli/CliBindings.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
