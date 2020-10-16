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

    /*-- Formatted multispectral input image data file (required) --*/
      string   multispectral_image_file;        /*-- INPUT MULTISPECTRAL IMAGE FILE NAME --*/
      bool     multispectral_image_flag;        /*-- FLAG --*/

    /*-- Formatte panchromatic input image data file to be extracted from (required) --*/
      string   panchromatic_image_file;        /*-- INPUT PANCHROMATIC IMAGE FILE NAME FOR IMAGE TO BE EXTRACTED FROM --*/
      bool     panchromatic_image_flag;        /*-- FLAG --*/

    /*-- Registered output extract panchromatic image data file (required) --*/
      string   extracted_panchromatic_image_file;        /*-- OUTPUT EXTRACTED PANCHROMATIC IMAGE FILE NAME --*/
      bool     extracted_panchromatic_image_flag;        /*-- FLAG --*/

    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
