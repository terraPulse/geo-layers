# Makefile for the Params object classes

%.o: $(RHSEG_DIR)/params/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS = params.o friends.o

all: $(OBJECTS)

