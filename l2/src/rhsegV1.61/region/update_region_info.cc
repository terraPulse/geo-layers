/*-----------------------------------------------------------
|
|  Routine Name: update_region_class_info (regular version)
|
|       Purpose: Updates region_classes.boundary_npix, and region_classes.region_objects_set values
|
|         Input: ncols         (Number of columns of spatial_data)
|                nrows         (Number of rows of spatial_data)
|                nslices       (Number of slices of spatial_data)
|                nslevels      (Number of segmentation levels saved)
|                spatial_data  (Class which holds information pertaining to the spatial data)
|
|        Output: region_classes   (Class which holds region related information)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: November 27, 2003
| Modifications: June 2, 2005 - Added temporary file I/O for faster processing of large data sets
|                January 4, 2006 - Added slice dimension (extension to three-dimensional analysis)
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|
------------------------------------------------------------*/

#include "region_class.h"
#include "region_object.h"
#include <params/params.h>
#include <spatial/spatial.h>
#include <pixel/pixel.h>
#include <vector>
#ifdef TIME_IT
#include <ctime>
#endif

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef THREEDIM
 void update_region_class_info(const int& ncols, const int& nrows,
                         const int& nslices, const short unsigned int& nslevels,
                         Spatial& spatial_data, vector<RegionClass>& region_classes)
#else
 void update_region_class_info(const int& ncols, const int& nrows,
                         const short unsigned int& nslevels,
                         Spatial& spatial_data, vector<RegionClass>& region_classes)
#endif
 {
   bool  boundary_flag;
   unsigned int region_class_label, region_index;
   unsigned int pixel_index, region_object_label = 0;
   int col, row;
#ifdef THREEDIM
   int slice;

   for (slice = 0; slice < nslices; slice++)
   {
#endif
    for (row = 0; row < nrows; row++)
     for (col = 0; col < ncols; col++)
     {
#ifdef THREEDIM
       pixel_index = col + row*ncols + slice*nrows*ncols;
#else
       pixel_index = col + row*ncols;
#endif
       region_class_label = spatial_data.get_region_class_label(pixel_index);
       if (region_class_label != 0)
       {
         region_index = region_class_label - 1;
         if (params.region_nb_objects_flag)
           region_object_label = spatial_data.get_region_object_label(pixel_index);
         boundary_flag = (spatial_data.get_boundary_map(pixel_index) == (nslevels+1));
         region_classes[region_index].update_region_class_info(boundary_flag,
                                                      region_object_label);
       }  //  if (region_class_label != 0)
     }
#ifdef THREEDIM
   }
#endif
#ifdef DEBUG
   unsigned int region_classes_size = region_classes.size( );
   if (params.debug > 3)
   {
     params.log_fs << endl << "After update_region_class_info, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag( ))
         region_classes[region_index].print();
   }
#endif
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: update_region_class_info (recursive version)
|
|       Purpose: Updates region_classes.boundary_npix, and region_classes.region_objects_set values
|
|         Input: recur_level   (Lowest recursive level at which this task is active)
|                section       (Section or window processed by this call to do_region_relabel)
|                nslevels      (Number of segmentation levels saved)
|                spatial_data  (Class which holds information pertaining to the spatial data)
|
|        Output: region_classes   (Class which holds region related information)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: November 27, 2003
| Modifications: (see comments for first update_region_class_info function)
|
------------------------------------------------------------*/
 void update_region_class_info(const short unsigned int& recur_level,
                         const short unsigned int& section, const short unsigned int& nslevels,
                         Spatial& spatial_data, vector<RegionClass>& region_classes,
                         Temp& temp_data)
 {
#ifdef DEBUG
   unsigned int region_index;
   unsigned int region_classes_size = region_classes.size( );
   if (params.debug > 3)
   {
     params.log_fs << endl << "Entering update_region_class_info, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag( ))
         region_classes[region_index].print();
   }
#endif

   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of update_region_class_info can be called.
#ifdef THREEDIM
     update_region_class_info(params.pixel_ncols,params.pixel_nrows,params.pixel_nslices,
                        nslevels,spatial_data,region_classes);
#else
     update_region_class_info(params.pixel_ncols,params.pixel_nrows,
                        nslevels,spatial_data,region_classes);
#endif
   }
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // update_region_class_info must be called recursively.
     int stride, nb_sections;
     set_stride_sections(recur_level,stride,nb_sections);
#ifdef PARALLEL
#ifndef DEBUG
     unsigned int region_index;
     unsigned int region_classes_size = region_classes.size( );
#endif
   // Send region_classes to the parallel recur_tasks
#ifdef THREEDIM
     parallel_recur_requests((short unsigned int) 2, recur_level, 0, 0, (int) nslevels, 0,
                             region_classes_size, 0, 0, temp_data);
#else
     parallel_recur_requests((short unsigned int) 2, recur_level, 0, 0, (int) nslevels,
                             region_classes_size, 0, 0, temp_data);
#endif
   // Process current task.
     update_region_class_info((recur_level+1),section,nslevels,spatial_data,region_classes,temp_data);

     unsigned int short_buf_size = 0, int_buf_size = 0, double_buf_size = 0;
     int sum_pixel_gdissim_tag = 102;
   // Receive information back from the individual tasks
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
// Receive region_classes.boundary_npix and region_classes.sum_pixel_gdissim
       short_buf_size = double_buf_size = 0;
#ifdef TIME_IT
       end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
       temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
       MPI::COMM_WORLD.Recv(&int_buf_size, 1, MPI::UNSIGNED,
                            recur_section, sum_pixel_gdissim_tag);
#ifdef TIME_IT
       end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
       temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
       check_buf_size(0,short_buf_size,int_buf_size,0,double_buf_size,
                      temp_data);
       MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                            recur_section, sum_pixel_gdissim_tag);
#ifdef TIME_IT
       end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
       temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif

       unsigned int int_buf_position = 0;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         region_classes[region_index].update_region_info(temp_data,int_buf_position);
       if (params.debug > 3)
         params.log_fs << "Received region_info data from task " << recur_section << endl;
#else // !PARALLEL
     // In the serial case, process the specified data section
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
       {
         spatial_data.restore_region_class_label_map(recur_section);
         if (params.region_nb_objects_flag)
           spatial_data.restore_region_object_label_map(recur_section);
         spatial_data.restore_boundary_map(recur_section);
       }
       update_region_class_info((recur_level+1),recur_section,nslevels,spatial_data,region_classes,temp_data);
#endif // !PARALLEL
#ifdef DEBUG
       if (params.debug > 3)
       {
         params.log_fs << endl << "Dump of the region data:" << endl << endl;
         for (region_index = 0; region_index < region_classes_size; ++region_index)
           if (region_classes[region_index].get_active_flag( ))
             region_classes[region_index].print();
       }
#endif
     } // for (int recur_section = min_section; recur_section < max_section; recur_section += stride)
   }
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After update_region_class_info, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag( ))
         region_classes[region_index].print();
   }
#endif
   return;
 }
/*-----------------------------------------------------------
|
|  Routine Name: update_region_object_info (regular version)
|
|       Purpose: Updates region_objects.boundary_npix values
|
|         Input: ncols         (Number of columns of spatial_data)
|                nrows         (Number of rows of spatial_data)
|                nslices       (Number of slices of spatial_data)
|                nslevels      (Number of segmentation levels saved)
|                spatial_data  (Class which holds information pertaining to the spatial data)
|
|        Output: region_objects   (Class which holds connected region related information)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: November 27, 2003
| Modifications: (see comments for first update_region_class_info function)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void update_region_object_info(const int& ncols, const int& nrows,
                              const int& nslices, const short unsigned int& nslevels,
                              Spatial& spatial_data, vector<RegionObject>& region_objects)
#else
 void update_region_object_info(const int& ncols, const int& nrows,
                              const short unsigned int& nslevels, Spatial& spatial_data,
                              vector<RegionObject>& region_objects)
#endif
 {
   int col, row;
#ifdef THREEDIM
   int slice;
#endif
   unsigned int region_object_label, region_object_index, pixel_index;
#ifdef DEBUG
   unsigned int region_objects_size = region_objects.size( );
   if (params.debug > 3)
   {
     params.log_fs << endl << "Entering update_region_object_info, dump of the connected region data:" << endl << endl;
     for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
       if (region_objects[region_object_index].get_active_flag( ))
         region_objects[region_object_index].print(region_objects);
   }
#endif

#ifdef THREEDIM
   for (slice = 0; slice < nslices; slice++)
   {
#endif
    for (row = 0; row < nrows; row++)
     for (col = 0; col < ncols; col++)
     {
#ifdef THREEDIM
       pixel_index = col + row*ncols + slice*nrows*ncols;
#else
       pixel_index = col + row*ncols;
#endif
       region_object_label = spatial_data.get_region_object_label(pixel_index);
       if (region_object_label != 0)
       {
         region_object_index = region_object_label - 1;
         if (spatial_data.get_boundary_map(pixel_index) == (nslevels+1))
           region_objects[region_object_index].increment_boundary_npix( );
       }  //  if (region_object_label != 0)
     }
#ifdef THREEDIM
   }
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After update_region_object_info, dump of the connected region data:" << endl << endl;
     for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
       if (region_objects[region_object_index].get_active_flag( ))
         region_objects[region_object_index].print(region_objects);
   }
#endif
   return;
 }
/*-----------------------------------------------------------
|
|  Routine Name: update_region_object_info (recursive version)
|
|       Purpose: Updates region_objects.boundary_npix values
|
|         Input: recur_level   (Lowest recursive level at which this task is active)
|                section       (Section or window processed by this call to do_region_relabel)
|                nslevels      (Number of segmentation levels saved)
|                spatial_data  (Class which holds information pertaining to the spatial data)
|
|        Output: region_objects   (Class which holds connected region related information)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: November 27, 2003
| Modifications: (see comments for first update_region_class_info function)
|
------------------------------------------------------------*/
 void update_region_object_info(const short unsigned int& recur_level,
                              const short unsigned int& section, const short unsigned int& nslevels,
                              Spatial& spatial_data, vector<RegionObject>& region_objects,
                              Temp& temp_data)
 {
   unsigned int region_object_index, nb_objects, max_region_object_label;

   nb_objects = max_region_object_label = 0;
   unsigned int region_objects_size = region_objects.size( );
   for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
     if (region_objects[region_object_index].get_active_flag( ))
     {
       ++nb_objects;
       if (region_objects[region_object_index].get_label( ) > max_region_object_label)
         max_region_object_label = region_objects[region_object_index].get_label( );
     }

   if (params.debug > 3)
     params.log_fs << "Entering update_region_object_info, max_region_object_label = " << max_region_object_label << endl;

   if (max_region_object_label > 0)
   {
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "Entering update_region_object_info, dump of the region data:" << endl << endl;
       for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
         if (region_objects[region_object_index].get_active_flag( ))
           region_objects[region_object_index].print(region_objects);
     }
#endif

     if (recur_level >= (params.onb_levels-1))
     {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of update_region_object_info can be called.
#ifdef THREEDIM
       update_region_object_info(params.pixel_ncols,params.pixel_nrows,params.pixel_nslices,
                               nslevels,spatial_data,region_objects);
#else
       update_region_object_info(params.pixel_ncols,params.pixel_nrows,
                               nslevels,spatial_data,region_objects);
#endif
     }
     else
     {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // update_region_object_info must be called recursively.
       int stride, nb_sections;
       set_stride_sections(recur_level,stride,nb_sections);
#ifdef PARALLEL
       unsigned int short_buf_size = 0, int_buf_size = 0, double_buf_size = 0;
       short_buf_size = 0;
       int_buf_size = 2*nb_objects;
       double_buf_size = params.nbands*nb_objects;
       if (params.region_sumsq_flag)
         double_buf_size += params.nbands*nb_objects;
       if (params.region_sumxlogx_flag)
         double_buf_size += params.nbands*nb_objects;
       if (params.region_std_dev_flag)
         double_buf_size += params.nbands*nb_objects;
       check_buf_size(0,short_buf_size,int_buf_size,0,double_buf_size,
                      temp_data);

       int_buf_size = double_buf_size = 0;
       for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
         if (region_objects[region_object_index].get_active_flag( ))
         {
           region_objects[region_object_index].load_data(temp_data,
                                                         int_buf_size,double_buf_size);
         }

     // Send region_objects to the parallel recur_tasks
#ifdef THREEDIM
       parallel_recur_requests((short unsigned int) 2, recur_level, 1, 0, (int) nslevels, 0, max_region_object_label,
                               int_buf_size, double_buf_size, temp_data);
#else
       parallel_recur_requests((short unsigned int) 2, recur_level, 1, 0, (int) nslevels, max_region_object_label,
                               int_buf_size, double_buf_size, temp_data);
#endif
     // Process current task.
       update_region_object_info((recur_level+1),section,nslevels,spatial_data,region_objects,temp_data);

       int sum_pixel_gdissim_tag = 102;
     // Receive information back from the individual tasks
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
#ifdef TIME_IT
         end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
         elapsed_time = end_time - temp_data.start_time;
         if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
         temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
      // Receive region_objects.boundary_npix
         MPI::COMM_WORLD.Recv(&int_buf_size, 1, MPI::UNSIGNED,
                              recur_section, sum_pixel_gdissim_tag);
#ifdef TIME_IT
         end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
         elapsed_time = end_time - temp_data.start_time;
         if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
         temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
         check_buf_size(0,0,int_buf_size,0,0,
                        temp_data);
         MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                              recur_section, sum_pixel_gdissim_tag);
#ifdef TIME_IT
         end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
         elapsed_time = end_time - temp_data.start_time;
         if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
         temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
         unsigned int int_buf_position = 0;
         for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
           if (region_objects[region_object_index].get_active_flag( ))
             region_objects[region_object_index].update_boundary_npix(temp_data,int_buf_position);
         if (params.debug > 3)
           params.log_fs << "Received region_object_info data from task " << recur_section << endl;
#else
       // Process current section.
         if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
         {
           spatial_data.restore_region_object_label_map(recur_section);
           spatial_data.restore_boundary_map(recur_section);
         }
         update_region_object_info((recur_level+1),recur_section,nslevels,spatial_data,region_objects,temp_data);
#endif
#ifdef DEBUG
         if (params.debug > 3)
         {
           params.log_fs << endl << "Dump of the region data:" << endl << endl;
           for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
             if (region_objects[region_object_index].get_active_flag( ))
               region_objects[region_object_index].print(region_objects);
         }
#endif
       } // for (int recur_section = min_section; recur_section < max_section; recur_section += stride)
     }
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "After update_region_object_info, dump of the region data:" << endl << endl;
       for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
         if (region_objects[region_object_index].get_active_flag( ))
           region_objects[region_object_index].print(region_objects);
     }
#endif
   }
   return;
 }
/*-----------------------------------------------------------
|
|  Routine Name: update_sum_pixel_gdissim (regular version)
|
|       Purpose: Updates region_classes.sum_pixel_gdissim values
|
|         Input: ncols         (Number of columns of spatial_data)
|                nrows         (Number of rows of spatial_data)
|                nslices       (Number of slices of spatial_data)
|                pixel_data    (Class which holds information pertaining to the pixel data)
|
|        Output: region_classes   (Class which holds region related information)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: November 27, 2003
| Modifications: (see comments for first update_region_class_info function)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void update_sum_pixel_gdissim(const int& ncols, const int& nrows,
                               const int& nslices, vector<Pixel>& pixel_data,
                               vector<RegionClass>& region_classes)
#else
 void update_sum_pixel_gdissim(const int& ncols, const int& nrows,
                               vector<Pixel>& pixel_data,
                               vector<RegionClass>& region_classes)
#endif
 {
   unsigned int pixel_index, region_class_label, region_index;
   int col, row;
#ifdef THREEDIM
   int slice;
#endif
#ifdef DEBUG
   unsigned int region_classes_size = region_classes.size( );
   if (params.debug > 3)
   {
     params.log_fs << endl << "Entering update_sum_pixel_gdissim, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag( ))
         region_classes[region_index].print();
   }
#endif

#ifdef THREEDIM
   for (slice = 0; slice < nslices; slice++)
   {
#endif
    for (row = 0; row < nrows; row++)
     for (col = 0; col < ncols; col++)
     {
#ifdef THREEDIM
       pixel_index = col + row*ncols + slice*nrows*ncols;
#else
       pixel_index = col + row*ncols;
#endif
       region_class_label = pixel_data[pixel_index].get_region_label( );
       if (region_class_label != 0)
       {
         region_index = region_class_label - 1;
         region_classes[region_index].update_sum_pixel_gdissim(&pixel_data[pixel_index]);
       }  //  if (region_class_label != 0)
     }
#ifdef THREEDIM
   }
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After update_sum_pixel_gdissim, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag( ))
         region_classes[region_index].print();
   }
#endif
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: update_sum_pixel_gdissim (recursive version)
|
|       Purpose: Updates region_classes.sum_pixel_gdissim values
|
|         Input: recur_level   (Lowest recursive level at which this task is active)
|                section       (Section or window processed by this call to do_region_relabel)
|                pixel_data    (Class which holds information pertaining to the pixel data)
|
|        Output: region_classes   (Class which holds region related information)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: November 27, 2003
| Modifications: (see comments for first update_sum_pixel_gdissim function)
|
------------------------------------------------------------*/
 void update_sum_pixel_gdissim(const short unsigned int& recur_level, const short unsigned int& section,
                               vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                               Temp& temp_data)
 {
   unsigned int region_index, nregions, max_region_class_label;
   unsigned int region_classes_size = region_classes.size( );

   nregions = max_region_class_label = 0;
   for (region_index = 0; region_index < region_classes_size; ++region_index)
     if (region_classes[region_index].get_active_flag( ))
     {
       ++nregions;
       if (region_classes[region_index].get_label( ) > max_region_class_label)
         max_region_class_label = region_classes[region_index].get_label( );
     }

   if (params.debug > 3)
     params.log_fs << "Entering update_sum_pixel_gdissim, max_region_class_label = " << max_region_class_label << endl;

   if (max_region_class_label > 0)
   {
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "Entering update_sum_pixel_gdissim, dump of sum_pixel_gdissim:" << endl << endl;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         if (region_classes[region_index].get_active_flag( ))
         {
           params.log_fs << "For region " << region_classes[region_index].get_label( );
           params.log_fs << " sum_pixel_gdissim = " << region_classes[region_index].get_sum_pixel_gdissim( ) << endl;
         }   
     }
#endif

     if (recur_level >= (params.onb_levels-1))
     {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of update_sum_pixel_gdissim can be called.
#ifdef THREEDIM
       update_sum_pixel_gdissim(params.pixel_ncols,params.pixel_nrows,params.pixel_nslices,
                                pixel_data,region_classes);
#else
       update_sum_pixel_gdissim(params.pixel_ncols,params.pixel_nrows,
                                pixel_data,region_classes);
#endif
     }
     else
     {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // update_sum_pixel_gdissim must be called recursively.
       int stride, nb_sections;
       set_stride_sections(recur_level,stride,nb_sections);
#ifdef PARALLEL
       unsigned int short_buf_size = 0, int_buf_size = 0, double_buf_size = 0;
       short_buf_size = 0;
       int_buf_size = 2*nregions;
       double_buf_size = params.nbands*nregions;
       if (params.region_sumsq_flag)
         double_buf_size += params.nbands*nregions;
       if (params.region_sumxlogx_flag)
         double_buf_size += params.nbands*nregions;
       if (params.region_std_dev_flag)
         double_buf_size += params.nbands*nregions;
       check_buf_size(0,short_buf_size,int_buf_size,0,double_buf_size,
                      temp_data);

       short_buf_size = int_buf_size = double_buf_size = 0;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         if (region_classes[region_index].get_active_flag( ))
         {
           temp_data.int_buffer[int_buf_size++] = region_classes[region_index].get_label( );
           region_classes[region_index].load_data(temp_data,int_buf_size,double_buf_size);
         }

     // Send region_classes to the parallel recur_tasks
#ifdef THREEDIM
       parallel_recur_requests((short unsigned int) 2, recur_level, 3, 0, 0, 0, max_region_class_label,
                               int_buf_size, double_buf_size, temp_data);
#else
       parallel_recur_requests((short unsigned int) 2, recur_level, 3, 0, 0, max_region_class_label,
                               int_buf_size, double_buf_size, temp_data);
#endif
     // Process current task's data section.
       update_sum_pixel_gdissim((recur_level+1),section,pixel_data,region_classes,temp_data);

       int sum_pixel_gdissim_tag = 102;
     // Receive information back from the individual parallel tasks
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
      // Receive region_classes.sum_pixel_gdissim
         short_buf_size = int_buf_size = 0;
#ifdef TIME_IT
         end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
         elapsed_time = end_time - temp_data.start_time;
         if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
         temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
         MPI::COMM_WORLD.Recv(&double_buf_size, 1, MPI::UNSIGNED,
                              recur_section, sum_pixel_gdissim_tag);
#ifdef TIME_IT
         end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
         elapsed_time = end_time - temp_data.start_time;
         if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
         temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
         check_buf_size(0,short_buf_size,int_buf_size,0,double_buf_size,
                        temp_data);
         MPI::COMM_WORLD.Recv(temp_data.double_buffer, double_buf_size, MPI::DOUBLE,
                              recur_section, sum_pixel_gdissim_tag);
#ifdef TIME_IT
         end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
         elapsed_time = end_time - temp_data.start_time;
         if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
         temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
         unsigned int double_buf_position = 0;
         for (region_index = 0; region_index < region_classes_size; ++region_index)
           if (region_classes[region_index].get_active_flag( ))
             region_classes[region_index].update_sum_pixel_gdissim(temp_data.double_buffer[double_buf_position++]);
         if (params.debug > 3)
           params.log_fs << "Received sum_pixel_gdissim data from task " << recur_section << endl;
#else  // !PARALLEL
       // In the serial case, process the specified data section
         if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
           restore_pixel_data(recur_section,pixel_data,temp_data);
         update_sum_pixel_gdissim((recur_level+1),recur_section,pixel_data,region_classes,temp_data);
#endif // !PARALLEL
#ifdef DEBUG
         if (params.debug > 3)
         {
           params.log_fs << endl << "Dump of the region data:" << endl << endl;
           for (region_index = 0; region_index < region_classes_size; ++region_index)
             if (region_classes[region_index].get_active_flag( ))
               region_classes[region_index].print();
         }
#endif
       } // for (int recur_section = min_section; recur_section < max_section; recur_section += stride)
     }
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "After update_sum_pixel_gdissim, dump of sum_pixel_gdissim:" << endl << endl;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         if (region_classes[region_index].get_active_flag( ))
         {
           params.log_fs << "For region " << region_classes[region_index].get_label( );
           params.log_fs << " sum_pixel_gdissim = " << region_classes[region_index].get_sum_pixel_gdissim( ) << endl;
         }   
     }
#endif
   }
   return;
 }

} // namespace HSEGTilton
