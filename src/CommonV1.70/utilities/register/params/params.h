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
      string version;              /* -- PROGRAM PARAMETER --*/

    /*-- ENVI format base input image data file (required) --*/
      string   base_image_file;        /*-- BASE INPUT IMAGE FILE NAME --*/
      bool     base_image_flag;        /*-- FLAG --*/

    /*-- ENVI format input image data file to be registered (required) --*/
      string   register_image_file;        /*-- INPUT IMAGE FILE NAME FOR IMAGE TO BE REGISTERED--*/
      bool     register_image_flag;        /*-- FLAG --*/

    /*-- Registered output image data file (required) --*/
      string   OUT_registered_image_file;        /*-- OUTPUT REGISTERED IMAGE FILE NAME --*/
      bool     OUT_registered_image_flag;        /*-- FLAG --*/

    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
