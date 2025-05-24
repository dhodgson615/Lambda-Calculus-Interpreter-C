CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic

all: lambda

lambda: lambda.c
\t$(CC) $(CFLAGS) lambda.c -o lambda

clean:
\trm -f lambda
