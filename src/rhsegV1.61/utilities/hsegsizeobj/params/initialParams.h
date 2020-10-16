#ifndef INITIALPARAMS_H
#define INITIALPARAMS_H

#include <defines.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

namespace HSEGTilton
{

  class InitialParams
  {
    public:
    // Constructor and Destructor
      InitialParams();
      virtual ~InitialParams();

    // Member functions
      bool read(const char *param_file);
      bool read_oparam();
//      void set_temp_files();
      void print();
      void print_oparam();
//      void remove_temp_files();

    // Member variables (public)

    /*-- RHSEG output parameter file (required) --*/
      string  oparam_file;         /*-- USER INPUT FILE NAME --*/

    /*-- Output sizenpix image file (optional) --*/
      string  sizenpix_file;       /*-- USER INPUT FILENAME --*/

    /*-- Output sizeobj image file (required) --*/
      string  sizeobj_file;        /*-- USER INPUT FILENAME --*/
    /*-- Output sizeobj image file (required) --*/
      unsigned int  levels;        /*-- USER INPUT FILENAME --*/
    /*-- Output sizeobj image file (required) --*/
      unsigned int  sizethres[100];        /*-- USER INPUT FILENAME --*/
      bool sizenpix_flag;
      bool sizeobj_flag;

    /* Default filename prefix */
//      string prefix;               /* -- PROGRAM PARAMETER --*/

    /*-- Temporary files --*/
      string   temp_seg_level_label_file;
/*
      string   temp_object_label_file;
      string   temp_region_class_file;
      string   temp_mask_file;
*/
    /*-- Exit status from paramGUI --*/
      bool status;                        /*-- PROGRAM PARAMETER --*/

    protected:

    private:

  };

} // HSEGTilton

#endif /* INITIALPARAMS_H */
