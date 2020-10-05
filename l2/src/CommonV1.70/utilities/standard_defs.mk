COMMON_DIR := ../../CommonV1.70
INSTALL_DIR := ../../../bin

ifeq ($(NOGDAL), true)
  GDAL_BUILD_FLAG := false
else
  GDAL_BUILD_FLAG := true
endif

ifeq ($(NOOPENCV), true)
  OPENCV_BUILD_FLAG := false
else
  OPENCV_BUILD_FLAG := $(GDAL_BUILD_FLAG)
endif

ifeq ($(NOSHAPE), true)
  SHAPE_BUILD_FLAG := false
else
  SHAPE_BUILD_FLAG := $(GDAL_BUILD_FLAG)
endif

ifeq ($(GDAL_BUILD_FLAG), true)
  ifeq ($(find_program gdal-config), gdal-config)
    GDAL_VERSION != gdal-config --version
  else
    GDAL_VERSION := 0.0
    &echo "GDAL not installed on your system."
    &echo "Only those programs not requiring GDAL will be built."
    GDAL_BUILD_FLAG := false
    OPENCV_BUILD_FLAG := false
    SHAPE_BUILD_FLAG := false
  endif
endif

ifeq ($(OPENCV_BUILD_FLAG), true)
  ifeq ($(dir_noslash $(findfile opencv.pc, $(PKG_CONFIG_PATH) \
                        /usr/local/lib/pkgconfig /usr/lib/pkgconfig \
                        /usr/lib64/pkgconfig /usr/share/pkgconfig)), .)
    &echo "OpenCV software not installed on your system."
    &echo "Only those programs not requiring OpenCV software will be built."
    OPENCV_BUILD_FLAG = false
  else
    OPENCV_BUILD_FLAG = true
    OPENCV_VERSION != pkg-config --modversion opencv
    &echo "Found OpenCV version $(OPENCV_VERSION)"
  endif
endif

ifeq ($(SHAPE_BUILD_FLAG), true)
  ifeq ($(dir_noslash $(findfile shapefil.h, /usr/local/include /usr/local/include/libshp /usr/include)), .)
    &echo "Shapefile software not installed on your system."
    &echo "Only those programs not requiring Shapefile software will be built."
    SHAPE_BUILD_FLAG := false
  endif
endif

CPP := g++
LD := g++
CPPFLAGS := -Wall -O3 -ffloat-store -I$(COMMON_DIR) -Wno-unused-result
DEFINES :=

ifeq ($(GDAL_BUILD_FLAG), true)
  CPPFLAGS += $(shell gdal-config --cflags)
  LIBS += $(shell gdal-config --libs)
  DEFINES += -DGDAL
endif

ifeq ($(OPENCV_BUILD_FLAG), true)
  CPPFLAGS += $(shell pkg-config opencv --cflags)
  LIBS += $(shell pkg-config opencv --libs)
  DEFINES += -DOPENCV
endif

ifeq ($(SHAPE_BUILD_FLAG), true)
  CPPFLAGS += $(shell pkg-config shapelib --cflags)
  LIBS += $(shell pkg-config shapelib --libs)
  DEFINES += -DSHAPEFILE
endif

ifsys msys
DEFINES += -DWINDOWS
endif
