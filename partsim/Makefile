CPP=$(if $(INTEL), icpc, g++)
MPICPP=$(if $(INTEL), icpc, mpic++)

CFLAGS  += -tcollect -I$(VT_ROOT)/include
FFLAGS  += -tcollect -I$(VT_ROOT)/include
LFLAGS  += -L$(VT_LIB_DIR) -lVT $(VT_ADD_LIBS)

SRC=physics.c
OBJS=$(SRC:.cpp=.o)
TARGETS=partsim

all: $(TARGETS) objclean

partsim: $(OBJS) main.cpp
	icpc -Nmpi -g $(CFLAGS) $(MPIFLAGS) $(FFLAGS) $(OBJS) \
		main.cpp -o partsim $(LFLAGS)

%.o: %.cpp
	$(CPP) $(CFLAGS) -c $<

objclean:
	$(RM) *.o

clean: objclean
	$(RM) -R partsim only-set-in-job-environment *.stf* *.prot
