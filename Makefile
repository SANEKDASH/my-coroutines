EXECUTABLE=coro

SOURCES=main.c my_coro/my_coro.c

FLAGS=-Wall -lc -lpthread

CC = gcc

all:
	$(CC) $(SOURCES) $(FLAGS) -o $(EXECUTABLE)

clean:
	$(RM) $(EXECUTABLE)
