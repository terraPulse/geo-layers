#ifndef INITIALPARAMS_H
#define INITIALPARAMS_H

#include <string>
#include <fstream>

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
      void print();

    /*-- RHSEG output parameter file (required) --*/
      string   oparam_file;        /*-- USER INPUT FILE NAME --*/
      bool     oparam_flag;        /*-- FLAG --*/

    /*-- Flag for including the list of the region classes neighboring each region object (optional) --*/
      bool   region_class_nghbrs_list_flag;      /*-- USER INPUT PARAMETER --*/

    /*-- Flag for including the list of the region objects neighboring each region object (optional) --*/
      bool   region_object_nghbrs_list_flag;     /*-- USER INPUT PARAMETER --*/

    /*-- Debug option (optional) --*/
      int    debug;                       /*-- USER INPUT PARAMETER --*/

    /*-- Output log file (optional) --*/
      string  log_file;            /*-- USER INPUT FILENAME --*/

    protected:

    private:
  };

} // HSEGTilton

#endif /* INITIALPARAMS_H */

