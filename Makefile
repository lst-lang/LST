CC := gcc
CFLAGS += -ansi -Wall -pedantic -g
INCLUDES := execute.h boot.h optional.h
SRC := execute.c boot.c optional.c main.c
OBJ := execute.o boot.o optional.o main.o

%.o: %.c ${INCLUDES} Makefile
	$(CC) $(CFLAGS) -c $< -o $@

forth: $(OBJ)
	$(CC) $(LDFLAGS) -o forth $^

clean:
	rm -f $(OBJ) forth
