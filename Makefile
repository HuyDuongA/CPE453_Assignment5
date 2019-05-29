CC = gcc
CFLAGS = -std=c89 -g -Wall

all: minls minget 

minls: minls.o minls_func.o
	$(CC) $(CFLAGS) -o minls minls.o minls_func.o 

minls.o: minls.c
	$(CC) -c $(CFLAGS) minls.c

minls_func.o: minls_func.c
	$(CC) -c $(CFLAGS) minls_func.c -o minls_func.o 


minget: minget.o minget_func.o
	$(CC) $(CFLAGS) -o minget minget.o minget_func.o 

minget.o: minget.c
	$(CC) -c $(CFLAGS) minget.c

minget_func.o: minget_func.c
	$(CC) -c $(CFLAGS) minget_func.c -o minget_func.o 

clean:
	rm -f *.o minls minget
