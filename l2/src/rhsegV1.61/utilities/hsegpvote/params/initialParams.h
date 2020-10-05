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
      void set_temp_files();
      void print();
      void print_oparam();
      void remove_temp_files();

    // Member variables (public)

    /*-- Input pixel-based classification (required) --*/
      string  pixel_class_file;    /*-- USER INPUT IMAGE FILENAME --*/

    /*-- RHSEG output parameter file (required) --*/
      string  oparam_file;         /*-- USER INPUT FILE NAME --*/

    /*-- Input training set label data (required) --*/
      string training_label_file;  /*-- USER INPUT IMAGE FILENAME --*/

    /*-- Flag requesting use of region classes even if region objects are available --*/
      bool   region_class_flag;       /*-- USER INPUT PARAMETER --*/

    /*-- Color Table (file) for output classification (optional) --*/
      string color_table_file;        /*-- USER SUPPLIED COLOR TABLE FILENAME --*/
      bool   color_table_flag;        /*-- EXISTENCE FLAG -- */

    /*-- Output class optimized classification (required) --*/
      string  opt_class_file;      /*-- USER INPUT FILENAME --*/

    /*-- Output log file (required) --*/
      string  log_file;            /*-- USER INPUT FILENAME --*/
      fstream log_fs;              /*-- ASSOCIATED FILE STREAM --*/

    /* Default filename prefix */
      string prefix;               /* -- PROGRAM PARAMETER --*/

    /*-- Temporary files --*/
      string   temp_seg_level_label_file;
      string   temp_object_label_file;
      string   temp_region_class_file;
      string   temp_mask_file;

    /*-- Exit status from paramGUI --*/
      bool status;                        /*-- PROGRAM PARAMETER --*/

    protected:

    private:

  };

} // HSEGTilton

#endif /* INITIALPARAMS_H */
