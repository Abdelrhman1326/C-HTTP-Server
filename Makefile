CC       ?= cc
CFLAGS   ?= -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS ?= -Iinclude

BUILD_DIR := build
TARGET    := $(BUILD_DIR)/server
SRC       := $(wildcard src/*.c)
OBJ       := $(SRC:src/%.c=$(BUILD_DIR)/%.o)
DEP       := $(OBJ:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEP)

.PHONY: all run clean
