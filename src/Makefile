MPICC?=mpicc
CFLAGS?=-O0 -g

IOSIM_OBJECTS = iosim.o ../bigfile/bigfile.o ../bigfile/bigfile-mpi.o

all : iosim

iosim : $(IOSIM_OBJECTS)
	$(MPICC) -o $@ $^

%.o : %.c
	$(MPICC) -I../bigfile $(CFLAGS) -c -o $@ $^


