CC           := gcc
CFLAGS       := -std=c17 -Wall -Wextra -Werror -pedantic
OPTFLAGS     := -O3 -march=native -flto -mtune=native -funroll-loops
VERBOSE      := false

SRCS         := main.c lambda.c expr.c
TEST_SRCS    := test.c lambda.c expr.c
OBJS         := $(SRCS:.c=.o)
TEST_OBJS    := $(TEST_SRCS:.c=.o)
DEPS         := $(SRCS:.c=.d)
TARGET       := lambda
TEST_TARGET  := test_lambda

.PHONY: all clean run quick debug profile lldb asm test

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	@echo "Linking $@..."
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@)

%.o: %.c
	@echo "Compiling $<..."
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@)

$(TEST_TARGET): $(TEST_OBJS)
	@echo "Linking $@..."
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@)

test: $(TEST_TARGET)
	./$(TEST_TARGET)
	make clean

quick: CFLAGS += -O1
quick: clean $(TARGET)

run: all
	./$(TARGET)

clean:
	@echo "Cleaning up..."
	rm -f $(TARGET) $(TARGET)_debug $(TARGET)_profile $(TEST_TARGET)
	rm -f $(OBJS) $(TEST_OBJS) $(DEPS)
	rm -f *.s
	rm -f *.d