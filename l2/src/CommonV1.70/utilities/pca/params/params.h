/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Params class, which is a class 
   >>>>                 holding program parameters.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  April 4, 2014
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
      string version;		   /* -- PROGRAM PARAMETER --*/

    // Member variables (public)
    /*-- Input image (required) --*/
      string input_image_file;     /*-- USER INPUT IMAGE FILENAME --*/

    /*-- Number of components retained (optional, no default) --*/
      int number_components;      /*-- USER INPUT PARAMETER --*/
      bool number_flag;           /*-- EXISTENCE FLAG --*/

    /*-- Retained percent variance (optional, default provided) --*/
      float variance;              /*-- USER INPUT PARAMETER --*/

    /*-- Output image file name (required) --*/
      string output_image_file;	   /*-- USER INPUT PARAMETER --*/

    // FRIEND FUNCTIONS and CLASSES
    protected:

    private:
};

string process_line(const string& line, const bool& list_flag);

#endif /* PARAMS_H */
