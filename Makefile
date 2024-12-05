CC = gcc
CFLAGS = -g -Wall

collector:
	$(CC) $(CFLAGS) -o bin/collector -I src/ src/collector/server.c src/collector/collector.c

clean:
	rm collector/*.o

distclean:
	rm bin/collector

