#ARITH_DIR = /home/x/public/DNA/arith
ARITH_DIR = ../../../../../../src/arith
ARITH_LIB = $(ARITH_DIR)/libArith.a
BIN = .

CC = gcc
CFLAGS = -g -O3 -Wall -I$(ARITH_DIR) -fno-strict-aliasing
LFLAGS = -L$(ARITH_DIR)

LIBS = -lArith -lm
DEPS = $(ARITH_LIB) defs.h

PROGS = $(BIN)/DNAEncR1

OBJS = mem.o template.o util.o fcm.o repeat.o

all:
	cd $(ARITH_DIR); $(MAKE)
	$(MAKE) progs

progs: $(PROGS)

$(BIN)/DNAEncR1: DNAEncR.c $(DEPS) $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $(BIN)/DNAEncR1 DNAEncR.c $(OBJS) $(LIBS)

mem.o: mem.c mem.h $(DEPS)
	$(CC) -c $(CFLAGS) mem.c

common.o: common.c common.h $(DEPS)
	$(CC) -c $(CFLAGS) common.c

fcm.o: fcm.c fcm.h $(DEPS)
	$(CC) -c $(CFLAGS) fcm.c

repeat.o: repeat.c repeat.h $(DEPS)
	$(CC) -c $(CFLAGS) repeat.c

util.o: util.c util.h $(DEPS)
	$(CC) -c $(CFLAGS) util.c

clean:
	/bin/rm -f *.o $(PROGS)

