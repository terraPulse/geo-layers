/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Params class, which is a class 
   >>>>                 holding program parameters.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  June 13, 2011
   >>>> Modifications:  Aug. 21, 2012 - Added optional copy_color_table flag.
   >>>>                 Oct. 22, 2012 - Added output_value parameter.
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

    /*-- Comparison Type (required) --*/
      MathOperation compare_type;           /*-- USER INPUT PARAMETER --*/

    /*-- Second input image (optional) --*/
      string input_image2_file;             /*-- USER INPUT IMAGE FILENAME --*/
      bool input_image2_flag;               /*-- EXISTENCE FLAG --*/

    /*-- Value to compare (optional) --*/
      float compare_value;                  /*-- USER INPUT PARAMETER --*/
      bool compare_value_flag;              /*-- EXISTENCE FLAG --*/

    /*-- Output image (required) --*/
      string output_image_file;       /*-- USER INPUT IMAGE FILENAME --*/

    /*-- Output value (optional) --*/
      int output_value;                     /*-- USER INPUT PARAMETER --*/

    /*-- Copy color table flag (optional) --*/
      bool copy_color_table_flag;           /*-- USER INPUT PARAMETER --*/

    /* Program version */
      string version;               /* -- PROGRAM PARAMETER --*/

    // FRIEND FUNCTIONS and CLASSES
    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
