IDIR = include
CC = gcc
CFLAGS = -I$(IDIR)

ODIR = obj
LDIR = lib
SRCDIR = src

# LIBS = -lm

# _DEPS = hellomake.h
# DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = lex.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


# $(ODIR)/%.o: %.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(ODIR) && \
	$(CC) -c -o $@ $< $(CFLAGS)

# build: $(OBJ)
# 	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

lang: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
