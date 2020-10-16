# Makefile for the InitialParams, Params and ParamsGUI object classes
# NOTE: The LAST occurrence of a conflicting rule takes precedence.

%.o: $(RHSEG_DIR)/params/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

%.o: ../../hsegviewer/params/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS := initialParams.o params.o paramsGUI.o

all: $(OBJECTS)

