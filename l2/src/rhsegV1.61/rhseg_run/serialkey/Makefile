# Makefile for objects used only for the serialkey version of HSEG

%.o: $(RHSEG_DIR)/serialkey/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS = serialkey.o

all: $(OBJECTS)

