#============================================================================#
#             JARVIS 2014-2019, IEETA/DETI, UNIVERSITY OF AVEIRO             #
#============================================================================#
BIN    = .
CC     = gcc
CPLP   = -ffast-math -msse2
#-----------------------------------------------------------------------------
CFLAGS = -O3 -Wall $(CPLP) -DPROGRESS
#-----------------------------------------------------------------------------
LIBS   = -lm
DEPS   = defs.h
PROGS  = $(BIN)/JARVIS
OBJS   = bitio.o arith.o arith_aux.o levels.o common.o msg.o mem.o dna.o \
         files.o strings.o args.o repeats.o pmodels.o buffer.o cm_tolerant.o \
         cm_array.o cm_hash.o cm.o
#-----------------------------------------------------------------------------
all:
	$(MAKE) progs
progs: $(PROGS)
$(BIN)/JARVIS: Jarvis.c $(DEPS) $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN)/JARVIS Jarvis.c $(OBJS) $(LIBS)
bitio.o: bitio.c bitio.h
	$(CC) -c $(CFLAGS) bitio.c
arith.o: arith.c arith.h
	$(CC) -c $(CFLAGS) arith.c
arith_aux.o: arith_aux.c arith_aux.h
	$(CC) -c $(CFLAGS) arith_aux.c
levels.o: levels.c levels.h
	$(CC) -c $(CFLAGS) levels.c
common.o: common.c common.h
	$(CC) -c $(CFLAGS) common.c
msg.o: msg.c msg.h
	$(CC) -c $(CFLAGS) msg.c
mem.o: mem.c mem.h
	$(CC) -c $(CFLAGS) mem.c
dna.o: dna.c dna.h
	$(CC) -c $(CFLAGS) dna.c
files.o: files.c files.h
	$(CC) -c $(CFLAGS) files.c
strings.o: strings.c strings.h
	$(CC) -c $(CFLAGS) strings.c
buffer.o: buffer.c buffer.h
	$(CC) -c $(CFLAGS) buffer.c
args.o: args.c args.h
	$(CC) -c $(CFLAGS) args.c
repeats.o: repeats.c repeats.h
	$(CC) -c $(CFLAGS) repeats.c
pmodels.o: pmodels.c pmodels.h
	$(CC) -c $(CFLAGS) pmodels.c
cm_tolerant.o: cm_tolerant.c cm_tolerant.h
	$(CC) -c $(CFLAGS) cm_tolerant.c
cm_array.o: cm_array.c cm_array.h
	$(CC) -c $(CFLAGS) cm_array.c
cm_hash.o: cm_hash.c cm_hash.h
	$(CC) -c $(CFLAGS) cm_hash.c
cm.o: cm.c cm.h
	$(CC) -c $(CFLAGS) cm.c
clean:
	/bin/rm -f *.o
cleanall:
	/bin/rm -f *.o $(PROGS)
#=============================================================================
