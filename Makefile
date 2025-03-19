CC = gcc
CFLAGS = -O0 -g -Wall -shared
LIBS = -lrrd -lpthread

all: seaflows

seaflows: sflow queue matrix collector broker rrdtool
	$(CC) $(CFLAGS) $(LIBS) -o bin/seaflows -I src/ src/collector/collector.o src/broker/broker.o src/rrdtool/rrdtool.o src/sflow/sflow.o src/queue/queue.o src/matrix/matrix.o src/seaflows.c

collector:
	$(CC) $(CFLAGS) -c -o src/collector/collector.o -I src/ src/collector/collector.c

broker:
	$(CC) $(CFLAGS) -c -o src/broker/broker.o -I src/ src/broker/broker.c

sflow:
	$(CC) $(CFLAGS) -c -o src/sflow/sflow.o -I src/ src/sflow/sflow.c

queue:
	$(CC) $(CFLAGS) -c -o src/queue/queue.o -I src/ src/queue/queue.c

matrix:
	$(CC) $(CFLAGS) -c -o src/matrix/matrix.o -I src/ src/matrix/matrix.c

rrdtool:
	$(CC) $(CFLAGS) -c -o src/rrdtool/rrdtool.o -I src/ src/rrdtool/rrdtool.c

clean:
	rm src/collector/*.o
	rm src/sflow/*.o
	rm src/queue/*.o
	rm src/broker/*.o
	rm src/matrix/*.o
	rm src/rrdtool/*.o

distclean: clean
	rm bin/seaflows
