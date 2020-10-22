all:
	gcc -o forth execute.c boot.c optional.c main.c -ansi -Wall -pedantic -g
