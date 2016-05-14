CFLAGS =  

LFLAGS= -lpthread -lrt -lm

all: blurc thresc

clean:
	-$(RM) *.o blurc thresc

blurc: ppmio.o gaussw.o blurfilter.o blurmain.o
	icpc -fast -o $@ -openmp ppmio.o gaussw.o blurfilter.o blurmain.o $(LFLAGS)

thresc: thresmain.o ppmio.o thresfilter.o
	icpc -fast -o $@ -openmp thresmain.o ppmio.o thresfilter.o $(LFLAGS)
%.o:%.c
	icpc -c -o $@ $^ $(CFLAGS) 
