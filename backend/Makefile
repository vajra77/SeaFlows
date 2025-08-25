CC = gcc
CFLAGS = -Wall -g -DSFLOW_DEBUG
LIBS = -lrrd

all: gc sflow rrdtool bucket collector broker main
	$(CC) -o bin/seaflows src/gc.o src/sflow.o src/rrdtool.o src/bucket.o src/collector.o src/broker.o src/seaflows.o $(LIBS)

main:
	$(CC) $(CFLAGS) -c -o src/seaflows.o -I include/ src/seaflows.c

collector:
	$(CC) $(CFLAGS) -c -o src/collector.o -I include/ src/collector.c

broker:
	$(CC) $(CFLAGS) -c -o src/broker.o -I include/ src/broker.c

sflow:
	$(CC) $(CFLAGS) -c -o src/sflow.o -I include/ src/sflow.c

bucket:
	$(CC) $(CFLAGS) -c -o src/bucket.o -I include/ src/bucket.c

rrdtool:
	$(CC) $(CFLAGS) -c -o src/rrdtool.o -I include/ src/rrdtool.c

gc:
	$(CC) $(CFLAGS) -c -o src/gc.o -I include/ src/gc.c

clean:
	rm src/*.o

distclean: clean
	rm bin/seaflows
