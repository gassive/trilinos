.PHONY: all preproc clean

include ../../make.include

#INCLUDE = $(shell perl $(TRILINOS_INCLUDE)/strip_dup_incl_paths.pl -I$(TRILINOS_INCLUDE) -I$(Teko_TESTS_INCLUDE) -I$(Teko_INCLUDE))
INCLUDE = -I$(Teko_TESTS_INCLUDE) -I$(Teko_INCLUDE) -I$(TRILINOS_INCLUDE) 
CPPFLAGS=-DHAVE_CONFIG_H $(THYRA_CXXFLAGS) $(DEBUG_FLAGS)

OBJS = tLSCStablePreconditionerFactory.o \
       tLSCStabilized.o \
       tJacobi2x2PreconditionerFactory.o \
       tBlockJacobiPreconditionerFactory.o \
       tBlockUpperTriInverseOp.o \
       tBlockLowerTriInverseOp.o \
       tLSCIntegrationTest.o \
       tLSCHIntegrationTest.o \
       tGraphLaplacian.o \
       tLU2x2PreconditionerFactory.o \
       tParallelInverse.o \
       tSIMPLEPreconditionerFactory.o \
       tExplicitOps.o \
       tLumping.o \
       tAbsRowSum.o \
       tNeumannSeries.o \
       tPCDStrategy.o

PREC = $(OBJS:%.o=%._tmp)

all: $(OBJS)
#	make -C ./NS
	make -C ./Epetra

preproc: $(PREC)

$(PREC): %._tmp : %.cpp %.hpp
	cpp $(INCLUDE) $<

$(OBJS): %.o : %.cpp %.hpp
	$(CC) -c $(INCLUDE) $(CPPFLAGS) $< -o $@
	cp $@ $(Teko_DIR)/tests/objs/$@

clean:
	-rm -f $(OBJS) $(OBJS:%=$(Teko_DIR)/tests/objs/%) $(PREC)
	make -C ./Epetra clean
#	make -C ./NS clean
