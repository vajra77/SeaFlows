CC = gcc
CFLAGS = -g -Wall

collector: sflow.o collector.o server.o
	$(CC) $(CFLAGS) -o bin/collector src/collector/server.o src/collector/collector.o src/sflow/sflow.o

collector.o:
	$(CC) $(CFLAGS) -c -o src/collector/collector.o -I src/ src/collector/collector.c

server.o:
	$(CC) $(CFLAGS) -c -o src/collector/server.o -I src/ src/collector/server.c

sflow.o:
	$(CC) $(CFLAGS) -c -o src/sflow/sflow.o -I src/ src/sflow/sflow.c

clean:
	rm src/collector/*.o
	rm src/sflow/*.o

distclean:
	rm bin/collector

