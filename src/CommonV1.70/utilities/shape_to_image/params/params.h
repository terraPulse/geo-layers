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
      void print();

    // Member variables (public)
      string version;                   /* -- PROGRAM PARAMETER --*/

    /*-- Input shape file (required) --*/
      string   shape_file;              /*-- INPUT SHAPE FILE NAME --*/

    /*-- Formatted input mask file (required) --*/
      string   mask_image_file;         /*-- INPUT MASK IMAGE FILE NAME --*/

    /*-- Output image data file (required) --*/
      string   output_image_file;       /*-- OUTPUT IMAGE FILE NAME --*/

    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
