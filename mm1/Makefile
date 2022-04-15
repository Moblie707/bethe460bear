CC = cc

all : consumer

consumer : consumer.o queue.o
	$(CC) consumer.o queue.o -o consumer

consumer.o : consumer.c structs.h prototypes.h
	$(CC) consumer.c -c

queue.o : queue.c structs.h
	$(CC) queue.c -c

clean:
	rm *.o
	touch *.c

pristine:
	rm *.o
	rm consumer
	touch *.c
