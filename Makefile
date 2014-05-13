IDIR = include
CC = gcc
CFLAGS = -I$(IDIR) -Wall

ODIR = obj
LDIR = lib
SRCDIR = src

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(ODIR)/%.o,$(SRC))

$(ODIR)/%.o: $(SRCDIR)/%.c
	mkdir -p "`dirname $@`" && \
	$(CC) -c -o $@ $< $(CFLAGS)

lang: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -rf $(ODIR) \
	rm lang \
	rm *~ core $(INCDIR)/*~
