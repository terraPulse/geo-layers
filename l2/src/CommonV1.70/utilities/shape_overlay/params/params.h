#ifndef PARAMS_H
#define PARAMS_H

#include <string>

using namespace std;

namespace CommonTilton
{

  class Params 
  {
    public:
    // Constructor and Destructor
      Params(const string& value);
      virtual ~Params();

    // Member functions
      void print_version();
      bool read(const char *param_file);
      void set_temp_files();
      void print();
      void remove_temp_files();

    // Member variables (public)
      string version;              /* -- PROGRAM PARAMETER --*/

    /*-- Formatted input image data file (required) --*/
      string   input_image_file;        /*-- INPUT IMAGE FILE NAME --*/

    /*-- Input shape file (required) --*/
      string   shape_file;              /*-- INPUT SHAPE FILE NAME --*/

    /*-- Red, Green and Blue values for overlay display of the input shape file (required) --*/
      int      red, green, blue;

    /*-- Output image data file (required) --*/
      string   output_image_file;       /*-- OUTPUT IMAGE FILE NAME --*/

    /*-- Temporary file --*/
      string   temp_output_image_file;  /* -- Temporary version of output image file --*/

    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
