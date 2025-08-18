CC = gcc
CFLAGS = -Wall -g  
LIBS = -lrrd -lgc

all: sflow queue collector rrdtool main
	$(CC) -o bin/seaflows src/sflow/sflow.o src/rrdtool/rrdtool.o src/collector/collector.o src/seaflows.o $(LIBS)

main:
	$(CC) $(CFLAGS) -c -o src/seaflows.o -I src/ src/seaflows.c

collector: queue
	$(CC) $(CFLAGS) -c -o src/collector/collector.o -I src/ src/collector/collector.c

sflow:
	$(CC) $(CFLAGS) -c -o src/sflow/sflow.o -I src/ src/sflow/sflow.c

queue:
	$(CC) $(CFLAGS) -c -o src/queue/queue.o -I src/ src/queue/queue.c

rrdtool:
	$(CC) $(CFLAGS) -c -o src/rrdtool/rrdtool.o -I src/ src/rrdtool/rrdtool.c

clean:
	rm src/collector/*.o
	rm src/sflow/*.o
	rm src/queue/*.o
	rm src/rrdtool/*.o
	rm src/*.o

distclean: clean
	rm bin/seaflows
