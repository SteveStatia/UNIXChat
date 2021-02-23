CFLAGS = -Wall -g -D _POSIX_C_SOURCE=200809L -Werror

all: build

build:
	gcc $(CFLAGS) main.c receiver.c keyboard.c printer.c sender.c instructorList.o -lpthread -o s-talk


run: build
	./main

valgrind: build
	valgrind --leak-check=full ./s-talk 4444 asb9700u-a04 4242

clean:
	rm -f s-talk
