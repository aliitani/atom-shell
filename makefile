all:
	gcc -Wall -Wpedantic -std=c99 ant_shell.c -g -o ant_shell
run: 
	./ant_shell
