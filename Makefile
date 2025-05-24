CC := gcc

CFLAGS = -std=c17 -Wall -Wextra -Werror -pedantic -O3 -march=native -flto -mtune=native -funroll-loops

all: lambda

lambda: lambda.c
	$(CC) $(CFLAGS) lambda.c -o lambda

clean:
	rm -f lambda
