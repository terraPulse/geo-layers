/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Params class, which is a class 
   >>>>                 holding program parameters.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  January 8, 2010
   >>>> Modifications:  August 17, 2010: Allowed for use without GDAL.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */
#ifndef PARAMS_H
#define PARAMS_H

#include <image/image.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

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
    /*-- Input pixel-based classification (required) --*/
      string pixel_class_file;      /*-- USER INPUT IMAGE FILENAME --*/

#ifndef GDAL
    /*-- Number of columns in classification data file (required) --*/
      int    ncols;                /*-- USER INPUT PARAMETER --*/

    /*-- Number of rows in classification data file (required) --*/
      int    nrows;                /*-- USER INPUT PARAMETER --*/

    /*-- Data type of classification data (required) --*/
      RHSEGDType pixel_dtype;  	   /*-- USER INPUT PARAMETER --*/
#endif

    /*-- Input region segmentation (required) --*/
      string region_seg_file;       /*-- USER INPUT IMAGE FILENAME --*/

#ifdef GDAL
    /*-- Color Table (file) for output classification (optional) --*/
      string color_table_file;        /*-- USER SUPPLIED COLOR TABLE FILENAME --*/
      bool   color_table_flag;        /*-- EXISTENCE FLAG -- */
#else
    /*-- Data type of region segmentation data (required) --*/
      RHSEGDType region_dtype;  	   /*-- USER INPUT PARAMETER --*/
#endif

    /*-- Output region-based classification (required) --*/
      string region_class_file;       /*-- USER INPUT IMAGE FILENAME --*/

    /* Program version */
      string version;               /* -- PROGRAM PARAMETER --*/

    // FRIEND FUNCTIONS and CLASSES
    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
