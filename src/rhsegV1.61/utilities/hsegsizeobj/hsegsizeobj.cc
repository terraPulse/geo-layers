/*-----------------------------------------------------------
|
|  Routine Name: hsegsizeobj - Revision of Panshi Wang's sizeobj.c program
|
|       Purpose: Main function for the hsegsizeobj program
|
|         Input: 
|
|        Output: 
|
|       Returns: TRUE (1) on success, FALSE (0) on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|		 Based on sizeobj.c program written by Panshi Wang, U of MD
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: September 15, 2014
| Modifications: 
|
------------------------------------------------------------*/

#include "hsegsizeobj.h"
#include "params/initialParams.h"
#include "params/params.h"
#include <iostream>
#include <cmath>
extern HSEGTilton::InitialParams initialParams;
extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{

  bool hsegsizeobj()
  {
    initialParams.print();
    sizeobj();

    return true;
  }

} // namespace HSEGTilton

