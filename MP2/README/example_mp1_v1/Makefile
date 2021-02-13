.PHONY: clean all

all: loser md5.o mp1.o

clean:
	rm -f loser *.o

md5.o: md5.c md5.h
	gcc -c md5.c

mp1.o: mp1.c
	gcc -c mp1.c

loser: mp1.o md5.o
	gcc -o loser md5.o mp1.o -Wall
