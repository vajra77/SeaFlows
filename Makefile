CC = gcc
CFLAGS = -Wall -g -DDEBUG
LIBS = -lrrd -lgc

all: sflow rrdtool bucket collector broker main
	$(CC) -o bin/seaflows src/sflow/sflow.o src/rrdtool/rrdtool.o src/bucket/bucket.o src/collector/collector.o src/broker/broker.o src/seaflows.o $(LIBS)

main:
	$(CC) $(CFLAGS) -c -o src/seaflows.o -I src/ src/seaflows.c

collector:
	$(CC) $(CFLAGS) -c -o src/collector/collector.o -I src/ src/collector/collector.c

broker:
	$(CC) $(CFLAGS) -c -o src/broker/broker.o -I src/ src/broker/broker.c

sflow:
	$(CC) $(CFLAGS) -c -o src/sflow/sflow.o -I src/ src/sflow/sflow.c

bucket:
	$(CC) $(CFLAGS) -c -o src/bucket/bucket.o -I src/ src/bucket/bucket.c

rrdtool:
	$(CC) $(CFLAGS) -c -o src/rrdtool/rrdtool.o -I src/ src/rrdtool/rrdtool.c

clean:
	rm src/collector/*.o
	rm src/broker/*.o
	rm src/sflow/*.o
	rm src/bucket/*.o
	rm src/rrdtool/*.o
	rm src/*.o

distclean: clean
	rm bin/seaflows
