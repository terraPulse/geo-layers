# Makefile for objects associated with the spatial object class

%.o: $(RHSEG_DIR)/spatial/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS := spatial.o find_section.o boundary_map.o update_region_label.o

ifneq ($(PARALLEL_BUILD_FLAG), true)
OBJECTS += save_restore_data.o
endif

all: $(OBJECTS)

