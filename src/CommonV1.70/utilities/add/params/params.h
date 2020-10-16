/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Params class, which is a class 
   >>>>                 holding program parameters.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  July 7, 2010
   >>>> Modifications:  
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
    /*-- First input image (required) --*/
      string input_image1_file;             /*-- USER INPUT IMAGE FILENAME --*/

#ifndef GDAL
    /*-- Number of columns in input image data file (required) --*/
      int    ncols;                /*-- USER INPUT PARAMETER --*/
      bool   ncols_flag;           /*-- EXISTENCE FLAG --*/

    /*-- Number of rows in input image data file (required) --*/
      int    nrows;                /*-- USER INPUT PARAMETER --*/
      bool   nrows_flag;           /*-- EXISTENCE FLAG --*/

    /*-- Number of spectral bands in input image data file (required) --*/
      int    nbands;		   /*-- USER INPUT PARAMETER --*/
      bool   nbands_flag;          /*-- EXISTENCE FLAG --*/

    /*-- Data type of input image data (required) --*/
      RHSEGDType dtype;   	   /*-- USER INPUT PARAMETER --*/
      bool       dtype_flag;       /*-- EXISTENCE FLAG --*/
#endif

    /*-- Second input image (optional) --*/
      string input_image2_file;             /*-- USER INPUT IMAGE FILENAME --*/
      bool input_image2_flag;               /*-- EXISTENCE FLAG --*/

    /*-- Value to add (optional) --*/
      float add_value;                      /*-- USER INPUT PARAMETER --*/
      bool add_value_flag;                  /*-- EXISTENCE FLAG --*/

    /*-- Output image (required) --*/
      string output_image_file;             /*-- USER INPUT IMAGE FILENAME --*/

    /*-- Copy color table flag (optional) --*/
      bool copy_color_table_flag;           /*-- USER INPUT PARAMETER --*/

    /* Program version */
      string version;                       /* -- PROGRAM PARAMETER --*/

    // FRIEND FUNCTIONS and CLASSES
    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
