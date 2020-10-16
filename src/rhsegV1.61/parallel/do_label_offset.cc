/*-----------------------------------------------------------
|
|  Routine Name: do_class_label_offset
|
|       Purpose: Adds the offset, class_label_offset to the region_classes label
|
|         Input: recur_level      (Lowest recursive level at which this task is active)
|                class_label_offset    (Offset to be added to region indices)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                
|        Output:
|
|         Other: temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: September 13, 2002.
| Modifications: February 10, 2003:  Changed region index to region label
|                June 11, 2003 - Modified the temp_data structure and its use.
|                December 22, 2004 - Changed region label from short unsigned int to unsigned int
|                January 5, 2006 - Added slice dimension (extension to three-dimensional analysis)
|                May 16, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|
------------------------------------------------------------*/
#include <defines.h>
#include <params/params.h>
#include <pixel/pixel.h>
#include <spatial/spatial.h>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 void do_class_label_offset(const short unsigned int& recur_level,
                       unsigned int class_label_offset, vector<Pixel>& pixel_data,
                       Temp& temp_data)
 {
   unsigned int region_label;
   unsigned int pixel_index;

   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the class_label_offset can be applied directly to the region_label.
     unsigned int pixel_data_size = pixel_data.size();
     for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
     {
       region_label = pixel_data[pixel_index].get_region_label();
       if (region_label != 0)
         pixel_data[pixel_index].set_region_label(region_label + class_label_offset);
     }
     if (params.debug > 2)
     {
       if (class_label_offset > 0)
          params.log_fs << "Performed region label offset of " << class_label_offset;
       params.log_fs << " for task " << params.myid << endl;
     }
   }
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // a recursive call must be made.
#ifdef THREEDIM
     parallel_recur_requests((short unsigned int) 1, recur_level, class_label_offset, 0, 0, 0, 0, 0, 0, temp_data);
#else
     parallel_recur_requests((short unsigned int) 1, recur_level, class_label_offset, 0, 0, 0, 0, 0, temp_data);
#endif
   // Process current task
     int stride, nb_tasks;
     set_stride_sections(recur_level,stride,nb_tasks);
     short unsigned int next_recur_level = recur_level + 1;
     do_class_label_offset(next_recur_level,class_label_offset,pixel_data,temp_data);

     int class_label_offset_tag = 40;
   // Receive completion confirmation from the parallel recur_tasks
     int min_taskid = params.myid + stride;
     int max_taskid = params.myid + nb_tasks;
#ifdef TIME_IT
     float end_time, elapsed_time;
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
     for (int recur_taskid = min_taskid; recur_taskid < max_taskid; recur_taskid += stride)
     {
       MPI::COMM_WORLD.Recv(&class_label_offset, 1, MPI::UNSIGNED, recur_taskid, class_label_offset_tag);
     }
#ifdef TIME_IT
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
   }
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: do_object_label_offset
|
|       Purpose: Adds the offset, class_label_offset to the region_objects label
|
|         Input: recur_level      (Lowest recursive level at which this task is active)
|                object_label_offset (Offset to be added to region_object indices)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                
|        Output:
|
|         Other: temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: September 13, 2002.
| Modifications: (See comments for do_class_label_offset function)
|
------------------------------------------------------------*/
 void do_object_label_offset(const short unsigned int& recur_level,
                            unsigned int object_label_offset, Spatial& spatial_data,
                            Temp& temp_data)
 {
   unsigned int region_object_label, pixel_index;

   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the object_label_offset can be applied directly to the region_object_label_map.
#ifdef THREEDIM
     unsigned int npixels = params.pixel_ncols*params.pixel_nrows*params.pixel_nslices;
#else
     unsigned int npixels = params.pixel_ncols*params.pixel_nrows;
#endif
     for (pixel_index = 0; pixel_index < npixels; ++pixel_index)
     {
       region_object_label = spatial_data.region_object_label_map[pixel_index];
       if (region_object_label != 0)
         spatial_data.region_object_label_map[pixel_index] = region_object_label + object_label_offset;
     }
     if (params.debug > 2)
     {
       if (object_label_offset > 0)
          params.log_fs << "Performed connected region label offset of " << object_label_offset;
       params.log_fs << " for task " << params.myid << endl;
     }
   }
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // a recursive call must be made.
#ifdef THREEDIM
     parallel_recur_requests((short unsigned int) 1, recur_level, 0, 0, 0, 0, object_label_offset, 0, 0, temp_data);
#else
     parallel_recur_requests((short unsigned int) 1, recur_level, 0, 0, 0, object_label_offset, 0, 0, temp_data);
#endif
   // Process current task
     int stride, nb_tasks;
     set_stride_sections(recur_level,stride,nb_tasks);
     short unsigned int next_recur_level = recur_level + 1;
     do_object_label_offset(next_recur_level,object_label_offset,spatial_data,temp_data);

     int class_label_offset_tag = 40;
   // Receive completion confirmation from the parallel recur_tasks
     int min_taskid = params.myid + stride;
     int max_taskid = params.myid + nb_tasks;
#ifdef TIME_IT
     float end_time, elapsed_time;
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
     for (int recur_taskid = min_taskid; recur_taskid < max_taskid; recur_taskid += stride)
     {
       MPI::COMM_WORLD.Recv(&object_label_offset, 1, MPI::UNSIGNED, recur_taskid, class_label_offset_tag);
     }
#ifdef TIME_IT
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
   }
   return;
 }
} // namespace HSEGTilton

