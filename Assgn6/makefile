# Create Static Library for goodmalloc.cpp

CC = g++
CFLAGS = -g -c
LFLAGS = -g

main:libgoodmalloc.a
	$(CC) $(LFLAGS) mergesort.cpp -L. -lgoodmalloc -o main

libgoodmalloc.a: goodmalloc.o
	ar rcs libgoodmalloc.a goodmalloc.o

goodmalloc.o: goodmalloc.cpp goodmalloc.h
	$(CC) $(CFLAGS) goodmalloc.cpp

clean:
	rm -f *.o *.a output.txt main