/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Params class, which is a class 
   >>>>                 holding program parameters.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  April 15, 2013
   >>>> Modifications:  April 22, 2013 - Added edge_threshold parameter
   >>>>                 May   22, 2013 - Added optional scale_output and required output_mask_image parameters
   >>>>                 May   28, 2013 - Replaced band_average flag with output_type parameter
   >>>>                 July  26, 2013 - Added mask_file and mask_value input parameters
   >>>>                 February 10, 2014 - Added bias_value input parameter
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */
#ifndef PARAMS_H
#define PARAMS_H

#include <image/image.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace CommonTilton
{
// Params class
  class Params 
  {
    public:
    // Constructor and Destructor
      Params(const string& value);
      virtual ~Params();

    // Member functions
      void print_version();
      bool read(const char *param_file);
      void print();

    // Member variables (public)
    /*-- Input image (required) --*/
      string input_image_file;      /*-- USER INPUT IMAGE FILENAME --*/

    /*-- Number of columns in input image data file (required) --*/
      int    ncols;                 /*-- USER INPUT PARAMETER --*/

    /*-- Number of rows in input image data file (required) --*/
      int    nrows;                 /*-- USER INPUT PARAMETER --*/

    /*-- Number of spectral bands in input image data file (required) --*/
      int    nbands;		    /*-- USER INPUT PARAMETER --*/

    /*-- Data type of input image data (required) --*/
      RHSEGDType dtype;   	    /*-- USER INPUT PARAMETER --*/
#ifdef GDAL
      GDALDataType data_type;       /*-- PROGRAM PARAMETER --*/

    /*-- Driver description for output image files --*/
      string output_driver_description;           /*-- PROGRAM PARAMETER --*/
#endif

    /*-- Input data mask file (optional) --*/
      string mask_file;              /*-- USER INPUT FILENAME --*/
      bool   mask_flag;              /*-- EXISTENCE FLAG --*/

    /*-- Input mask value (optional) --*/
      int    mask_value;	     /*-- USER INPUT PARAMETER --*/

    /*-- Input bias value (optional) --*/
      int    bias_value;	     /*-- USER INPUT PARAMETER --*/

    /*-- Edge operation type (required) --*/
      EdgeOperation edge_operation;  /*-- USER INPUT PARAMETER --*/

    /*-- Frei-Chen operation option (optional) --*/
      short int fc_option;   	     /*-- USER INPUT PARAMETER --*/

    /*-- Edge threshold (optional) --*/
      float edge_threshold;          /*-- USER INPUT PARAMETER --*/

    /*-- Output type (optional) --*/
      int output_type;               /*-- USER INPUT PARAMETER --*/

    /*-- Scale output flag (optional) --*/
      bool scale_output_flag;        /*-- USER INPUT PARAMETER --*/

    /*-- Output image (required) --*/
      string output_image_file;      /*-- USER INPUT IMAGE FILENAME --*/

    /*-- Output mask image (required) --*/
      string output_mask_image_file; /*-- USER INPUT IMAGE FILENAME --*/

    /* Program version */
      string version;                       /* -- PROGRAM PARAMETER --*/

    /*-- Temporary file name --*/
      string temp_file_name;       /*-- PROGRAM PARAMETER --*/

    // FRIEND FUNCTIONS and CLASSES
    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
