CC = gcc
CFLAGS = -g -Wall

seaflows: sflow.o collector.o queue.o
	$(CC) $(CFLAGS) -o bin/seaflows src/seaflows/collector.o src/sflow/sflow.o src/seaflows/main.c

collector.o:
	$(CC) $(CFLAGS) -c -o src/seaflows/collector.o -I src/ src/seaflows/collector.c

sflow.o:
	$(CC) $(CFLAGS) -c -o src/sflow/sflow.o -I src/ src/sflow/sflow.c

queue.o:
	$(CC) $(CFLAGS) -c -o src/queue/queue.o -I src/ src/queue/queue.c

clean:
	rm src/seaflows/*.o
	rm src/sflow/*.o
	rm src/queue/*.o

distclean:
	rm bin/seaflows

