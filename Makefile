CC = gcc
CFLAGS = -g -Wall

all: seaflows

seaflows: sflow.o queue.o matrix.o collector.o broker.o
	$(CC) $(CFLAGS) -o bin/seaflows -I src/ src/collector/collector.o src/broker/broker.o src/sflow/sflow.o src/queue/queue.o src/matrix/matrix.o src/seaflows.c

collector.o:
	$(CC) $(CFLAGS) -c -o src/collector/collector.o -I src/ src/collector/collector.c

broker.o:
	$(CC) $(CFLAGS) -c -o src/broker/broker.o -I src/ src/broker/broker.c

sflow.o:
	$(CC) $(CFLAGS) -c -o src/sflow/sflow.o -I src/ src/sflow/sflow.c

queue.o:
	$(CC) $(CFLAGS) -c -o src/queue/queue.o -I src/ src/queue/queue.c

matrix.o:
	$(CC) $(CFLAGS) -c -o src/matrix/matrix.o -I src/ src/matrix/matrix.c

clean:
	rm src/collector/*.o
	rm src/sflow/*.o
	rm src/queue/*.o

distclean: clean
	rm bin/seaflows
