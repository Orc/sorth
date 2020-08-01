CC=cc
CFLAGS=-O
INSTALL=install -c
BINDIR=/usr/local/bin

sorth: sorth.c
	$(CC) $(CFLAGS) -o sorth sorth.c

install:
	$(INSTALL) sorth $(BINDIR)

clean:
	rm -f sorth sorth.o
