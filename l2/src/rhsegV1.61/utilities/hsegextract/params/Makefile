# Makefile for the Params and related object classes
# NOTE: The LAST occurrence of a conflicting rule takes precedence.

%.o: $(RHSEG_DIR)/params/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

%.o: %.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS := initialParams.o params.o
ifeq ($(GTKMM_BUILD_FLAG), true)
  OBJECTS += paramsGUI.o
endif

all: $(OBJECTS)

