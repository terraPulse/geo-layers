#ifndef PARAMS_H
#define PARAMS_H

#include "../band_differences.h"
#include <image/image.h>
#include <string>
#include <sstream>
#include <stdexcept>

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
      void print();
      void set_temp_files();
      void remove_temp_files();

    // Member variables (public)
      string version;              /* -- PROGRAM PARAMETER --*/

    /*-- Input image data file name (required) --*/
      string   input_image_file;        /*-- BASE INPUT IMAGE FILE NAME --*/

    /*-- Number of columns in input image data file (required) --*/
      int    ncols;                /*-- USER INPUT PARAMETER --*/

    /*-- Number of rows in input image data file (required) --*/
      int    nrows;                /*-- USER INPUT PARAMETER --*/

    /*-- Number of spectral bands in input image data file (required) --*/
      int    nbands;		   /*-- USER INPUT PARAMETER --*/

    /*-- Data type of input image data (required) --*/
      RHSEGDType dtype;   	   /*-- USER INPUT PARAMETER --*/

    /*-- Input data mask file (optional) --*/
      string mask_file;            /*-- USER INPUT FILENAME --*/
      bool   mask_flag;            /*-- EXISTENCE FLAG --*/

    /*-- Input mask value (optional) --*/
      int    mask_value;	   /*-- USER INPUT PARAMETER --*/

    /*-- Output band differences image data file name (required) --*/
      string   band_diff_image_file;        /*-- OUTPUT REGISTERED IMAGE FILE NAME --*/

    /*-- Temporary file --*/
      string   temp_NIR_image_file;
      string   temp_red_image_file;

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

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
