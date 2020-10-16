/*-----------------------------------------------------------
|
|  Routine Name: nghbrs_label_set_init (regular version)
|
|       Purpose: Initialize region_classes.nghbrs_label_set from current pixel_data values
|
|         Input: ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|
|        Output: region_classes   (Class which holds region class related information)
|
|         Other:
|
|       Returns: (void)
|
|    Written By: James C. Tilton
|          Date: October 6, 2004
| Modifications: December 23, 2004 - Changed region label from short unsigned int to unsigned int
|                December 16, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|		 April 16, 2013 - Renamed nghbrs_info_map_init (from nghbrs_label_set_init) and modified to utilize
|				  the nghbrs_info_map instead of the nghbrs_label_set
|                August 1, 2013 - Revised to accommodate standard deviation and region edge information.
|                January 8, 2014 - Renamed back to nghbrs_label_set_init and revised to utilize the nghbrs_label_set.
|
------------------------------------------------------------*/

#include "region_class.h"
#include <params/params.h>
#include <pixel/pixel.h>
#include <index/index.h>
#ifdef TIME_IT
#include <ctime>
#endif

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef THREEDIM
 void nghbrs_label_set_init(const int& ncols, const int& nrows,
                            const int& nslices, vector<Pixel>& pixel_data,
                            vector<RegionClass>& region_classes)
#else
 void nghbrs_label_set_init(const int& ncols, const int& nrows,
                            vector<Pixel>& pixel_data, vector<RegionClass>& region_classes)
#endif
 {
   unsigned int region_index, region_label;
   unsigned int pixel_index, region_classes_size = region_classes.size();
   int col, row;
#ifdef THREEDIM
   int slice;
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << "nghbrs_label_set_init called with ncols = " << ncols;
#ifdef THREEDIM
     params.log_fs << ", nrows = " << nrows << ", and nslices = " << nslices << endl;
#else
     params.log_fs << ", and nrows = " << nrows << endl;
#endif
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
       region_label = pixel_data[pixel_index].get_region_label();
       if (region_label != 0)
       {
         if (region_label > region_classes_size)
         {
           region_classes_size = region_label;
           region_classes.resize(region_classes_size);
         }
         region_index = region_label - 1;
         region_classes[region_index].set_active_flag(true);
#ifdef THREEDIM
         region_classes[region_index].nghbrs_label_set_init(pixel_data,col,row,slice,ncols,nrows,nslices);
#else
         region_classes[region_index].nghbrs_label_set_init(pixel_data,col,row,ncols,nrows);
#endif
       }
     }
#ifdef THREEDIM
   }
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting nghbrs_label_set_init(S), dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag())
         region_classes[region_index].print(region_classes);
   }
#endif
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: nghbrs_label_set_init (recursive version)
|
|       Purpose: Initialize region_classes.nghbrs_label_set from current pixel_data values
|
|         Input: recur_level      (Lowest recursive level at which this task is active)
|                section          (Section or window processed by this call to get_seam_index_data)
|                ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|
|        Output: region_classes      (Class which holds region related information)
|
|  Input/Output: max_region_label (Maximum region label in region_classes)
|
|         Other: temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton
|          Date: October 6, 2004
| Modifications: December 23, 2004 - Changed region label from short unsigned int to unsigned int
|                December 16, 2005 - Added slice dimension (extension to three-dimensional analysis)
|		 April 16, 2013 - Renamed nghbrs_info_map_init (from nghbrs_label_set_init) and modified to utilize
|				  the nghbrs_info_map instead of the nghbrs_label_set
|                August 1, 2013 - Revised to accommodate standard deviation and region edge information.
|                January 8, 2014 - Renamed back to nghbrs_label_set_init and revised to utilize the nghbrs_label_set.
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void nghbrs_label_set_init(const short unsigned int& recur_level, const short unsigned int& section,
                           unsigned int max_region_label, const int& ncols, const int& nrows, const int& nslices,
                           vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                           Temp& temp_data)
#else
 void nghbrs_label_set_init(const short unsigned int& recur_level, const short unsigned int& section,
                           unsigned int max_region_label, const int& ncols, const int& nrows,
                           vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                           Temp& temp_data)
#endif
 {
#ifdef DEBUG
   unsigned int region_index, region_classes_size = region_classes.size();
   if (params.debug > 3)
   {
     params.log_fs << "nghbrs_label_set_init called at recur_level = " << recur_level;
     params.log_fs << " with max_region_label = " << max_region_label << ", ncols = " << ncols;
#ifdef THREEDIM
     params.log_fs << ", nrows = " << nrows << ", and nslices = " << nslices << endl;
#else
     params.log_fs << ", and nrows = " << nrows << endl;
#endif
   }
#endif

   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of nghbrs_label_set_init can be called.
#ifdef THREEDIM
     nghbrs_label_set_init(ncols,nrows,nslices,pixel_data,region_classes);
#else
     nghbrs_label_set_init(ncols,nrows,pixel_data,region_classes);
#endif
   }
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // nghbrs_label_set_init must be called recursively.
     int recur_ncols = ncols;
     int recur_nrows = nrows;
     bool col_flag, row_flag;
#ifdef THREEDIM
     int recur_nslices = nslices;
     bool slice_flag;
     set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag,slice_flag);
#else
     set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag);
#endif
     if (col_flag)
       recur_ncols /= 2;
     if (row_flag)
       recur_nrows /= 2;
#ifdef THREEDIM
     if (slice_flag)
       recur_nslices /= 2;
#endif
     int stride, nb_sections;
     set_stride_sections(recur_level,stride,nb_sections);
#ifdef PARALLEL
#ifdef THREEDIM
   // Send request to the parallel recur_tasks
     parallel_recur_requests((short unsigned int) 22,recur_level,max_region_label,ncols,nrows,nslices,0,0,0,temp_data);
   // Process current section's data
     nghbrs_label_set_init((recur_level+1),section,max_region_label,
                          recur_ncols,recur_nrows,recur_nslices,pixel_data,region_classes,temp_data);
#else
   // Send request to the parallel recur_tasks
     parallel_recur_requests((short unsigned int) 22,recur_level,max_region_label,ncols,nrows,0,0,0,temp_data);
   // Process current section's data
     nghbrs_label_set_init((recur_level+1),section,max_region_label,
                          recur_ncols,recur_nrows,pixel_data,region_classes,temp_data);
#endif
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "After recursive call to nghbrs_label_set_init, dump of the region data:" << endl << endl;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         if (region_classes[region_index].get_active_flag())
           region_classes[region_index].print(region_classes);
     }
#else
     unsigned int region_index, region_classes_size = region_classes.size();
#endif
     unsigned int region_label;
     unsigned int int_buf_size = 0;
     unsigned int int_buf_position;
     int nghbrs_label_set_init_tag = 122;
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
   // Receive region_classes information from the parallel recur_sections
#ifdef TIME_IT
       end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
       temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
       MPI::COMM_WORLD.Recv(&max_region_label, 1, MPI::UNSIGNED, recur_section, nghbrs_label_set_init_tag);
#ifdef TIME_IT
       end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
       temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
#ifdef DEBUG
       if (params.debug > 3)
       {
         params.log_fs << "Received nghbrs_label_set_init results from task " << recur_section;
         params.log_fs << " with max_region_label = " << max_region_label << endl;
       }
#endif
       MPI::COMM_WORLD.Recv(&int_buf_size, 1, MPI::UNSIGNED, recur_section, nghbrs_label_set_init_tag);

       check_buf_size(0,0,int_buf_size,0,0,temp_data);
       if (int_buf_size > 0)
         MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, recur_section, nghbrs_label_set_init_tag);

       if (int_buf_size > 0)
       {
#ifdef TIME_IT
         end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
         elapsed_time = end_time - temp_data.start_time;
         if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
         temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
         int_buf_position = 0;
         if (max_region_label > region_classes_size)
           region_classes.resize(max_region_label,RegionClass());
         for (region_index = 0; region_index < max_region_label; ++region_index)
         {
           region_label = temp_data.int_buffer[int_buf_position];
           if (region_label == region_classes[region_index].get_label())
           {
             int_buf_position++;
             region_classes[region_index].set_active_flag(true);
             region_classes[region_index].update_nghbrs_label_set(temp_data,int_buf_position);
           }
         }
       }
#else // !PARALLEL
     // In the serial case, process the specified data section
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
         restore_pixel_data(recur_section,pixel_data,temp_data);
#ifdef THREEDIM
       nghbrs_label_set_init((recur_level+1),recur_section,max_region_label,
                             recur_ncols,recur_nrows,recur_nslices,pixel_data,region_classes,temp_data);
#else
       nghbrs_label_set_init((recur_level+1),recur_section,max_region_label,
                             recur_ncols,recur_nrows,pixel_data,region_classes,temp_data);
#endif
#endif // !PARALLEL
     } // for (recur_section = min_section; recur_section < max_section; recur_section += stride)

    // Update the nghbrs_label_set along the processing window seams.
     unsigned int index_data_size = 1;
     if (col_flag)
#ifdef THREEDIM
       index_data_size = nrows*nslices*params.seam_size;
#else
       index_data_size = nrows*params.seam_size;
#endif
     vector<Index> col_seam_index_data(index_data_size);
     index_data_size = 1;
     if (row_flag)
#ifdef THREEDIM
       index_data_size = ncols*nslices*params.seam_size;
#else
       index_data_size = ncols*params.seam_size;
#endif
     vector<Index> row_seam_index_data(index_data_size);
#ifdef THREEDIM
     index_data_size = 1;
     if (slice_flag)
       index_data_size = ncols*nrows*params.seam_size;
     vector<Index> slice_seam_index_data(index_data_size);

     get_seam_index_data(recur_level,section,params.recur_mask_flags[recur_level],pixel_data,ncols,nrows,nslices,col_seam_index_data,
                         row_seam_index_data,slice_seam_index_data,temp_data);
     update_nghbrs_label_set(recur_level,ncols,nrows,nslices,col_seam_index_data,
                             row_seam_index_data,slice_seam_index_data,region_classes);
#else
     get_seam_index_data(recur_level,section,params.recur_mask_flags[recur_level],pixel_data,ncols,nrows,col_seam_index_data,
                         row_seam_index_data,temp_data);
     update_nghbrs_label_set(recur_level,ncols,nrows,col_seam_index_data,
                             row_seam_index_data,region_classes);
#endif
   }
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting nghbrs_label_set_init(P), dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag())
         region_classes[region_index].print(region_classes);
   }
#endif
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: update_nghbrs_label_set
|
|       Purpose: Update the nghbrs_label_set for each region to include neighborhood
|                information from across the processing window boundary
|
|         Input: ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                col_seam_index_data  (RegionClass labels from the processing window column seam)
|                row_seam_index_data  (RegionClass labels from the processing window row seam)
|                slice_seam_index_data(RegionClass labels from the processing window slice seam)
|                
|        Output: region_classes      (Class which holds region related information)
|
|         Other:
|
|       Returns: (void)
|
|    Written By: James C. Tilton
|          Date: October 6, 2003
| Modifications: December 23, 2004 - Changed region label from short unsigned int to unsigned int
|                November 1, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                April 18, 2013 - Renamed update_nghbrs_info_map (from update_nghbrs_label_set) and modified to utilize
|                                 the nghbrs_info_map instead of the nghbrs_label_set
|                January 8, 2014 - Renamed back to update_nghbrs_label_set and modified to utilize the nghbrs_label_set
|
------------------------------------------------------------*/

#ifdef THREEDIM
 void update_nghbrs_label_set(const short unsigned int& recur_level, const int& ncols, const int& nrows, const int& nslices,
                              vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                              vector<Index>& slice_seam_index_data, vector<RegionClass>& region_classes)
#else
 void update_nghbrs_label_set(const short unsigned int& recur_level, const int& ncols, const int& nrows,
                              vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                              vector<RegionClass>& region_classes)
#endif
 {
   unsigned int region_label, region_index;
   unsigned int seam_index;
   int col, row;
   bool col_flag, row_flag;
#ifdef THREEDIM
   int slice;
   bool slice_flag;
   set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag,slice_flag);
#else
   set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag);
#endif
   unsigned int region_classes_size = region_classes.size( );
   if (params.initial_merge_flag)
   {
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       region_classes[region_index].set_seam_flag(false);
   }
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Entering update_nghbrs_label_set, ncols = " << ncols;
#ifdef THREEDIM
     params.log_fs << ", nrows = " << nrows << " and nslices = " << nslices << endl << endl;
#else
     params.log_fs << " and nrows = " << nrows << endl << endl;
#endif
   }
   if (params.debug > 3)
   {
     if (col_flag)
     {
       params.log_fs << endl << "Entering update_nghbrs_label_set, col_seam_index_data:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
        for (row = 0; row < nrows; row++)
        {
         for (col = 0; col < params.seam_size; col++)
         {
#ifdef THREEDIM
            region_label = col_seam_index_data[row + slice*nrows + col*nslices*nrows].get_region_class_label( );
#else
            region_label = col_seam_index_data[row + col*nrows].get_region_class_label( );
#endif
            if (region_label < 10)
              params.log_fs << "    " << region_label << " ";
            else if (region_label < 100)
              params.log_fs << "   " << region_label << " ";
            else if (region_label < 1000)
              params.log_fs << "  " << region_label << " ";
            else if (region_label < 10000)
              params.log_fs << " " << region_label << " ";
            else
              params.log_fs << region_label << " ";
         }
         params.log_fs << endl;
        }
        params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (col_flag)
     if (row_flag)
     {
       params.log_fs << endl << "Entering update_nghbrs_label_set, row_seam_index_data:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
        for (row = 0; row < params.seam_size; row++)
        {
         for (col = 0; col < ncols; col++)
         {
#ifdef THREEDIM
            region_label = row_seam_index_data[col + slice*ncols + row*nslices*ncols].get_region_class_label( );
#else
            region_label = row_seam_index_data[col + row*ncols].get_region_class_label( );
#endif
            if (region_label < 10)
              params.log_fs << "    " << region_label << " ";
            else if (region_label < 100)
              params.log_fs << "   " << region_label << " ";
            else if (region_label < 1000)
              params.log_fs << "  " << region_label << " ";
            else if (region_label < 10000)
              params.log_fs << " " << region_label << " ";
            else
              params.log_fs << region_label << " ";
         }
         params.log_fs << endl;
        }
        params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (row_flag)
#ifdef THREEDIM
     if (slice_flag)
     {
       params.log_fs << endl << "Entering update_nghbrs_label_set, slice_seam_index_data:" << endl << endl;
       for (slice = 0; slice < params.seam_size; slice++)
       {
        for (row = 0; row < nrows; row++)
        {
         for (col = 0; col < ncols; col++)
         {
            region_label = slice_seam_index_data[col + row*ncols + slice*nrows*ncols].get_region_class_label( );
            if (region_label < 10)
              params.log_fs << "    " << region_label << " ";
            else if (region_label < 100)
              params.log_fs << "   " << region_label << " ";
            else if (region_label < 1000)
              params.log_fs << "  " << region_label << " ";
            else if (region_label < 10000)
              params.log_fs << " " << region_label << " ";
            else
              params.log_fs << region_label << " ";
         }
         params.log_fs << endl;
        }
        params.log_fs << endl;
       }
     } // if (slice_flag)
#endif
   }
#endif

   if (col_flag)
   {
#ifdef THREEDIM
     for (slice = 0; slice < nslices; slice++)
     {
#endif
      for (row = 0; row < nrows; row++)
       for (col = 0; col < params.seam_size; col++)
       {
#ifdef THREEDIM
         seam_index = row + slice*nrows + col*nslices*nrows;
#else
         seam_index = row + col*nrows;
#endif
         region_label = col_seam_index_data[seam_index].get_region_class_label( );
         if (region_label != 0)
         {
           region_index = region_label - 1;
           if (params.initial_merge_flag)
             region_classes[region_index].set_seam_flag(true);
#ifdef THREEDIM
           region_classes[region_index].col_init(col_seam_index_data,
                                                 col,row,slice,nrows,nslices);
#else
           region_classes[region_index].col_init(col_seam_index_data,
                                                 col,row,nrows);
#endif
         }
       }
#ifdef THREEDIM
     }
#endif
   } // if (col_flag)
   if (row_flag)
   {
    // Set values for row_seam_index_data
#ifdef THREEDIM
     for (slice = 0; slice < nslices; slice++)
     {
#endif
      for (row = 0; row < params.seam_size; row++)
       for (col = 0; col < ncols; col++)
       {
#ifdef THREEDIM
         seam_index = col + slice*ncols + row*nslices*ncols;
#else
         seam_index = col + row*ncols;
#endif
         region_label = row_seam_index_data[seam_index].get_region_class_label( );
         if (region_label != 0)
         {
           region_index = region_label - 1;
           if (params.initial_merge_flag)
             region_classes[region_index].set_seam_flag(true);
#ifdef THREEDIM
           region_classes[region_index].row_init(row_seam_index_data,
                                                 col,row,slice,ncols,nslices);
#else
           region_classes[region_index].row_init(row_seam_index_data,
                                                 col,row,ncols);
#endif
         }
       }
#ifdef THREEDIM
     }
#endif
   } // if (row_flag)
#ifdef THREEDIM
   if (slice_flag)
   {
    // Set values for slice_seam_index_data
     for (slice = 0; slice < params.seam_size; slice++)
      for (row = 0; row < nrows; row++)
       for (col = 0; col < ncols; col++)
       {
         seam_index = col + row*ncols + slice*nrows*ncols;
         region_label = slice_seam_index_data[seam_index].get_region_class_label( );
         if (region_label != 0)
         {
           region_index = region_label - 1;
           if (params.initial_merge_flag)
             region_classes[region_index].set_seam_flag(true);
           region_classes[region_index].slice_init(slice_seam_index_data,
                                                   col,row,slice,ncols,nrows);
         }
       }
   } // if (slice_flag)
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting update_nghbrs_label_set, dump of the region data:" << endl << endl;
     region_classes_size = region_classes.size( );
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag( ))
         region_classes[region_index].print(region_classes);
   }
   if (params.debug > 2)
   {
     params.log_fs << endl << "Exiting update_nghbrs_label_set" << endl << endl;
   }
#endif
   return;
 }
} // namespace HSEGTilton
