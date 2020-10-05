# Makefile for objects used only for the parallel version of HSEG

%.o: $(RHSEG_DIR)/parallel/%.cc
    $(CPP) -c $(CPPFLAGS) $(DEFINES) $(input) -o $(output)

OBJECTS = broadcast_params.o do_label_offset.o \
          recur_send.o recur_receive.o \
          parallel_requests.o parallel_server.o do_termination.o

all: $(OBJECTS)

