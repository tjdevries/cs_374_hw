TARGET = calcPI2
CC = gcc
CFLAGS = -Wall -ansi -pedantic -std=c99 -I/opt/openmpi/include
LFLAGS = -o $(TARGET) -lpthread -lmpi -lm

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(TARGET).c $(LFLAGS)

clean:
	rm -f $(TARGET) *.o *~ *#


