# Makefile for the Params and related object classes

%.o: $(RHSEG_DIR)/params/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS := params.o friends.o
ifeq ($(GTKMM_BUILD_FLAG), true)
  OBJECTS += initialGUI.o outputFileGUI.o paramsGUI.o expParamsGUI.o finalGUI.o
endif

all: $(OBJECTS)

