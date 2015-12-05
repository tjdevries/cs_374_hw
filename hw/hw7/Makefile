TARGET = mpiArraySum
TARGETO = ompArraySum
CC = mpicc
CFLAGS = -Wall -c -ansi -pedantic -std=c99 -g
LFLAGS = -o $(TARGET) -lm -g -ldl

all: $(TARGET) $(TARGETO)

$(TARGET): $(TARGET).o
	$(CC) $(TARGET).o $(LFLAGS)

$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) $(TARGET).c

$(TARGETO): $(TARGETO).c
	gcc -o $(TARGETO) -fopenmp $(TARGETO).c

clean:
	rm -f $(TARGET) *.o *~ *#
	rm -f $(TARGETO) *.o *~ *#
