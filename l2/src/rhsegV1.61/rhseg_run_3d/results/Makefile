# Makefile for objects used only for the results related objects

%.o: $(RHSEG_DIR)/results/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS = results.o

all: $(OBJECTS)

