/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Params class, which is a class
   >>>>                 holding program parameters.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  May 9, 2008
   >>>> Modifications:  May 21, 2008 - Added option for gtkmm based GUI.
   >>>>                 August 8, 2008 - Added hseg_out_thresholds and hseg_out_nregions parameters.
   >>>>                 May 12, 2009: Modified user interface and data I/O for compatibility with ILIADS project needs.
   >>>>                 July 8, 2009: Final clean-up of code for release
   >>>>                 July 24, 2009: Minor bug fix for hseg_out_nregions and hseg_out_thresholds
   >>>>                 October 6, 2009: Added robustness to the reading of parameter values from parameter files
   >>>>                 November 9, 2009: Minor bug fix in generation file name prefix (for filenames containing ".")
   >>>>			January 10, 2010: Changed the definition of min_npixels parameter
   >>>>			January 19, 2010: Changed the definition of spclust_start parameter and renamed it spclust_max
   >>>>                 January 19, 2010: Eliminated max_nregions parameter
   >>>>                 May 26, 2010: Default redefined for split_pixels_factor for program modes HSWO and HSEG.
   >>>>                 June 7, 2010: Modified operation of the RHSEG program mode, eliminating the need for the max_min_npixels parameter.
   >>>>                 September 9, 2010 - Added facility to enter serial key information from GUI.
   >>>>                 September 10, 2010 - Reintroduced optional small region merge acceleration factor.
   >>>>                 December 28, 2010 - Added spclust_min parameter.
   >>>>                 March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
   >>>>                 March 25, 2011 - Added a recursion_mask in order to help equalize the dimension sizes at the deepest level of recursion.
   >>>>                                  Also added the prime_dimension parameter to tag the largest dimension.
   >>>>                 April 26, 2011 - Replaced the recursion mask (recur_mask) with recursion mask flags (recur_mask_flags).
   >>>>                 May 22, 2013 - Added -edge_image, -edge_mask, -edge_threshold and -edge_wght user parameters
   >>>>                 May 24, 2013 - Added random_init_seed parameter
   >>>>                 July 26, 2013 - Added -std_dev_image and -std_dev_mask user parameters and removed std_dev_wght_flag program parameter.
   >>>>                 August 8, 2013 - split_pixels_factor made to apply only to RHSEG program mode.
   >>>>                 September 16, 2013 - Removed the split_pixels_factor, seam_threshold_factor and region_threshold_factor parameters.
   >>>>                 February 6, 2014 - Modified to convert signed short integer data to 32-bit float when negative values present
   >>>>                                    (instead of converting to unsigned short integer and improperly reading negative values).
   >>>>                 February 7, 2014 - Default format for regionMapInImage corrected to be 32-bit unsigned integer.
   >>>>                 February 7, 2014 - Added sort_flag parameter.
   >>>>                 February 28, 2014 - Added initial_merge_flag parameter.
   >>>>                 March 12, 2014 -- Changed default value of random_init_seed from true to false.
   >>>>                 March 19, 2014 -- Added initial_merge_npix parameter.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */
#ifndef PARAMS_H
#define PARAMS_H

#include <defines.h>
#if (defined(GDAL) || defined(RHSEG_EXTRACT))
#include <image/image.h>
#endif
#ifndef IMAGE_H
  enum RHSEGDType { Unknown, UInt8, UInt16, UInt32, Float32 };
#define IMAGE_H
#endif
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <climits>
#include <stdexcept>
#include <vector>

using namespace std;
#if (defined(GDAL) || defined(RHSEG_EXTRACT))
using namespace CommonTilton;
#endif

namespace HSEGTilton
{
  class oParams;

// Params class
  class Params
  {
    public:
    // Constructor and Destructor
      Params(const string& value);
      virtual ~Params();

    // Member functions
      void print_version();
      bool read_init(const char *param_file);
      bool set_defaults();
      bool read(const char *param_file);
      void set_region_sumsq_flag();
      bool set_maxnbdir();
      bool set_object_maxnbdir();
      bool calc_defaults();
#ifdef RHSEG_RUN
      bool calc();
      void write_oparam(const oParams& oparams);
#else
      void write_oparam();
#endif
      void print();

    // Member variables (public)
    /*-- Program Mode (required) --*/
      int program_mode;  /*-- USER INPUT PARAMETER --*/

    /*-- Input image data file (required) --*/
      string input_image_file;     /*-- USER INPUT IMAGE FILENAME --*/
      bool   input_image_flag;     /*-- EXISTENCE FLAG --*/

#ifndef GDAL
    /*-- Number of columns in input image data file (required) --*/
      int    ncols;                /*-- USER INPUT PARAMETER --*/
      bool   ncols_flag;           /*-- EXISTENCE FLAG --*/

    /*-- Number of rows in input image data file (required) --*/
      int    nrows;                /*-- USER INPUT PARAMETER --*/
      bool   nrows_flag;           /*-- EXISTENCE FLAG --*/

#ifdef THREEDIM
      /*-- Number of slices in input image data file (required) --*/
      int    nslices;              /*-- USER INPUT PARAMETER --*/
      bool   nslices_flag;         /*-- EXISTENCE FLAG --*/
#endif

    /*-- Number of spectral bands in input image data file (required) --*/
      int    nbands;		   /*-- USER INPUT PARAMETER --*/
      bool   nbands_flag;          /*-- EXISTENCE FLAG --*/

    /*-- Data type of input image data (required) --*/
      RHSEGDType dtype;   	   /*-- USER INPUT PARAMETER --*/
      bool       dtype_flag;       /*-- EXISTENCE FLAG --*/
#endif

    /*-- Input data mask file (optional) --*/
      string mask_file;            /*-- USER INPUT FILENAME --*/
      bool   mask_flag;            /*-- EXISTENCE FLAG --*/

#ifndef GDAL
    /*-- Input mask value (optional) --*/
      int    mask_value;	   /*-- USER INPUT PARAMETER --*/
#endif

    /*-- Input standard deviation image file (optional) --*/
      string std_dev_image_file;      /*-- USER INPUT IMAGE FILENAME --*/
      bool   std_dev_image_flag;      /*-- EXISTENCE FLAG --*/

    /*-- Input standard deviation mask file (optional) --*/
      string std_dev_mask_file;       /*-- USER INPUT IMAGE FILENAME --*/
      bool   std_dev_mask_flag;       /*-- EXISTENCE FLAG --*/

    /*-- Input edge image file (required in RHSEG mode, optional otherwise) --*/
      string edge_image_file;      /*-- USER INPUT IMAGE FILENAME --*/
      bool   edge_image_flag;      /*-- EXISTENCE FLAG --*/

    /*-- Input edge mask file (optional) --*/
      string edge_mask_file;       /*-- USER INPUT IMAGE FILENAME --*/
      bool   edge_mask_flag;       /*-- EXISTENCE FLAG --*/

    /*-- Input region map file (optional) --*/
      string region_map_in_file;   /*-- USER INPUT FILENAME --*/
      bool   region_map_in_flag;   /*-- EXISTENCE FLAG --*/
      bool   complete_labeling_flag;  /*-- FLAG INDICATING IF REGION_MAP_IN IS A COMPLETE LABELING OF THE IMAGE --*/

    /*-- Relative importance of spectral clustering versus region growing (required) --*/
      float  spclust_wght;         /*-- USER INPUT PARAMETER --*/
      bool   spclust_wght_flag;    /*-- EXISTENCE FLAG --*/

    /*-- Specify dissimilarity criterion for region merging (required)--*/
      int    dissim_crit;               /*-- USER INPUT PARAMETER --*/

    /*-- Output log file (required) --*/
      string  log_file;            /*-- USER INPUT FILENAME --*/
      fstream log_fs;              /*-- ASSOCIATED FILE STREAM --*/

    /*-- Input image data scale and offset values (optional) --*/
      vector<double> scale, offset;     /*-- USER INPUT PARAMETERS --*/

    /*-- Output region classes map file (required) --*/
      string  class_labels_map_file;    /*-- USER INPUT FILENAME --*/

    /*-- Output boundary map file (optional) --*/
      string  boundary_map_file;        /*-- USER INPUT FILENAME --*/
      bool    boundary_map_flag;        /*-- EXISTENCE FLAG --*/

    /*-- Output region object list file (required) --*/
      string region_classes_file;       /*-- USER INPUT FILENAME --*/

    /*-- Output region objects map file (optional) --*/
      string object_labels_map_file;    /*-- USER INPUT FILENAME --*/
      bool   object_labels_map_flag;    /*-- EXISTENCE FLAG --*/

    /*-- Output region objects file (optional) --*/
      string region_objects_file;       /*-- USER INPUT FILENAME --*/
      bool   region_objects_flag;       /*-- EXISTENCE FLAG --*/

    /*-- Region object connectivity flag --*/
      bool   object_conn_type1_flag;    /*-- USER INPUT PARAMETER --*/

    /*-- Output parameter file (required) --*/
      string oparam_file;               /*-- USER INPUT FILENAME --*/

    /*-- Region sum output flag --*/
      bool   region_sum_flag;           /*-- USER INPUT PARAMETER --*/

    /*-- Region standard deviation output flag --*/
      bool   region_std_dev_flag;       /*-- USER INPUT PARAMETER --*/

    /*-- Region boundary number of pixels output flag --*/
      bool   region_boundary_npix_flag; /*-- USER INPUT PARAMETER --*/

    /*-- Region merge threshold output flag --*/
      bool   region_threshold_flag;     /*-- USER INPUT PARAMETER --*/

    /*-- Output flag of the list of region classes neighboring each region class --*/
      bool   region_nghbrs_list_flag;   /*-- USER INPUT PARAMETER --*/

    /*-- Region number of regions objects flag --*/
      bool   region_nb_objects_flag;    /*-- USER INPUT PARAMETER --*/

    /*-- List of region objects output flag --*/
      bool   region_objects_list_flag;  /*-- USER INPUT PARAMETER --*/

    /*-- Weight for standard deviation feature (optional) --*/
      float  std_dev_wght;              /*-- USER INPUT PARAMETER --*/

    /*-- Edge threshold value (optional) --*/
      float  edge_threshold;            /*-- USER INPUT PARAMETER --*/

    /*-- Edge power value (optional) --*/
      float  edge_power;                /*-- USER INPUT PARAMETER --*/

    /*-- Edge weight value (optional) --*/
      float  edge_wght;                 /*-- USER INPUT PARAMETER --*/
      float  max_edge_value, min_edge_value;  /*-- PROGRAM PARAMETERS --*/

    /*-- Seam edge threshold value (optional) --*/
      float  seam_edge_threshold;       /*-- USER INPUT PARAMETER --*/

    /*-- Neighbor connectivity type --*/
      int    conn_type;                 /*-- USER INPUT PARAMETER --*/

    /*-- Number of regions at which hierarchical segmentation output is started (optional) --*/
      unsigned int chk_nregions;              /*-- USER INPUT PARAMETER --*/
      bool         chk_nregions_flag;         /*-- EXISTENCE FLAG --*/

    /*-- The set of number of region at which hierarchical segmentation outputs are made (optional) --*/
      vector<unsigned int> hseg_out_nregions;          /*-- USER INPUT PARAMETER --*/
      bool                 hseg_out_nregions_flag;     /*-- EXISTENCE FLAG --*/

    /*-- The set of merge thresholds at which hierarchical segmentation outputs are made (optional) --*/
      vector<float> hseg_out_thresholds;      /*-- USER INPUT PARAMETER --*/
      bool          hseg_out_thresholds_flag; /*-- EXISTENCE FLAG --*/

    /*-- Number of regions for final convergence --*/
      unsigned int conv_nregions;         /*-- USER INPUT PARAMETER --*/

    /*-- Flag that signifies whether or not global dissimilarity values are requested --*/
      bool   gdissim_flag;                /*-- USER INPUT PARAMETER --*/

    /*-- Debug option (optional) --*/
      int    debug;                       /*-- USER INPUT PARAMETER --*/

    /*-- Image normalization type (optional) --*/
      int    normind;                     /*-- USER INPUT PARAMETER --*/

    /*-- Threshold for initial fast region merging by a pixel oriented first merge process (optional) --*/
      float  init_threshold;              /*-- USER INPUT PARAMETER --*/

    /*-- Initial merge npix parameter and flag (optional, default supplied) --*/
      unsigned int    initial_merge_npix; /*-- USER INPUT PARAMETER --*/
      bool            initial_merge_flag; /*-- EXISTENCE FLAG --*/

    /*-- Flag that signifies whether or not a randomized seed (based on current clock time) is used for 
         the sampling procedure utilized in the intial fast region merging porcess (optional) --*/
      bool   random_init_seed_flag;       /*-- USER INPUT PARAMETER --*/

    /*-- Flag that signifies whether of not the region classes and objects are sorted by npix or distance to minimum vector (optional) --*/
      int    sort_flag;                   /*-- USER INPUT PARAMETER --*/

    /*-- Edge dissimilarity option (optional) --*/
      int    edge_dissim_option;          /*-- USER INPUT PARAMETER --*/

    /*-- Number of levels in recursive stages (optional) --*/
      int   rnb_levels;                   /*-- USER INPUT PARAMETER --*/
      bool  rnb_levels_flag;              /*-- USER OVERRIDE FLAG --*/

    /*-- Recursive level into which the data is initially input (PARALLEL) --*/
      int   inb_levels;                   /*-- USER INPUT PARAMETER --*/

    /*-- Lowest recursive level at which data is stored and from
         which segmentation results are output (PARALLEL)--*/
      int   onb_levels;                   /*-- USER INPUT PARAMETER --*/

#ifndef PARALLEL
    /*-- Recursive level at which the data I/O is performed --*/
      int   ionb_levels;                  /*-- USER INPUT PARAMETER --*/
      bool  ionb_levels_flag;             /*-- USER OVERRIDE FLAG --*/
#endif
// NOTE:  For programming convenience, inb_levels = onb_levels = ionb_levels in non-Parallel case.

    /*-- Number of regions for convergence at intermediate stages (optional) --*/
      unsigned int min_nregions;               /*-- USER INPUT PARAMETER --*/
      bool         min_nregions_flag;          /*-- USER OVERRIDE FLAG --*/

    /*-- Minimum number of regions for which spectral clustering is performed (optional) --*/
      unsigned int spclust_min;                /*-- USER INPUT PARAMETER --*/

    /*-- Maximum number of regions for which spectral clustering is performed (optional) --*/
      unsigned int spclust_max;                /*-- USER INPUT PARAMETER --*/

    /*-- Flag that signifies whether or not small region merge acceleration is requested --*/
      bool   merge_accel_flag;                 /*-- USER INPUT PARAMETER --*/

    /* Program version */
      string version;              /* -- PROGRAM PARAMETER --*/

    /* Default current folder */
      string current_folder;       /* -- PROGRAM PARAMETER --*/

    /* Default filename prefix */
      string prefix;               /* -- PROGRAM PARAMETER --*/

    /* Default image filename suffix */
      string suffix;               /* -- PROGRAM PARAMETER --*/

#ifdef GDAL
    /*-- Number of columns in input image data file --*/
      int    ncols;                /*-- PROGRAM PARAMETER --*/
      bool   ncols_flag;           /*-- EXISTENCE FLAG --*/

    /*-- Number of rows in input image data file --*/
      int    nrows;                /*-- PROGRAM PARAMETER --*/
      bool   nrows_flag;           /*-- EXISTENCE FLAG --*/
#ifdef THREEDIM
      /*-- Number of slices in input image data file --*/
      int    nslices;              /*-- PROGRAM PARAMETER --*/
      bool   nslices_flag;         /*-- EXISTENCE FLAG --*/
#endif
    /*-- Number of spectral bands in input image data file --*/
      int    nbands;               /*-- PROGRAM PARAMETER --*/
      bool   nbands_flag;          /*-- EXISTENCE FLAG --*/

    /*-- Data type of input image data --*/
      RHSEGDType dtype;            /*-- PROGRAM PARAMETER --*/
      bool       dtype_flag;       /*-- EXISTENCE FLAG --*/
      GDALDataType data_type;      /*-- PROGRAM PARAMETER --*/

    /*-- Input mask value --*/
      int    mask_value;           /*-- PROGRAM PARAMETER --*/

    /*-- Driver description for output image files --*/
      string output_driver_description;           /*-- PROGRAM PARAMETER --*/
#endif

    /*-- Dimensionality (1-D, 2-D or 3-D allowed) --*/
      int    nb_dimensions;        /*-- PROGRAM PARAMETER --*/

    /*-- Tag for the largest dimension (1->column, 2->row, 3->slice, 0 or other->undefined) --*/
      int    prime_dimension;        /*-- PROGRAM PARAMETER --*/

    /*-- Flag that signifies whether or not region sumsq information is required --*/
      bool   region_sumsq_flag;    /*-- PROGRAM PARAMETER --*/

    /*-- Flag that signifies whether or not region sumxlogx information is required --*/
      bool   region_sumxlogx_flag; /*-- PROGRAM PARAMETER --*/

    /*-- Minimum number of pixels a region must contain to be involved in spectral clustering --*/
      unsigned int    min_npixels; /*-- PROGRAM PARAMETER --*/

    /*-- Maximum number of neighbors considered in the region growing analysis --*/
      int    maxnbdir;             /*-- PROGRAM PARAMETER --*/

    /*-- Maximum number of neighbors considered for region objects in connected component labeling --*/
      int    object_maxnbdir;      /*-- PROGRAM PARAMETER --*/

    /*-- Number of number of region levels at which hierarchical segmentation outputs are made --*/
      int    nb_hseg_out_nregions; /*-- PROGRAM PARAMETER --*/

    /*-- Number of merge thresholds at which hierarchical segmentation outputs are made --*/
      int    nb_hseg_out_thresholds; /*-- PROGRAM PARAMETER --*/

    /*-- Size of seam between processing windows --*/
      int    seam_size;             /*-- PROGRAM PARAMETER --*/

    /*-- Lowest level of recursion at which this task is active --*/
      int    recur_level;           /*-- PROGRAM PARAMETER --*/

    /*-- The recursive mask flags enable suppressing recursive subdivision in selected dimensions --*/
      vector<unsigned char> recur_mask_flags;     /*-- PROGRAM PARAMETERS --*/

#ifdef PARALLEL
    /*-- The rank of the current parallel process. --*/
      int    myid;                  /*-- PROGRAM PARAMETER --*/

    /*-- Size of the "comm" group, i.e. number of parallel processes --*/
      int    numprocs;              /*-- PROGRAM PARAMETER --*/

    /*-- Number of sub tasks called by this task--*/
      int    nb_tasks;              /*-- PROGRAM PARAMETER --*/
#else
    /*-- Temporary file name --*/
      string temp_file_name;       /*-- PROGRAM PARAMETER --*/

    /*-- Data dimensions at ionb_levels of recursion --*/
#ifdef THREEDIM
      int    ionb_ncols, ionb_nrows, ionb_nslices; /*-- PROGRAM PARAMETERS --*/
#else
      int    ionb_ncols, ionb_nrows;               /*-- PROGRAM PARAMETERS --*/
#endif
#endif

    /*-- Number of I/O processing sections  --*/
      int    nb_sections;                             /*-- PROGRAM PARAMETER --*/
// NOTE:  For programming convenience, nb_sections = nb_tasks in Parallel case.
#ifdef THREEDIM
    /*-- Data dimensions at the deepest level of recursion (rnb_levels) --*/
      int    rnb_ncols, rnb_nrows, rnb_nslices;       /*-- PROGRAM PARAMETERS --*/

    /*-- Data dimensions for the pixel_data array --*/
      int    pixel_ncols, pixel_nrows, pixel_nslices; /*-- PROGRAM PARAMETERS --*/

    /*-- Data dimensions at inb_levels of recursion (or at sub_tasks for coarse-to_fine) --*/
      int    inb_ncols, inb_nrows, inb_nslices;       /*-- PROGRAM PARAMETERS --*/

    /*-- Data dimensions at onb_levels of recursion (=inb_levels for coarse-to-fine) --*/
      int    onb_ncols, onb_nrows, onb_nslices;       /*-- PROGRAM PARAMETERS --*/
// NOTE:  For programming convenience, pixel_ncols = inb_ncols = onb_ncols = ionb_ncols in non-Parallel case.
// NOTE:  For programming convenience, pixel_nrows = inb_nrows = onb_nrows = ionb_nrows in non-Parallel case.
// NOTE:  For programming convenience, pixel_nslices = inb_nslices = onb_nslices = ionb_nslices in non-Parallel case.

    /*-- Data dimensions for the padded input data (= ncols, nrows and nslices if no padding). --*/
      int    padded_ncols, padded_nrows, padded_nslices;/*-- PROGRAM PARAMETERS --*/
#else
      int    rnb_ncols, rnb_nrows;                      /*-- PROGRAM PARAMETERS --*/
    /*-- Data dimensions for the pixel_data array --*/
      int    pixel_ncols, pixel_nrows;                  /*-- PROGRAM PARAMETERS --*/

    /*-- Data dimensions at inb_levels of recursion (or at sub_tasks for coarse-to_fine) --*/
      int    inb_ncols, inb_nrows;                      /*-- PROGRAM PARAMETERS --*/

    /*-- Data dimensions at onb_levels of recursion (=inb_levels for coarse-to-fine) --*/
      int    onb_ncols, onb_nrows;                      /*-- PROGRAM PARAMETERS --*/
// NOTE:  For programming convenience, pixel_ncols = inb_ncols = onb_ncols = ionb_ncols in non-Parallel case.
// NOTE:  For programming convenience, pixel_nrows = inb_nrows = onb_nrows = ionb_nrows in non-Parallel case.

    /*-- Data dimensions for the padded input data (= ncols and nrows if no padding). --*/
      int    padded_ncols, padded_nrows;        /*-- PROGRAM PARAMETERS --*/
#endif
    /*-- True if and only if padding is required. --*/
      bool   padded_flag;                       /*-- PROGRAM PARAMETER --*/

    /*-- Column offsets for each processing section at the intermediate or final processing level --*/
      vector<int> col_offset;                   /*-- PROGRAM PARAMETERS --*/
      int         col_offset_flag;              /*-- FLAG (0=>not set; 1=>set for interm.; 2=>set for final --*/

    /*-- Row offsets for each processing section at the intermediate or final processing level --*/
      vector<int> row_offset;                   /*-- PROGRAM PARAMETERS --*/
      int         row_offset_flag;              /*-- FLAG (0=>not set; 1=>set for interm.; 2=>set for final --*/
#ifdef THREEDIM
    /*-- slice offsets for each processing section at the intermediate or final processing level --*/
      vector<int> slice_offset;                 /*-- PROGRAM PARAMETERS --*/
      int         slice_offset_flag;            /*-- FLAG (0=>not set; 1=>set for interm.; 2=>set for final --*/
#endif

    /*-- Flag indicating if GTKMM GUI actually utilized, irrespective of define of GTKMM --*/
      bool    gtkmm_flag;                       /*-- PROGRAM PARAMETER --*/

    /*-- Flag indicating if GDAL is actually utilized for Image Input, irrespective of define of GDAL --*/
      bool    gdal_input_flag;                  /*-- PROGRAM PARAMETER --*/

#ifdef GTKMM
    /*-- Exit status from GUI --*/
      int     status;   	                /*-- PROGRAM PARAMETER --*/
#endif

    // FRIEND FUNCTIONS and CLASSES
    protected:

    private:
  };

  // For "stringify'ing integer and float values
  class BadConversion : public std::runtime_error
  {
    public:
      BadConversion(const std::string& s)
        : std::runtime_error(s)
        { }
  };

  inline std::string stringify_int(int x)
  {
    std::ostringstream o;
    if (!(o << x))
      throw BadConversion("stringify_int(int)");
    return o.str();
  }

  inline std::string stringify_float(float x)
  {
    std::ostringstream o;
    if (!(o << x))
      throw BadConversion("stringify_float(float)");
    return o.str();
  }

// Other associated classes and functions
  class oParams
  {
    public:
    // Constructor and Destructor
      oParams();
      virtual ~oParams();

    // Member functions

    // Member variables (public)
      vector<double> scale, offset, minval, meanval;
      unsigned int  tot_npixels, percent_complete;
      int nb_levels, level0_nb_classes, level0_nb_objects;
      vector<unsigned int> int_buffer_size;
      vector<double> max_threshold;
      vector<double> gdissim;
  };

  string process_line(const string& line, const bool& list_flag);

#ifdef RHSEG_RUN
#ifdef THREEDIM
  void set_offsets(const int& recur_level, const int& nb_sections, const int& section,
                   const int& init_col_offset, const int& init_row_offset, const int& init_slice_offset,
                   const int& ncols, const int& nrows, const int& nslices);
#else
  void set_offsets(const int& recur_level, const int& nb_sections, const int& section,
                   const int& init_col_offset, const int& init_row_offset,
                   const int& ncols, const int& nrows);
#endif
#endif
  void set_stride_sections(const int& recur_level, int& stride, int& nb_sections);

#ifdef THREEDIM
  int set_recur_flags(const unsigned char& recur_mask_flag, bool& col_flag, bool& row_flag, bool& slice_flag);
#else
  int set_recur_flags(const unsigned char& recur_mask_flag, bool& col_flag, bool& row_flag);
#endif

} // HSEGTilton

#endif /* PARAMS_H */
