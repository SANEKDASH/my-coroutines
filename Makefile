SOURCES = main.c wp_coroutine.c task_queue.c 

FLAGS = -lc -Wall -O0

CC = gcc

EXECUTABLE = wpc

all:
	$(CC) $(FLAGS) $(SOURCES) -o $(EXECUTABLE)
