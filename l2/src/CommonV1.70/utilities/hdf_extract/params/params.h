/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Params class, which is a class 
   >>>>                 holding program parameters.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  January 15, 2014
   >>>> Modifications:  
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */
#ifndef PARAMS_H
#define PARAMS_H

#include <string>
#include <fstream>
#include <sstream>
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
      string version;			/* -- PROGRAM PARAMETER --*/

    /*-- Input HDF file name (required) --*/
      string input_HDF;                 /*-- USER INPUT PARAMETER --*/

    /*-- List of subdatasets to be extracted (required) --*/
      vector<string> subdatasets;       /*-- USER INPUT PARAMETER --*/
      int nbands;                       /*-- PROGRAM PARAMETER --*/

    /*-- Output image file name (required) --*/
      string output_image;		/*-- USER INPUT PARAMETER --*/

    /*-- Output image format (optional, default = GTiff) --*/
      string output_format;		/*-- USER INPUT PARAMETER --*/

    // FRIEND FUNCTIONS and CLASSES
    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
