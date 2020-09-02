all:
	gcc -o forth system.c execute.c macro.c core.c optional.c bootstrp.c main.c -ansi -Wall -pedantic -g
