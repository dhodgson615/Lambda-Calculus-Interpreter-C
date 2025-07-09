CC := gcc

CFLAGS = -std=c17 -Wall -Wextra -Werror -pedantic -O3 -march=native -flto -mtune=native -funroll-loops

all: lambda

lambda: main.o lambda.o
	$(CC) $(CFLAGS) main.o lambda.o -o lambda

main.o: main.c lambda.h
	$(CC) $(CFLAGS) -c main.c -o main.o

lambda.o: lambda.c lambda.h
	$(CC) $(CFLAGS) -c lambda.c -o lambda.o

debug: main.c lambda.c
	$(CC) -std=c17 -Wall -Wextra -pedantic -O0 -g -pg main.c lambda.c -o lambda_debug

profile: main.c lambda.c
	$(CC) -std=c17 -Wall -Wextra -pedantic -O3 -pg main.c lambda.c -o lambda_profile

clean:
	rm -f lambda
	rm -f lambda_debug
	rm -f lambda_profile
	rm -f *.o

run: lambda
	make clean
	make
	./lambda
	rm -f lambda
	rm -f *.o

quick: lambda
	make clean
	make
	gtime -v ./lambda "* 50 50"
	rm -f lambda
	rm -f *.o

lldb: debug
	make clean
	make debug
	lldb lambda_debug
	./lambda_debug
	rm -f lambda_debug
	rm -f *.o

.PHONY: all debug profile clean