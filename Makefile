CC           := gcc
CFLAGS       := -std=c17 -Wall -Wextra -Werror -pedantic
OPTFLAGS     := -O3 -march=native -flto -mtune=native -funroll-loops
VERBOSE      := false

# Directory structure
SRC_DIR      := src
TEST_DIR     := test
OBJ_DIR      := objects
BUILD_DIR    := build

# Source files
SRCS         := $(SRC_DIR)/main.c $(SRC_DIR)/lambda.c $(SRC_DIR)/expr.c $(SRC_DIR)/strbuf.c
TEST_SRCS    := $(TEST_DIR)/test.c $(SRC_DIR)/lambda.c $(SRC_DIR)/expr.c $(SRC_DIR)/strbuf.c

# Object files with directory prefix
OBJS         := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TEST_OBJS    := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/%.o,$(TEST_SRCS)))

# Dependency files
DEPS         := $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# Target executables
TARGET       := $(BUILD_DIR)/lambda
TEST_TARGET  := $(BUILD_DIR)/test_lambda

.PHONY: all clean run quick debug profile lldb asm test dirs

all: dirs $(TARGET)

# Create required directories
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(TEST_DIR)

-include $(DEPS)

$(TARGET): $(OBJS)
	@echo "Linking $@..."
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@)

# Compile source files in src directory
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@)

$(OBJ_DIR)/lambda.o: $(SRC_DIR)/lambda.c $(SRC_DIR)/lambda.h $(SRC_DIR)/expr.h $(SRC_DIR)/strbuf.h
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@)

$(OBJ_DIR)/expr.o: $(SRC_DIR)/expr.c $(SRC_DIR)/expr.h
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@)

$(OBJ_DIR)/strbuf.o: $(SRC_DIR)/strbuf.c $(SRC_DIR)/strbuf.h
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@)

$(OBJ_DIR)/test.o: $(TEST_DIR)/test.c $(SRC_DIR)/expr.h $(SRC_DIR)/lambda.h $(SRC_DIR)/strbuf.h
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@)

$(TEST_TARGET): $(TEST_OBJS)
	@echo "Linking $@..."
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@)

test: dirs $(TEST_TARGET)
	$(TEST_TARGET)

quick: CFLAGS += -O1
quick: clean $(TARGET)

run: all
	$(TARGET)

clean:
	@echo "Cleaning up..."
	$(RM) -r $(OBJ_DIR)
	$(RM) -r $(BUILD_DIR)