/*-----------------------------------------------------------
|
|  Routine Name: update_region_label
|
|       Purpose: Updates pixel_data.region_label from spatial_data.region_object_label_map
|
|         Input: recur_level      (Lowest recursive level at which this task is active)
|                section          (Section or window processed by this call to update_region_label)
|                spatial_data     (Class which holds information pertaining to the spatial data)
|                
|        Output:
|
|         Other: pixel_data       (Class which holds information pertaining to the pixels processed by this task)
|                temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: January 29, 2003.
| Modifications: February 10, 2003:  Changed region index to region label
|                June 11, 2003 - Modified the temp_data structure and its use.
|                October 10, 2003 - Eliminated the use of index_data
|                June 1, 2005 - Added temporary file I/O for faster processing of large data sets
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|
------------------------------------------------------------*/

#include "spatial.h"
#include <params/params.h>
#include <pixel/pixel.h>
#ifdef TIME_IT
#include <ctime>
#endif

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 void update_region_label(const short unsigned int& recur_level, const short unsigned int& section,
                          Spatial& spatial_data, vector<Pixel>& pixel_data,
                          Temp& temp_data)
 {
   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of spatial_data.update_region_label can be called directly.
     spatial_data.update_region_label(pixel_data);
   }
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // update_region_label must be called recursively.
     short unsigned int next_recur_level = recur_level + 1;
     int stride, nb_sections;
     set_stride_sections(recur_level,stride,nb_sections);
#ifdef PARALLEL
    // Send request to the parallel recursive tasks
#ifdef THREEDIM
     parallel_recur_requests( (short unsigned int) 15, recur_level, 0, 0, 0, 0, 0, 0, 0, temp_data);
#else
     parallel_recur_requests( (short unsigned int) 15, recur_level, 0, 0, 0, 0, 0, 0, temp_data);
#endif
    // Process current task's data section
     update_region_label(next_recur_level,section,spatial_data,pixel_data,temp_data);

     int      update_region_label_tag = 115;
     unsigned int value;
     int min_section = section + stride;
#ifdef TIME_IT
     float end_time, elapsed_time;
#endif
#else
     int min_section = section;
#endif
     int max_section = section + nb_sections;
     for (int recur_section = min_section; recur_section < max_section; recur_section += stride)
     {
#ifdef PARALLEL
     // Receive confirmation back from the individual tasks
#ifdef DEBUG
       if (params.debug > 2)
         params.log_fs << "Waiting for return from update region_label request from task " << recur_section << endl;
#endif
#ifdef TIME_IT
       end_time = (((float) clock()/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
       temp_data.start_time = (((float) clock()/((float) CLOCKS_PER_SEC));
#endif
       MPI::COMM_WORLD.Recv(&value, 1, MPI::UNSIGNED, recur_section, update_region_label_tag);
#ifdef TIME_IT
       end_time = (((float) clock()/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
       temp_data.start_time = (((float) clock()/((float) CLOCKS_PER_SEC));
#endif
#else // !PARALLEL
      // In the serial case, process the specified data section
       if ((next_recur_level == (params.onb_levels-1)) && (params.nb_sections > 1))
       {
         restore_pixel_data(recur_section,pixel_data,temp_data);
         spatial_data.restore_region_object_label_map(recur_section);
       }
       update_region_label(next_recur_level,recur_section,spatial_data,pixel_data,temp_data);
       if ((next_recur_level == (params.onb_levels-1)) && (params.nb_sections > 1))
       {
         save_pixel_data(recur_section,pixel_data,temp_data);
         spatial_data.save_region_object_label_map(recur_section);
       }
#endif
     } // for (int recur_section = min_section; recur_section < max_section; recur_section += stride)
   }

   return;
 }
} // namespace HSEGTilton
