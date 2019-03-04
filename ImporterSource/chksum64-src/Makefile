
CC = gcc
CCFLAGS = -ansi -pedantic -Wall -Wshadow -O2
RM = rm -f

all: chksum64

clean:
	$(RM) chksum64

chksum64: chksum64.c
	$(CC) $(CCFLAGS) -o chksum64 chksum64.c
