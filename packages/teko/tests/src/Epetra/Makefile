.PHONY: all preproc clean

include ../../../make.include

#INCLUDE = $(shell perl $(TRILINOS_INCLUDE)/strip_dup_incl_paths.pl -I$(TRILINOS_INCLUDE) -I$(Teko_TESTS_INCLUDE) -I$(Teko_INCLUDE))
INCLUDE = -I$(Teko_TESTS_INCLUDE) -I$(Teko_INCLUDE) -I$(TRILINOS_INCLUDE)
CPPFLAGS=-DHAVE_CONFIG_H $(THYRA_CXXFLAGS) $(DEBUG_FLAGS)

OBJS = tEpetraOperatorWrapper.o \
       tStridedEpetraOperator.o \
       tEpetraThyraConverter.o \
       tInterlacedEpetra.o \
       tBlockingEpetra.o \
       tBlockedEpetraOperator.o

# tEpetraLSCIntegrationTest.o \

PREC = $(OBJS:%.o=%._tmp)

all: $(OBJS)
#	make -C ./NS
#	make -C ./Epetra

preproc: $(PREC)

$(PREC): %._tmp : %.cpp %.hpp
	cpp $(INCLUDE) $<

$(OBJS): %.o : %.cpp %.hpp
	$(CC) -c $(INCLUDE) $(CPPFLAGS) $< -o $@
	cp $@ $(Teko_DIR)/tests/objs/$@

clean:
	-rm -f $(OBJS) $(OBJS:%=$(Teko_DIR)/tests/objs/%) $(PREC)
