CC = cc

all : consumer producer

consumer : consumer.o queue.o communication.o
	$(CC) consumer.o queue.o communication.o -o consumer

producer : producer.o communication.o
	$(CC) producer.o communication.o -o producer

consumer.o : consumer.c structs.h prototypes.h constants.h
	$(CC) consumer.c -c

producer.o : producer.c structs.h prototypes.h constants.h
	$(CC) producer.c -c

queue.o : queue.c structs.h
	$(CC) queue.c -c

communication.o : communication.c
	$(CC) communication.c -c

clean:
	rm *.o
	touch *.c

pristine:
	rm *.o
	rm consumer
	rm producer
	touch *.c
