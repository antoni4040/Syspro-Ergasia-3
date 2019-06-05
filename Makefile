OBJS1   =	server.o	linked_list.o	requests.o
OBJS2   =	client.o	linked_list.o	requests.o	threads.o
SOURCE  =	server.c	client.c	linked_list.c	requests.c	threads.c
HEADER  =	linked_list.h	requests.h	threads.h
OUT1    =	server
OUT2    =	client
CC      =	gcc
FLAGS   =       -Wall   -g      -c      -std=c99	-D_XOPEN_SOURCE=700		-lpthread

all: $(OUT1) $(OUT2)

$(OUT1): $(OBJS1)
	$(CC)   -g      $(OBJS1) -o      $@

$(OUT2): $(OBJS2)
	$(CC) -lpthread   -g      $(OBJS2) -o      $@

server.o: server.c
	$(CC)   $(FLAGS)        server.c

client.o: client.c
	$(CC)   $(FLAGS)        client.c

requests.o: requests.c
	$(CC)   $(FLAGS)        requests.c

threads.o: threads.c
	$(CC)   $(FLAGS)        threads.c

linked_list.o: linked_list.c
	$(CC)   $(FLAGS)        linked_list.c


clean:
	rm -f $(OBJS1) $(OUT1) $(OBJS2) $(OUT2)

count:
	wc $(SOURCE) $(HEADER)

