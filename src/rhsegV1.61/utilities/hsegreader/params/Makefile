# Makefile for the InitialParams, Params and ParamsGUI object classes

%.o: $(RHSEG_DIR)/params/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

%.o: %.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS := initialParams.o params.o paramsGUI.o

all: $(OBJECTS)

