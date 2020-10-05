/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Params class, which is a class 
   >>>>                 holding program parameters.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  January 6, 2010
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
    /*-- Input test set image data (required) --*/
      string test_image_file;             /*-- USER INPUT IMAGE FILENAME --*/

    /*-- Input classified image data (required) --*/
      string classified_image_file;       /*-- USER INPUT IMAGE FILENAME --*/

#ifndef GDAL
    /*-- Number of columns in input images (required) --*/
      int    ncols;                /*-- USER INPUT PARAMETER --*/

    /*-- Number of rows in input images (required) --*/
      int    nrows;                /*-- USER INPUT PARAMETER --*/

    /*-- Data type of input images (required) --*/
      RHSEGDType dtype;  	   /*-- USER INPUT PARAMETER --*/
#endif

    /* Program version */
      string version;               /* -- PROGRAM PARAMETER --*/

    // FRIEND FUNCTIONS and CLASSES
    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
