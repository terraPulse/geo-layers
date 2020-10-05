/*-----------------------------------------------------------
|
|  Routine Name: object_nghbrs_set_init
|
|       Purpose: Initialize region_objects.class_nghbrs_set and region_objects.object_nghbrs_set from current spatial_data values
|
|         Input: params           (Program parameters via Params structure)
|                spatial_data     (Class which holds information pertaining to the spatial data)
|                log_fs           (Output log file)
|
|        Output: region_objects (Class which holds region object related information)
|
|         Other:
|
|       Returns: (void)
|
|    Written By: James C. Tilton
|          Date: October 6, 2004
| Modifications: December 23, 2004 - Changed region label from short unsigned int to unsigned int
|                December 16, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                February 20, 2008 - Modified to work with hsegreader
|
------------------------------------------------------------*/

#include <params/params.h>
#include <spatial/spatial.h>
#include <region/region_class.h>
#include <region/region_object.h>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 void object_nghbrs_set_init(const short unsigned int& hlevel, Spatial& spatial_data,
                           vector<RegionClass>& region_classes, vector<RegionObject>& region_objects)
 {
   unsigned int region_index, region_label;
   unsigned int pixel_index, region_objects_size = region_objects.size( );
   int col, row;
#ifdef THREEDIM
   int slice;
#endif
   for (region_index = 0; region_index < region_objects_size; region_index++)
   {
     region_objects[region_index].class_nghbrs_set_clear();
     region_objects[region_index].object_nghbrs_set_clear();
   }
#ifdef THREEDIM
   for (slice = 0; slice < params.nslices; slice++)
   {
#endif
    for (row = 0; row < params.nrows; row++)
     for (col = 0; col < params.ncols; col++)
     {
#ifdef THREEDIM
       pixel_index = col + row*params.ncols + slice*params.nrows*params.ncols;
#else
       pixel_index = col + row*params.ncols;
#endif
       region_label = spatial_data.get_region_object_label(pixel_index);
       if (region_label > 0)
       {
         region_index = region_label - 1;
         if (hlevel > 0)
         {
           if (!region_objects[region_index].get_active_flag())
             region_label = region_objects[region_index].get_merge_region_label();
           region_index = region_label - 1;
         }
#ifdef THREEDIM
         region_objects[region_index].object_nghbrs_set_init(hlevel,spatial_data,
                                                             region_classes,region_objects,col,row,slice);
#else
         region_objects[region_index].object_nghbrs_set_init(hlevel,spatial_data,
                                                             region_classes,region_objects,col,row);
#endif
       }
     }
#ifdef THREEDIM
   }
#endif
   return;
 }

} // namespace HSEGTilton
