CC := gcc

CFLAGS = -std=c17 -Wall -Wextra -Werror -pedantic -O3 -march=native -flto -mtune=native -funroll-loops

all: lambda

lambda: lambda.c
	$(CC) $(CFLAGS) lambda.c -o lambda

debug: lambda.c
	$(CC) -std=c17 -Wall -Wextra -pedantic -O0 -g -pg lambda.c -o lambda_debug

profile: lambda.c
	$(CC) -std=c17 -Wall -Wextra -pedantic -O3 -pg lambda.c -o lambda_profile

clean:
	rm -f lambda
	rm -f lambda_debug
	rm -f lambda_profile

run: lambda
	./lambda
	rm -f lambda

quick: lambda
	./lambda "* 20 20"
	rm -f lambda

.PHONY: all debug profile clean