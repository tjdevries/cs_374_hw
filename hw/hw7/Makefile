TARGET = mpiArraySum
CC = mpicc
CFLAGS = -Wall -c -ansi -pedantic -std=c99 -g
LFLAGS = -o $(TARGET) -lm -g -ldl

$(TARGET): $(TARGET).o
	$(CC) $(TARGET).o $(LFLAGS)

$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) $(TARGET).c

clean:
	rm -f $(TARGET) *.o *~ *#
