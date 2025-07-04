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
