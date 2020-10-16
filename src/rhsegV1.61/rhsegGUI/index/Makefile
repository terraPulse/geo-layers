# Makefile for objects used for the index related objects

%.o: $(RHSEG_DIR)/index/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS = index.o get_seam_index_data.o get_border_index_data.o

all: $(OBJECTS)

