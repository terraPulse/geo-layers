# Makefile for objects used only for the pixel related objects

%.o: $(RHSEG_DIR)/pixel/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS = pixel.o do_region_relabel.o

all: $(OBJECTS)

