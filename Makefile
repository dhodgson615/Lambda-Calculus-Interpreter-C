# Compiler settings
CC          := gcc
CFLAGS      := -std=c23 -Wall -Wextra -Werror -pedantic -Iinclude
OFLAGS      := -O3 -march=native -flto -mtune=native -funroll-loops           \
               -fomit-frame-pointer -pipe -ffast-math                         \
               -ffunction-sections -fdata-sections

# Platform-specific settings
UNAME_S     := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
 LDFLAGS    := -Wl,-dead_strip
else
 LDFLAGS    := -Wl,--gc-sections -Wl,-O1
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
ASM_FILES   := $(patsubst $(SRC_DIR)/%.c,$(ASM_DIR)/%.s,$(SRCS))

.PHONY: all clean run quick debug profile lldb asm test dirs build_dirs clean_empty

all: build_dirs $(TARGET) clean_empty
	@echo "Build complete: $(TARGET)"

dirs:
	@mkdir -p $(DIRS)
	@echo "Creating directories: $(DIRS)"

build_dirs:
	@mkdir -p $(BUILD_DIRS)
	@echo "Creating build directories: $(BUILD_DIRS)"

clean_empty:
	@echo "Removing empty directories..."
	@for dir in $(DIRS); do \
		if [ -d "$$dir" ] && [ -z "$$(ls -A "$$dir" 2>/dev/null)" ]; then \
			rmdir "$$dir" && echo "Removed empty directory: $$dir"; \
		fi; \
	done

-include $(DEPS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	$Qmkdir -p $(dir $@)
	$Q$(CC) $(CFLAGS) $(OFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	@echo "Compiling $<..."
	$Qmkdir -p $(dir $@)
	$Q$(CC) $(CFLAGS) $(OFLAGS) -MMD -MP -c $< -o $@

$(TARGET): $(OBJS)
	@echo "Linking $@..."
	$Q$(CC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $^ -o $@

$(TEST_TARGET): $(TEST_OBJS) $(COMMON_OBJS)
	@echo "Linking $@..."
	$Q$(CC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $^ -o $@

test: build_dirs $(TEST_TARGET) clean_empty
	@echo "Running tests..."
	$Q$(TEST_TARGET)

$(ASM_DIR)/%.s: $(SRC_DIR)/%.c
	$Qmkdir -p $(dir $@)
	@echo "Generating assembly for $<..."
	$Q$(CC) $(CFLAGS) $(OFLAGS) -S -o $@ $<

asm: CFLAGS += -g -fverbose-asm
asm: dirs clean_empty
	@echo "Generating individual assembly files..."
	$(MAKE) $(ASM_FILES)
	@echo "Generating combined program assembly..."
	$Qmkdir -p $(ASM_DIR)
	@echo "Creating combined source file..."
	$Q(for src in $(SRCS); do echo "/* ---- $$src ---- */"; cat "$$src"; echo; done) > $(ASM_DIR)/combined_source.c
	$Q$(CC) $(CFLAGS) $(OFLAGS) -I$(SRC_DIR) -S $(ASM_DIR)/combined_source.c -o $(ASM_DIR)/program.s
	$Q$(RM) $(ASM_DIR)/combined_source.c
	@echo "Assembly files generated in $(ASM_DIR)/"
	@echo "Full program assembly available at $(ASM_DIR)/program.s"

debug: CFLAGS += -g -O0
debug: clean all

lldb: debug
	@echo "Starting LLDB..."
	@lldb $(TARGET)

rebuild: clean all
	@echo "Project rebuild complete."

run: all
	$(TARGET)

clean:
	@echo "Cleaning up..."
	$(RM) -r $(OBJ_DIR) $(BUILD_DIR) $(ASM_DIR)