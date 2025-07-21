# Compiler settings
CC          := gcc
CFLAGS      := -std=c23 -Wall -Wextra -Werror -pedantic
OPTFLAGS    := -O3 -march=native -flto -mtune=native -funroll-loops \
               -fomit-frame-pointer -pipe -ffast-math \
               -ffunction-sections -fdata-sections

# Platform-specific settings
UNAME_S     := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
 LDFLAGS := -Wl,-dead_strip
else
 LDFLAGS := -Wl,--gc-sections -Wl,-O1
endif

# Build configuration
VERBOSE     := false
Q           := $(if $(filter true,$(VERBOSE)),,@)

# Directory structure
SRC_DIR     := src
TEST_DIR    := test
OBJ_DIR     := objects
BUILD_DIR   := build
ASM_DIR     := asm
DIRS        := $(SRC_DIR) $(TEST_DIR) $(OBJ_DIR) $(BUILD_DIR) $(ASM_DIR)
BUILD_DIRS  := $(OBJ_DIR) $(BUILD_DIR)

# Source files and targets
SRCS        := $(wildcard $(SRC_DIR)/*.c)
OBJS        := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS        := $(OBJS:.o=.d)
TARGET      := $(BUILD_DIR)/lambda

# Test files
TEST_SRCS   := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS   := $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/%.o,$(TEST_SRCS))
COMMON_SRCS := $(filter-out $(SRC_DIR)/main.c,$(SRCS))
COMMON_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(COMMON_SRCS))
TEST_TARGET := $(BUILD_DIR)/test

# Assembly files
ASM_FILES   := $(patsubst $(SRC_DIR)/%.c,$(ASM_DIR)/%.s,$(SRCS))

.PHONY: all clean run quick debug profile lldb asm test dirs build_dirs clean_empty

# Default target
all: build_dirs $(TARGET) clean_empty
	@echo "Build complete: $(TARGET)"

# Create all directories
dirs:
	@mkdir -p $(DIRS)
	@echo "Creating directories: $(DIRS)"

# Create only directories needed for build
build_dirs:
	@mkdir -p $(BUILD_DIRS)
	@echo "Creating build directories: $(BUILD_DIRS)"

# Remove empty directories
clean_empty:
	@echo "Removing empty directories..."
	@for dir in $(DIRS); do \
		if [ -d "$$dir" ] && [ -z "$$(ls -A "$$dir" 2>/dev/null)" ]; then \
			rmdir "$$dir" && echo "Removed empty directory: $$dir"; \
		fi; \
	done

# Include generated dependencies
-include $(DEPS)

# Compilation rules
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	$Qmkdir -p $(dir $@)
	$Q$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	@echo "Compiling $<..."
	$Qmkdir -p $(dir $@)
	$Q$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@

# Linking rules
$(TARGET): $(OBJS)
	@echo "Linking $@..."
	$Q$(CC) $(CFLAGS) $(OPTFLAGS) $(LDFLAGS) $^ -o $@

$(TEST_TARGET): $(TEST_OBJS) $(COMMON_OBJS)
	@echo "Linking $@..."
	$Q$(CC) $(CFLAGS) $(OPTFLAGS) $(LDFLAGS) $^ -o $@

# Assembly generation
$(ASM_DIR)/%.s: $(SRC_DIR)/%.c
	$Qmkdir -p $(dir $@)
	@echo "Generating assembly for $<..."
	$Q$(CC) $(CFLAGS) $(OPTFLAGS) -S -o $@ $<

# Build targets
asm: dirs $(ASM_FILES) clean_empty
	@echo "Assembly files generated in $(ASM_DIR)/"

test: dirs $(TEST_TARGET) clean_empty
	$(TEST_TARGET)

# Build variations
quick: CFLAGS += -O1
quick: clean all

debug: CFLAGS += -g -O0
debug: clean all

profile: CFLAGS += -pg
profile: LDFLAGS += -pg
profile: clean all

lldb: debug

run: all
	$(TARGET)

# Cleanup
clean:
	@echo "Cleaning up..."
	$(RM) -r $(OBJ_DIR) $(BUILD_DIR) $(ASM_DIR)