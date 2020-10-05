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
    /*-- Input image (required) --*/
      string input_image_file;        /*-- USER INPUT IMAGE FILENAME --*/

    /*-- Selection rate (required) --*/
      float rate;                     /*-- USER INPUT UNSIGNED FLOAT --*/

    /*-- Color Table (file) for output classified image (optional) --*/
      string color_table_file;        /*-- USER SUPPLIED COLOR TABLE FILENAME --*/
      bool   color_table_flag;        /*-- EXISTENCE FLAG -- */

    /*-- Output image (required) --*/
      string output_image_file;       /*-- USER INPUT IMAGE FILENAME --*/

    /* Program version */
      string version;                 /* -- PROGRAM PARAMETER --*/

    // FRIEND FUNCTIONS and CLASSES
    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
