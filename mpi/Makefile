
CFLAGS = 

LFLAGS= -lpthread -lrt -lm

all: thresc #blurc

clean:
	-$(RM) *.o blur thresc

#blurc: ppmio.o gaussw.o blurfilter.o blurmain.o
#	mpic++ -o $@ ppmio.o gaussw.o blurfilter.o blurmain.o $(LFLAGS)

thresc: thresmain.o ppmio.o thresfilter.o
	gcc -o -fopenmp $@ thresmain.o ppmio.o thresfilter.o $(LFLAGS)
	
%.o:%.c
	mpic++ -c -o $@ $^ $(CFLAGS)

arc:
	tar cf - *.c *.h *.f90 Makefile Makefile.sgi|gzip - > filters.tar.gz
