/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose: Defines for the HSEG software suite.
   >>>>                For compiler directives and constant definitions
   >>>>
   >>>>    Written By: James C. Tilton, NASA GSFC, Mail Code 606.3, Greenbelt, MD 20771
   >>>>                James.C.Tilton@nasa.gov
   >>>>
   >>>>       Written: January 28, 2009
   >>>> Modifications: May 12, 2009: Modified user interface and data I/O for compatibility with ILIADS project needs.
   >>>>                July 8, 2009: Final clean-up of code for release
   >>>>		       January 10, 2010: Changed the definition of min_npixels parameter
   >>>>                January 19, 2010: Changed the definition of the spclust_start parameter and renamed it spclust_max
   >>>>                July 6, 2010: Added SMALL_EPSILON constant
   >>>>                September 10, 2010: Added MERGE_ACCEL_FLAG constant
   >>>>                December 28, 2010: Added SPCLUST_MIN constant
   >>>>                January 14, 2011: Changed value for MAX_NREGIONS constant
   >>>>                April 11, 2011: Moved include of mpi.h to this file.
   >>>>                June 8, 2011: Changed default value for MERGE_ACCEL_FLAG to true
   >>>>                April 26, 2013: Deleted CONTAGIOUS flag (will always use contagious option - also with V1.70 became unnecessary)
   >>>>                May 22, 2013: Added EDGE_THRESHOLD and EDGE_WGHT constants and removed the NGHBRS_LABEL_SET_SIZE_LIMIT constant
   >>>>                July 23, 2013: Removed the CLRG (centroid linked region growing) option for first merge region growing
   >>>>                               This leaves Muerlle-Allen first merge region growing as the only option.
   >>>>                July 26, 2013: STD_DEV_WEIGHT constant reset from 0.0 to 1.0.
   >>>>                September 16, 2013: Added SEAM_EDGE_THRESHOLD constant and reset value of EDGE_THRESHOLD from 0.005 to 0.05.
   >>>>                September 16, 2013: Removed SPLIT_PIXELS_FACTOR, SEAM_THRESHOLD_FACTOR and REGION_THRESHOLD_FACTOR constants.
   >>>>                September 27, 2013: Added MIN_SEAM_EDGE_REGION_SIZE and MIN_SEAM_EDGE_NPIX constants.
   >>>>                September 27, 2013: Added EDGE_COMPUTE_OPTION and EDGE_DISSIM_OPTION constants.
   >>>>                October 31, 2013: Removed EDGE_COMPUTE_OPTION constant.
   >>>>                November 12, 2013: Changed the value of MIN_SEAM_EDGE_NPIX from 5 to 3.
   >>>>                February 7, 2014: Changed the NO_SORT to the SORT_FLAG boolean constant with default value true.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

//* Compiler Directives: Leaving defined or undefining selects particular compiler options
//* Notes:
//* ->Normally MSE_SQRT should be left defined.
//* ->Normally DEBUG and TIME_IT should be undefined.
//* ->Other flags should be left defined or made undefined depending on the situation.
#ifndef DEFINES_H
#define DEFINES_H

#define GLSIMP
#undef GLSIMP        // Leave defined if building special version for the GLS-IMP group

#define CYGWIN
#undef CYGWIN          // Leave defined if compiling under cygwin on a Windows machine (use Linux makefiles)
#define ARTEMIS
#undef ARTEMIS       // Leave defined if compiling for the Bartron "artemis" cluster

#define DEBUG
#undef DEBUG         // Leave defined if debug outputs are desired in log file.
#define TIME_IT
#undef TIME_IT       // Leave defined if detailed timings are desired in log file.

#define MSE_SQRT
//#undef MSE_SQRT      // Leave defined if using the square root of the MSE dissimilarity function.
#define MEAN_NORM_STD_DEV    // Leave defined if using mean normalized standard deviation
#undef MEAN_NORM_STD_DEV

//* Constant Definitions
#ifdef PARALLEL
#define MAX_NREGIONS        32768 // = 128*128*2. Used to determine default number of recursive levels (for PARALLEL version)
#define MAX_NUMPROCS          256
#else
#define MAX_NREGIONS      1048576 // = 1024*1024. Used to determine default number of recursive levels (for SERIAL version)
#endif
#define MAX_NPIXELS        262144 // Maximum number of pixels in processing window at the lowest recursive level
                                  // at which data I/O is performed
#define MAXNBDIR               26 // Maximum number of neighbors allowed
#define TIME_SIZE              26 // Dimension size for vector used in timing buffer string
#define CONV_FACTOR             0 // Valid values are 0, 1, 2, 3 or 4
#define PRINT_INTERVAL      10000
#define MAX_TEMP_SIZE          44
#define SMALL_EPSILON     1.0e-10

// Used in rhseg_read (HSEGReader)
#define MIN_STD_DEV_NPIX        10

// Used in rhseg_view (HSEGViewer)
#define NUMBER_OF_LABELS        69
#define HIGHLIGHT_COLOR         67   // NUMBER_OF_LABELS - 2
#define BAD_DATA_QUALITY_LABEL  68   // NUMBER_OF_LABELS - 1
#define INIT_SEG_LEVEL           0

// Used in rhseg_pvote (HSEGPvote)
#define NB_LIMIT		 4

// Program defaults
#define INPUT_IMAGE_FILE "_input"
#define MASK_FILE "_mask"
#define MASK_VALUE 0
#define STD_DEV_IN_FILE "_std_dev_in"
#define STD_DEV_MASK_FILE "_std_dev_mask"
#define EDGE_IN_FILE "_edge_in"
#define EDGE_MASK_FILE "_edge_mask"
#define REGION_MAP_IN_FILE "_region_map_in"
#define CLASS_LABELS_MAP_FILE "_class_labels_map"
#define BOUNDARY_MAP "_boundary_map"
#define BOUNDARY_MAP_FLAG false
#define REGION_CLASSES "_region_classes"
#define OBJECT_LABELS_MAP "_object_labels_map"
#define REGION_OBJECTS "_region_objects"
#define REGION_OBJECTS_FLAG false
#define REGION_SUM_FLAG_MAX 20
#define REGION_STD_DEV_FLAG false
#define REGION_BOUNDARY_NPIX_FLAG false
#define SORT_FLAG true
#define REGION_THRESHOLD_FLAG false
#define REGION_NGHBRS_LIST_FLAG true // Always true
#define RHSEG_SCALE 1.0
#define RHSEG_OFFSET 0.0
#define DISSIM_CRIT 6
#define SPCLUST_MAX 1024
#define SPCLUST_MIN  512
#define HSEG_OUT_NREGIONS_FLAG false
#define HSEG_OUT_THRESHOLDS_FLAG false
#define CONV_NREGIONS 2
#define GDISSIM_FLAG false
#define MERGE_ACCEL_FLAG false;
#ifdef GDAL
#define OUTPUT_DRIVER_DESCRIPTION "ENVI"
#endif

#define DEBUG_DEFAULT_VALUE 1
#define NORMIND 2
#define INIT_THRESHOLD 0.0
#define STD_DEV_WGHT 1.0
#define EDGE_DISSIM_OPTION 2
#define EDGE_THRESHOLD 0.05
#define EDGE_WGHT 1.0
#define SEAM_EDGE_THRESHOLD 0.05
#define MIN_SEAM_EDGE_REGION_SIZE 10
#define MIN_SEAM_EDGE_NPIX 3

// For the parallel processing version, the mpi.h file must be included here to avoid a conflict with SEEK_SET in stdio.h
#ifdef PARALLEL
#include <mpi.h>
#endif

#endif // DEFINES_H
