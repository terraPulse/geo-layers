# Makefile for objects used only for the region related objects

%.o: $(RHSEG_DIR)/region/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS = region_class.o region_seam.o region_object.o update_region_info.o \
          connected_component.o do_region_init.o merge_regions.o \
          update_nghbrs.o update_regions.o nghbrs_label_set.o

all: $(OBJECTS)

