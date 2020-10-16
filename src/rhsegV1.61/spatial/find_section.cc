/*-----------------------------------------------------------
|
|  Routine Name: find_section
|
|       Purpose: Finds the section that contains information on data at a given
|                row and column offsets.
|
|         Input: col_offset       (Column offset)
|                row_offset       (Row offset)
|                slice_offset     (Slice offset)
|
|        Output:
|
|       Returns: section           (Parallel: Task ID number; Serial: Processing section)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: Adapted from parallel/find_taskid on May 30, 2005
| Modifications: October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|
------------------------------------------------------------*/

#include <params/params.h>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef THREEDIM
  short unsigned int find_section(const int& col_offset, const int& row_offset, const int& slice_offset)
#else
  short unsigned int find_section(const int& col_offset, const int& row_offset)
#endif
  {
    short unsigned int section = 0, index;

    switch (params.nb_dimensions)
    {
       case 1: for (section = 0; section < params.nb_sections; ++section)
               {
#ifdef PARALLEL
                 index = section - params.myid;
#else
                 index = section;
#endif
                 if ((params.col_offset[index] == col_offset))
                 break;
               }
               break;
       case 2: for (section = 0; section < params.nb_sections; ++section)
               {
#ifdef PARALLEL
                 index = section - params.myid;
#else
                 index = section;
#endif
                 if ((params.col_offset[index] == col_offset) &&
                     (params.row_offset[index] == row_offset))
                 break;
               }
               break;
       case 3: for (section = 0; section < params.nb_sections; ++section)
               {
#ifdef PARALLEL
                 index = section - params.myid;
#else
                 index = section;
#endif
#ifdef THREEDIM
                 if ((params.col_offset[index] == col_offset) &&
                     (params.row_offset[index] == row_offset) &&
                     (params.slice_offset[index] == slice_offset))
#else
                 if ((params.col_offset[index] == col_offset) &&
                     (params.row_offset[index] == row_offset))
#endif
                 break;
               }
               break;
    }

    return section;
  }
} // namespace HSEGTilton
