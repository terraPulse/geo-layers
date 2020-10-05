/*-----------------------------------------------------------
|
|  Routine Name: do_termination - Send termination requests
|
|       Purpose: Send termination request to all nodes that are currently in server mode.
|
|         Input: recur_level   (Lowest recursive level at which this task is active)
|                stride        (Number of data sections (or tasks) covered by each child task)
|                nb_tasks      (Number of data sections (or tasks) covered by this task)
|                
|        Output:
|
|         Other: temp_data     (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: September 12, 2002.
| Modifications: May 16, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|
------------------------------------------------------------*/
#include <defines.h>
#include <rhseg/hseg.h>
#include <params/params.h>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 void do_termination(const short unsigned int& recur_level, Temp& temp_data)
 {
   if (recur_level < (params.onb_levels-1))
   {
     short unsigned int next_recur_level = recur_level + 1;
#ifdef THREEDIM
     parallel_recur_requests((short unsigned int) 0,recur_level,0,0,0,0,0,0,0,temp_data);
#else
     parallel_recur_requests((short unsigned int) 0,recur_level,0,0,0,0,0,0,temp_data);
#endif
     do_termination(next_recur_level,temp_data);
   }

   return;
 }
} // namespace HSEGTilton
