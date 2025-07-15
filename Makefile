CC           := gcc
CFLAGS       := -std=c17 -Wall -Wextra -Werror -pedantic
OPTFLAGS     := -O3 -march=native -flto -mtune=native -funroll-loops
VERBOSE      := false

SRCS         := main.c lambda.c expr.c
OBJS         := $(SRCS:.c=.o)
DEPS         := $(SRCS:.c=.d)
TARGET       := lambda

.PHONY: all clean run quick debug profile lldb asm

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	@echo "Linking $@..."
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@)

%.o: %.c
	@echo "Compiling $<..."
	$(if $(VERBOSE),$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@,@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@)

quick: CFLAGS += -O1
quick: clean $(TARGET)

run: all
	./$(TARGET)

clean:
	@echo "Cleaning up..."
	rm -f $(TARGET) $(TARGET)_debug $(TARGET)_profile
	rm -f $(OBJS) $(DEPS)
	rm -f *.s