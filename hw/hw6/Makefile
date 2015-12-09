TARGET = calcPI2
CC = gcc
CFLAGS = -Wall -ansi -pedantic -std=c99 -I/opt/openmpi/include
LFLAGS = -o $(TARGET) -lpthread -lmpi

$(TARGET): $(TARGET).c pthreadReduction.h
	$(CC) $(CFLAGS) $(TARGET).c $(LFLAGS)

clean:
	rm -f $(TARGET) *.o *~ *#


