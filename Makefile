CFLAGS = -std=c17 -Wall -Wextra -Werror -pedantic -O3

all: lambda

lambda: lambda.c
\t$(CC) $(CFLAGS) lambda.c -o lambda

clean:
\trm -f lambda
