CFLAGS = -I/usr/include/gdal -L/usr/lib -Wno-unused-result -march=native
CLDFLAGS = -lgdal -lm
BINDIR = ../../bin

all: create_mask match_reproj

create_mask: create_mask.c
	gcc create_mask.c $(CFLAGS) $(CLDFLAGS)  -o $(BINDIR)/create_mask
match_reproj: match_reproj.c
	gcc $(CFLAGS)  match_reproj.c $(CLDFLAGS) -o $(BINDIR)/match_reproj
