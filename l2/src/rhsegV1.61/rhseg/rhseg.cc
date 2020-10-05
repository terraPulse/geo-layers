/*-----------------------------------------------------------
|
|  Routine Name: rhseg - Recursive version of Hierarchical Segmentation
|
|       Purpose: Main function for the recursive version of HSEG
|
|         Input: spatial_data     (Class which holds information pertaining to input and output spatial data)
|                pixel_data       (Class which holds information pertaining to the pixels processed by this task)
|                
|        Output: region_classes      (Class which holds region related information)
|                max_threshold    (Maximum merging threshold encountered)
|
|         Other: temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: nregions         (Current number of regions)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: December 9, 2002.
| Modifications: February 7, 2003 - Changed region_index to region_label
|                May 20, 2003 - Added option to initialize with Classical Region Growing.
|                September 24, 2003 - Eliminated use of the index_class class (index_data).
|                May 29, 2005 - Added temporary file I/O for faster processing of large data sets
|                October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                September 5, 2008 - Modified rhseg call vis-a-vis nregions & max_threshold
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|
------------------------------------------------------------*/

#include "hseg.h"
#include <params/params.h>
#include <region/region_class.h>
#include <spatial/spatial.h>
#include <pixel/pixel.h>

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{
 unsigned int rhseg(unsigned int& nregions, double& max_threshold, Spatial& spatial_data, 
                    vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, 
                    vector<RegionClass *>& nghbr_heap, vector<RegionClass *>& region_heap, Temp& temp_data)
 {
   short unsigned int section = 0;

   int ncols = params.padded_ncols;
   int nrows = params.padded_nrows;
#ifdef THREEDIM
   int nslices = params.padded_nslices;
#endif
#ifdef PARALLEL
   section = params.myid;
   bool col_flag, row_flag;
#ifdef THREEDIM
   bool slice_flag;
#endif
 // The following code will only be excuted for params.recur_level > 0, i.e. in params.program_mode RHSEG (=3)
   for (short unsigned int recur_level = 0; recur_level < params.recur_level; ++recur_level)
   {
#ifdef THREEDIM
     set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag,slice_flag);
#else
     set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag);
#endif
     if (col_flag)
       ncols /= 2;
     if (row_flag)
       nrows /= 2;
#ifdef THREEDIM
     if (slice_flag)
       nslices /= 2;
#endif
   }
#endif

   unsigned int global_nregions = nregions;
#ifdef THREEDIM
   nregions = lrhseg(0, params.recur_level, section, ncols, nrows, nslices, global_nregions, max_threshold,
                     spatial_data, pixel_data, region_classes, nghbr_heap, region_heap, temp_data);
#else
   nregions = lrhseg(0, params.recur_level, section, ncols, nrows, global_nregions, max_threshold,
                     spatial_data, pixel_data, region_classes, nghbr_heap, region_heap, temp_data);
#endif

#ifdef PARALLEL
   if ((params.recur_level > 0) && (params.recur_level < params.onb_levels))
   {
   // This task must respond to requests from tasks running at recursive levels
   // along the chain that had called this task.
     parallel_server(section, spatial_data, pixel_data, region_classes,
                     nghbr_heap, region_heap, temp_data);
     if (params.debug > 0)
     {
       params.log_fs << "This node has completed its task";
       params.log_fs << endl;
     }
     return nregions;
   }
   else if (params.recur_level > 0)
   {
     if (params.debug > 0)
     {
       params.log_fs << "This node has completed its task";
       params.log_fs << endl;
     }
     return nregions;
   }
 // The only task that continues to this point and beyond is the task that initiated
 // overall chain of recursive calls.
#endif // PARALLEL
#ifdef DEBUG
   unsigned int pixel_index, pixel_data_size = pixel_data.size( );
   if (params.debug > 3)
   {
     params.log_fs << endl << "After calls to lrhseg in hseg, dump of the pixel data:" << endl << endl;
     for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
     {
       pixel_data[pixel_index].print(pixel_index);
     }
   }
   unsigned int region_index, region_classes_size = region_classes.size( );
   if (params.debug > 3)
   {
     params.log_fs << endl << "After calls to lrhseg in hseg, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag( ))
         region_classes[region_index].print(region_classes);
   }
#endif
   return nregions;
 }
} // namespace HSEGTilton

