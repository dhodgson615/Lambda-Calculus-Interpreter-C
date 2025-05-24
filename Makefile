CFLAGS = -std=c17 -Wall -Wextra -Werror -pedantic -O3

all: lambda

lambda: lambda.c
	$(CC) $(CFLAGS) lambda.c -o lambda

clean:
	rm -f lambda
