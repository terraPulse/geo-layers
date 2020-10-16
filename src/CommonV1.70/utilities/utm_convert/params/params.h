/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Params class, which is a class 
   >>>>                 holding program parameters.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  September 12, 2012
   >>>> Modifications:  
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */
#ifndef PARAMS_H
#define PARAMS_H

#include <image/image.h>
#include <string>
#include <fstream>
#include <sstream>

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

    /*-- Latitude value (optional) --*/
      double latitude;                /*-- USER INPUT PARAMETER --*/
      bool   latitude_flag;           /*-- EXISTENCE FLAG --*/

    /*-- Longitude value (optional) --*/
      double longitude;               /*-- USER INPUT PARAMETER --*/
      bool   longitude_flag;          /*-- EXISTENCE FLAG --*/

    /*-- UTM_X value (optional) --*/
      double UTM_X;		      /*-- USER INPUT PARAMETER --*/
      bool   UTM_X_flag;              /*-- EXISTENCE FLAG --*/

    /*-- UTM_Y value (optional) --*/
      double UTM_Y;		      /*-- USER INPUT PARAMETER --*/
      bool   UTM_Y_flag;              /*-- EXISTENCE FLAG --*/

    /*-- WGS 84 / UTM Zone number (required) --*/
      int WGS_84_UTM_Zone;            /*-- USER INPUT PARAMETER --*/

    /*-- Latitude/Longitude flag --*/
      bool lat_long_flag;             /*-- PROGRAM PARAMETER --*/

    // FRIEND FUNCTIONS and CLASSES
    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
