all:
	gcc main.c -o main -Wall -Wextra -lraylib -lm -lGL -lpthread -lX11 -ldl -lrt
