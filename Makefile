CC = gcc
CFLAGS = -g -Wall

seaflows: sflow.o collector.o
	$(CC) $(CFLAGS) -o bin/seaflows src/seaflows/collector.o src/sflow/sflow.o src/seaflows/main.c

collector.o:
	$(CC) $(CFLAGS) -c -o src/seaflows/collector.o -I src/ src/seaflows/collector.c

sflow.o:
	$(CC) $(CFLAGS) -c -o src/sflow/sflow.o -I src/ src/sflow/sflow.c

clean:
	rm src/seaflows/*.o
	rm src/sflow/*.o

distclean:
	rm bin/seaflows

