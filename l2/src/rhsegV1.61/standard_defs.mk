COMMON_DIR := ../CommonV1.70
RHSEG_DIR := ../rhsegV1.61
INSTALL_DIR := ../../bin
ifeq ($(NOSKEY), true)
  SKEY_BUILD_FLAG := false
else
  SKEY_BUILD_FLAG := true
endif
ifeq ($(NOGTKMM), true)
  GTKMM_BUILD_FLAG := false
else
  GTKMM_BUILD_FLAG := true
endif
ifeq ($(NOGDAL), true)
  GDAL_BUILD_FLAG := false
else
  GDAL_BUILD_FLAG := true
endif
ifeq ($(NOSHAPE), true)
  SHAPE_BUILD_FLAG := false
else
  SHAPE_BUILD_FLAG := $(GDAL_BUILD_FLAG)
endif
ifeq ($(NOPTHREADS), true)
  PTHREADS_BUILD_FLAG := false
else
  PTHREADS_BUILD_FLAG := $(GTKMM_BUILD_FLAG)
endif

ifeq ($(GTKMM_BUILD_FLAG), true)
  ifeq ($(dir_noslash $(findfile gtkmm-3.0.pc, $(PKG_CONFIG_PATH) \
                        /usr/local/lib/pkgconfig /usr/lib/pkgconfig \
                        /usr/lib/pkgconfig /usr/share/pkgconfig)), .)
    GTKMM3_BUILD_FLAG = false
    ifeq ($(dir_noslash $(findfile gtkmm-2.4.pc, $(PKG_CONFIG_PATH)\
                          /usr/local/lib/pkgconfig /usr/lib/pkgconfig \
                          /usr/lib/pkgconfig /usr/share/pkgconfig)), .)
      GTKMM_VERSION := 0.0
      &echo "gtkmm not installed on your system."
      &echo "Only those programs not requiring gtkmm will be built."
      GTKMM_BUILD_FLAG = false
    else
      GTKMM_BUILD_FLAG = true
      GTKMM_VERSION != pkg-config --modversion gtkmm-2.4
      &echo "Found gtkmm version $(GTKMM_VERSION)"
    endif
  else
    GTKMM3_BUILD_FLAG = true
    GTKMM_VERSION != pkg-config --modversion gtkmm-3.0
    &echo "Found gtkmm version $(GTKMM_VERSION)"
  endif
else
  GTKMM3_BUILD_FLAG = false
endif

ifeq ($(GDAL_BUILD_FLAG), true)
  ifeq ($(find_program gdal-config), gdal-config)
    GDAL_VERSION != gdal-config --version
    &echo "Found GDAL version $(GDAL_VERSION)"
  else
    GDAL_VERSION := 0.0
    &echo "GDAL not installed on your system."
    &echo "Only those programs not requiring GDAL will be built."
    GDAL_BUILD_FLAG := false
    SHAPE_BUILD_FLAG := false
  endif
endif

ifeq ($(SHAPE_BUILD_FLAG), true)
  ifeq ($(dir_noslash $(findfile shapefil.h, /usr/local/include /usr/local/include/libshp /usr/include)), .)
    &echo "Shapefile software not installed on your system."
    &echo "Only those programs not requiring Shapefile software will be built."
    SHAPE_BUILD_FLAG := false
  endif
endif

# Must explicitly check for pthreads on Windows systems (appears to be included with gktmm on LINUX)
ifsys msys
  ifeq ($(PTHREADS_BUILD_FLAG), true)
    PTHREADS32 := true
    ifeq ($(dir_noslash $(findfile pthreadGC2-w32.dll, /mingw/bin /local/bin)), .)
      PTHREADS32 := false
    endif
    PTHREADS64 := true
    ifeq ($(dir_noslash $(findfile pthreadGC2-w64.dll, /mingw/bin /local/bin)), .)
      PTHREADS64 := false
    endif
    ifeq ($(PTHREADS32), false)
      ifeq ($(PTHREADS64), false)
        &echo "Pthreads software not installed on your system."
        &echo "Only those programs not requiring Pthreads software will be built."
        PTHREADS_BUILD_FLAG := false
      endif
    endif
  endif
  ifeq ($(PTHREADS64), true)
    PTHREADS32 := false
  endif
  ifeq ($(PTHREADS_BUILD_FLAG), true)
    ifeq ($(PTHREADS32), true)
      &echo "Found 32 bit Pthreads software."
    else
      &echo "Found 64 bit Pthreads software."
    endif
  endif
endif

CPP := $(find_program mpiicpc icpc mpic++ g++)
ifeq ($(CPP), g++)
  NOPARALLEL = true
endif
ifeq ($(NOPARALLEL), true)
  PARALLEL_BUILD_FLAG = false
  CPP := g++
  LD := g++
  CPPFLAGS := -Wall -O3 -ffloat-store -Wno-unused-result
else
  PARALLEL_BUILD_FLAG = true
  SKEY_BUILD_FLAG = false
  ifeq ($(CPP), mpiicpc)
    LD := mpiicpc -lmpi
  else
    ifeq ($(CPP), icpc)
      LD := mpiicpc -lmpi
    else
      LD := mpic++ -lmpich
    endif
  endif
  CPPFLAGS := -O3
endif
# The "Common" directory MUST be searched before the "RHSeg" directory!!
ifeq ($(COMMON_DIR_FLAG), true)
  CPPFLAGS += -I$(COMMON_DIR)
endif
ifeq ($(RHSEG_DIR_FLAG), true)
  CPPFLAGS += -I$(RHSEG_DIR)
endif
ifeq ($(THREEDIM_FLAG), true)
  DEFINES := -DTHREEDIM
else
  DEFINES := -UTHREEDIM
endif
LIBS :=

ifeq ($(SKEY_BUILD_FLAG), true)
  DEFINES += -DSERIALKEY
else
  DEFINES += -USERIALKEY
endif
ifeq ($(GTKMM_BUILD_FLAG), true)
  DEFINES += -DGTKMM
  ifeq ($(GTKMM3_BUILD_FLAG), true)
    CPPFLAGS += $(shell pkg-config gtkmm-3.0 --cflags)
    LIBS += $(shell pkg-config gtkmm-3.0 --libs)
    DEFINES += -DGTKMM3
  else
    CPPFLAGS += $(shell pkg-config gtkmm-2.4 --cflags)
    LIBS += $(shell pkg-config gtkmm-2.4 --libs)
  endif
else
  DEFINES += -UGTKMM
endif
ifeq ($(GDAL_BUILD_FLAG), true)
  CPPFLAGS += $(shell gdal-config --cflags)
  DEFINES += -DGDAL
  LIBS += $(shell gdal-config --libs)
else
  DEFINES += -UGDAL
endif
ifeq ($(SHAPE_BUILD_FLAG), true)
  CPPFLAGS += $(shell pkg-config shapelib --cflags)
  DEFINES += -DSHAPEFILE
  LIBS += $(shell pkg-config shapelib --libs)
else
  DEFINES += -USHAPEFILE
endif
ifeq ($(PARALLEL_BUILD_FLAG), true)
  DEFINES += -DPARALLEL
else
  DEFINES += -UPARALLEL
endif

ifsys msys
  DEFINES += -DWINDOWS
  ifeq ($(PTHREADS_BUILD_FLAG), true)
    ifeq ($(PTHREADS32), true)
      LIBS += -lpthreadGCE2
    else
      LIBS += -lpthread
    endif
  endif
else
  DEFINES += -UWINDOWS
endif
