# Makefile for objects used only for the serialkey version of HSEG

%.o: $(RHSEG_DIR)/serialkey/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS := serialkey.o

ifneq ($(SKEY_BUILD_FLAG), true)
  OBJECTS += serialkeyGUI.o
endif

all: $(OBJECTS)

