CFLAGS = -Nmpi 

LFLAGS= -lpthread -lrt -lm

all: blurc thresc

clean:
	-$(RM) *.o blurc thresc

blurc: ppmio.o gaussw.o blurfilter.o blurmain.o
	mpiicpc -o $@ ppmio.o gaussw.o blurfilter.o blurmain.o $(LFLAGS)

thresc: thresmain.o ppmio.o thresfilter.o
	mpiicpc -o $@ thresmain.o ppmio.o thresfilter.o $(LFLAGS)
%.o:%.c
	mpiicpc -c -o $@ $^ $(CFLAGS) 
