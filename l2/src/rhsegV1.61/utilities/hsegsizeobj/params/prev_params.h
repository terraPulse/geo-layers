/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Params class, which is a class
   >>>>                 holding program parameters.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  September 23, 2014; based on ../../params/params.h
   >>>> Modifications:  
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */
#ifndef PARAMS_H
#define PARAMS_H

#include <defines.h>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <climits>
#include <stdexcept>
#include <vector>
#include "gdal_priv.h"
#include "cpl_conv.h" 

using namespace std;

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
      void print();

    // Member variables (public)
    /*-- Input image data file (required) --*/
      string input_image_file;     /*-- USER INPUT IMAGE FILENAME --*/
      bool   input_image_flag;     /*-- EXISTENCE FLAG --*/

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
      GDALDataType data_type;      /*-- PROGRAM PARAMETER --*/
      bool         data_type_flag;       /*-- EXISTENCE FLAG --*/

    /*-- Input data mask file (optional) --*/
      string mask_file;            /*-- USER INPUT FILENAME --*/
      bool   mask_flag;            /*-- EXISTENCE FLAG --*/

    /*-- Input mask value (optional) --*/
      int    mask_value;	   /*-- USER INPUT PARAMETER --*/

    /*-- Relative importance of spectral clustering versus region growing (required) --*/
      float  spclust_wght;         /*-- USER INPUT PARAMETER --*/
      bool   spclust_wght_flag;    /*-- EXISTENCE FLAG --*/

    /*-- Output log file (required) --*/
      string  log_file;            /*-- USER INPUT FILENAME --*/
      bool    log_flag;            /*-- EXISTENCE FLAG --*/
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

    /*-- Neighbor connectivity type --*/
      int    conn_type;                 /*-- USER INPUT PARAMETER --*/

    /*-- Flag that signifies whether or not global dissimilarity values are requested --*/
      bool   gdissim_flag;                /*-- USER INPUT PARAMETER --*/

    /*-- Debug option (optional) --*/
      int    debug;                       /*-- USER INPUT PARAMETER --*/

    /* Program version */
      string version;              /* -- PROGRAM PARAMETER --*/

    /* Default current folder */
      string current_folder;       /* -- PROGRAM PARAMETER --*/

    /* Default filename prefix */
      string prefix;               /* -- PROGRAM PARAMETER --*/

    /* Default image filename suffix */
      string suffix;               /* -- PROGRAM PARAMETER --*/

    /*-- Flag that signifies whether or not region sumsq information is required --*/
      bool   region_sumsq_flag;    /*-- PROGRAM PARAMETER --*/

    /*-- Flag that signifies whether or not region sumxlogx information is required --*/
      bool   region_sumxlogx_flag; /*-- PROGRAM PARAMETER --*/

    /*-- Maximum number of neighbors considered in the region growing analysis --*/
      int    maxnbdir;             /*-- PROGRAM PARAMETER --*/

    /*-- Maximum number of neighbors considered for region objects in connected component labeling --*/
      int    object_maxnbdir;      /*-- PROGRAM PARAMETER --*/

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

} // HSEGTilton

#endif /* PARAMS_H */
