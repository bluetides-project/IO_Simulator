MPICC?=mpicc
CFLAGS?=-O0 -g

IOSIM_OBJECTS = iosim.o ../bigfile/src/bigfile.o ../bigfile/src/bigfile-mpi.o

all : iosim

iosim : $(IOSIM_OBJECTS)
	$(MPICC) -o $@ $^

%.o : %.c
	$(MPICC) -I../bigfile $(CFLAGS) -c -o $@ $^


