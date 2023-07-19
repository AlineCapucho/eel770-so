# build: gcc -std=c17 -pthread main.c -o main

# run: ./main (parâmetro 1 - N) (parâmetro 2 - Np) (parâmetro 3 - Nc)

# the compiler
CC = gcc

# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall -pthread

# the build target executable:
MAIN = main

all: $(MAIN)

$(MAIN): $(MAIN).c
	$(CC) $(CFLAGS) -o $(MAIN) $(MAIN).c

clean:
	$(RM) $(MAIN)