# Makefile for objects used only for the region related objects

%.o: $(RHSEG_DIR)/region/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS := region_class.o region_object.o 

all: $(OBJECTS)

