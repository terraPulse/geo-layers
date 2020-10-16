/*-----------------------------------------------------------
|
|  Routine Name: do_region_class_relabel (regular version)
|
|       Purpose: Performs the region relabeling operation on pixel_data
|                as specified by the region_class_relabel_pairs map
|
|         Input: region_class_relabel_pairs (Map specifying the region relabel table)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                
|        Output:
|
|         Other:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: September 13, 2002.
| Modifications: February 10, 2003:  Changed region index to region label
|                June 11, 2003 - Modified the temp_data structure and its use.
|                November 17, 2003 - Added region_object version.
|                December 22, 2004 - Changed region label from short unsigned int to unsigned int
|                May 31, 2005 - Added temporary file I/O for faster processing of large data sets
|                October 21, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|
------------------------------------------------------------*/

#include "pixel.h"
#include <params/params.h>
#include <spatial/spatial.h>
#include <index/index.h>
#include <iostream>
#ifdef TIME_IT
#include <ctime>
#endif

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 void do_region_class_relabel(map<unsigned int,unsigned int>& region_class_relabel_pairs,
                        vector<Pixel>& pixel_data)
 {
   unsigned int region_label, pixel_index, pixel_data_size;
   map<unsigned int,unsigned int>::iterator region_class_relabel_pair_iter;
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Entering do_region_label, dump of the pixel data:" << endl << endl;
     pixel_data_size = pixel_data.size( );
     for (pixel_index = 0; pixel_index < pixel_data_size; pixel_index++)
     {
       region_label = pixel_data[pixel_index].get_region_label( );
       if (region_label != 0)
         params.log_fs << "Element " << pixel_index << " is associated with region label " << region_label << endl;
//     pixel_data[pixel_index].print(pixel_index);
     }
   }
   if (params.debug > 2)
   {
     params.log_fs << endl << "The region relabelings performed in do_region_class_relabel:" << endl;
     region_class_relabel_pair_iter = region_class_relabel_pairs.begin( );
     while (region_class_relabel_pair_iter != region_class_relabel_pairs.end( ))
     {
       if ((*region_class_relabel_pair_iter).first != (*region_class_relabel_pair_iter).second)
       {
         params.log_fs << "Region label " << (*region_class_relabel_pair_iter).first << " relabeled to ";
         params.log_fs << (*region_class_relabel_pair_iter).second << endl;
       }
       ++region_class_relabel_pair_iter;
     }
   }
#endif
   pixel_data_size = pixel_data.size( );
   for (pixel_index = 0; pixel_index < pixel_data_size; pixel_index++)
   {
     region_label = pixel_data[pixel_index].get_region_label( );
     if (region_label != 0)
     {
       region_class_relabel_pair_iter = region_class_relabel_pairs.find(region_label);
       if (region_class_relabel_pair_iter != region_class_relabel_pairs.end( ))
       {
         region_label = (*region_class_relabel_pair_iter).second;
         if (region_label == 0)
         {
           if (params.debug > 0)
           {
             params.log_fs << "WARNING:  Region label " << pixel_data[pixel_index].get_region_label( );
             params.log_fs << "relabeled to 0 (unknown label)." << endl;
           }
           else
           {
             cout << "WARNING:  Region label " << pixel_data[pixel_index].get_region_label( );
             cout << "relabeled to 0 (unknown label)." << endl;
           }
         }
         pixel_data[pixel_index].set_region_label(region_label);
       }
     }
   }
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting do_region_class_relabel, dump of the pixel data:" << endl << endl;
     pixel_data_size = pixel_data.size( );
     for (pixel_index = 0; pixel_index < pixel_data_size; pixel_index++)
     {
       region_label = pixel_data[pixel_index].get_region_label( );
       if (region_label != 0)
       {
         params.log_fs << "Element " << pixel_index << " is associated with region label " << region_label << endl;
//       pixel_data[pixel_index].print(pixel_index);
       }
     }
   }
#endif
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: do_region_class_relabel (recursive version)
|
|       Purpose: Performs the region relabeling operation on pixel_data
|                as specified by the region_class_relabel_pairs map
|
|         Input: recur_level      (Lowest recursive level at which this task is active)
|                section          (Section or window processed by this call to do_region_class_relabel)
|                region_class_relabel_pairs (Map specifying the region relabel table)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                
|        Output:
|
|         Other: temp_data    (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: September 13, 2002.
| Modifications: (see comments for first do_region_class_relabel function)
|
------------------------------------------------------------*/
 void do_region_class_relabel(const short unsigned int& recur_level, const short unsigned int& section,
                        map<unsigned int,unsigned int>& region_class_relabel_pairs,
                        vector<Pixel>& pixel_data,
                        Temp& temp_data)
 {
   if (params.debug > 3)
   {
     params.log_fs << "do_region_class_relabel called at recur_level = " << recur_level << endl;
   }

   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of do_region_class_relabel can be called.
     do_region_class_relabel(region_class_relabel_pairs,pixel_data);
   }
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // do_region_class_relabel must be called recursively.
     int stride, nb_sections;
     set_stride_sections(recur_level,stride,nb_sections);
#ifdef PARALLEL
     unsigned int int_buf_size = 2*region_class_relabel_pairs.size( );
     check_buf_size(0, 0, int_buf_size, 0, 0, temp_data);
     unsigned int int_buf_position = 0;
     map<unsigned int,unsigned int>::iterator region_class_relabel_pair_iter;
     region_class_relabel_pair_iter = region_class_relabel_pairs.begin( );
     while (region_class_relabel_pair_iter != region_class_relabel_pairs.end( ))
     {
       temp_data.int_buffer[int_buf_position++] = (*region_class_relabel_pair_iter).first;
       temp_data.int_buffer[int_buf_position++] = (*region_class_relabel_pair_iter).second;
       ++region_class_relabel_pair_iter;
     }
   // Send request to the parallel recur_tasks
#ifdef THREEDIM
     parallel_recur_requests((short unsigned int) 4, recur_level, 0, 0, 0, 0, 0, int_buf_size, 0, temp_data);
#else
     parallel_recur_requests((short unsigned int) 4, recur_level, 0, 0, 0, 0, int_buf_size, 0, temp_data);
#endif
   // Process current task's data section
     do_region_class_relabel((recur_level+1),section,region_class_relabel_pairs,pixel_data,temp_data);
   // Receive completion confirmation from the parallel recur_tasks
     int region_class_relabel_tag = 104;
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
       if (params.debug > 3)
         params.log_fs << "Waiting for confirmation of completion from task " << recur_section << endl;
#ifdef TIME_IT
       end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
       temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
       MPI::COMM_WORLD.Recv(&int_buf_size, 1, MPI::UNSIGNED, recur_section, region_class_relabel_tag);
#ifdef TIME_IT
       end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
       temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
#else // !PARALLEL
     // In the serial case, process the specified data section
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
         restore_pixel_data(recur_section,pixel_data,temp_data);
       do_region_class_relabel((recur_level+1),recur_section,
                         region_class_relabel_pairs,pixel_data,temp_data);
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
         save_pixel_data(recur_section,pixel_data,temp_data);
#endif // !PARALLEL
     }
   }

   if (params.debug > 3)
     params.log_fs << "Exited do_region_class_relabel " << endl;
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: do_region_class_relabel (for seam_index_data)
|
|       Purpose: Performs the region relabeling operation on seam_index_data
|                as specified by the region_class_relabel_pairs map
|
|         Input: region_class_relabel_pairs (Map specifying the region relabel table)
|                seam_index_data       (Class which holds information pertaining to the pixels along the processing window seam)
|                
|        Output:
|
|         Other:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: January 28, 2014
| Modifications: 
|
------------------------------------------------------------*/
#ifdef THREEDIM
  void do_region_class_relabel(map<unsigned int,unsigned int>& region_class_relabel_pairs,
                               const int& ncols, const int& nrows, const int& nslices, 
                               vector<Index>& seam_index_data)
#else
  void do_region_class_relabel(map<unsigned int,unsigned int>& region_class_relabel_pairs,
                               const int& ncols, const int& nrows,
                               vector<Index>& seam_index_data)

#endif
  {
    unsigned int region_label, seam_index;
    int col, row;
#ifdef THREEDIM
    int slice;
#endif
    map<unsigned int,unsigned int>::iterator region_class_relabel_pair_iter;

#ifdef THREEDIM
    for (slice = 0; slice < nslices; slice++)
    {
#endif
      for (row = 0; row < nrows; row++)
      {
        for (col = 0; col < ncols; col++)
        {
#ifdef THREEDIM
          seam_index = col + row*ncols + slice*nrows*ncols;
#else
          seam_index = col + row*ncols;
#endif
          region_label = seam_index_data[seam_index].get_region_class_label( );
          if (region_label != 0)
          {
            region_class_relabel_pair_iter = region_class_relabel_pairs.find(region_label);
            if (region_class_relabel_pair_iter != region_class_relabel_pairs.end( ))
            {
              region_label = (*region_class_relabel_pair_iter).second;
              if (region_label == 0)
              {
                if (params.debug > 0)
                {
                  params.log_fs << "WARNING:  Region label " << seam_index_data[seam_index].get_region_class_label( );
                  params.log_fs << "relabeled to 0 (unknown label)." << endl;
                }
                else
                {
                  cout << "WARNING:  Region label " << seam_index_data[seam_index].get_region_class_label( );
                  cout << "relabeled to 0 (unknown label)." << endl;
                }
              }
              seam_index_data[seam_index].set_region_class_label(region_label);
            }
          }
        }
      }
#ifdef THREEDIM
    }
#endif

    return;
  }

/*-----------------------------------------------------------
|
|  Routine Name: do_region_object_relabel (regular version)
|
|       Purpose: Performs the region relabeling operation on spatial_data
|                as specified by the region_object_relabel_pairs map
|
|         Input: region_object_relabel_pairs (Map specifying the conneted region relabel table)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                ncols            (Number of columns in spatial_data)
|                nrows            (Number of rows in spatial_data)
|                nslices          (Number of slices in spatial_data)
|                
|        Output:
|
|         Other:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: September 13, 2002.
| Modifications: (see comments for first do_region_class_relabel function)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void do_region_object_relabel(map<unsigned int,unsigned int>& region_object_relabel_pairs,
                             Spatial& spatial_data, const int& ncols, const int& nrows, const int& nslices)
#else
 void do_region_object_relabel(map<unsigned int,unsigned int>& region_object_relabel_pairs,
                             Spatial& spatial_data, const int& ncols, const int& nrows)
#endif
 {
   int col, row;
#ifdef THREEDIM
   int slice;
#endif
   unsigned int pixel_index, region_object_label;
   map<unsigned int,unsigned int>::iterator region_object_relabel_pair_iter;
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Entering region_object_relabel, dump of region_object_label_map:" << endl << endl;
#ifdef THREEDIM
     for (slice = 0; slice < nslices; slice++)
     {

#endif
      for (row = 0; row < nrows; row++)
      {
       for (col = 0; col < ncols; col++)
       {
#ifdef THREEDIM
         pixel_index = col + row*ncols + slice*nrows*ncols;
#else
         pixel_index = col + row*ncols;
#endif
         params.log_fs << spatial_data.get_region_object_label(pixel_index) << " ";
       }
       params.log_fs << endl;
      }
      params.log_fs << endl;
#ifdef THREEDIM
     }
     params.log_fs << endl;
#endif
   }

   if (params.debug > 3)
   {
     params.log_fs << endl << "The region relabelings performed in region_object_relabel:" << endl;
     region_object_relabel_pair_iter = region_object_relabel_pairs.begin( );
     while (region_object_relabel_pair_iter != region_object_relabel_pairs.end( ))
     {
       if ((*region_object_relabel_pair_iter).first != (*region_object_relabel_pair_iter).second)
       {
         params.log_fs << "Subregion label " << (*region_object_relabel_pair_iter).first << " relabeled to ";
         params.log_fs << (*region_object_relabel_pair_iter).second << endl;
       }
       ++region_object_relabel_pair_iter;
     }
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
         region_object_relabel_pair_iter = region_object_relabel_pairs.find(region_object_label);
         if (region_object_relabel_pair_iter != region_object_relabel_pairs.end( ))
         {
           region_object_label = (*region_object_relabel_pair_iter).second;
           if (region_object_label == 0)
           {
             if (params.debug > 0)
             {
               params.log_fs << "WARNING:  Connected region label " << spatial_data.get_region_object_label(pixel_index) << "relabeled to 0 (unknown label)." << endl;
             }
             else
             {
               cout << "WARNING:  Subregion label " << spatial_data.get_region_object_label(pixel_index) << "relabeled to 0 (unknown label)." << endl;
             }
           }
           spatial_data.set_region_object_label(region_object_label,pixel_index);
         }
       }
     }
#ifdef THREEDIM
   }
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting region_object_relabel, dump of region_object_label_map:" << endl << endl;
#ifdef THREEDIM
     for (slice = 0; slice < nslices; slice++)
     {
#endif
      for (row = 0; row < nrows; row++)
      {
       for (col = 0; col < ncols; col++)
       {
#ifdef THREEDIM
         pixel_index = col + row*ncols + slice*nrows*ncols;
#else
         pixel_index = col + row*ncols;
#endif
         params.log_fs << spatial_data.get_region_object_label(pixel_index) << " ";
       }
       params.log_fs << endl;
      }
      params.log_fs << endl;
#ifdef THREEDIM
     }
     params.log_fs << endl;
#endif
   }
#endif
   return;
 }
/*-----------------------------------------------------------
|
|  Routine Name: do_region_object_relabel (recursive version)
|
|       Purpose: Performs the connected region relabeling operation on spatial_data
|                as specified by the region_object_relabel_pairs map
|
|         Input: recur_level      (Lowest recursive level at which this task is active)
|                section          (Section or window processed by this call to do_region_object_relabel)
|                region_object_relabel_pairs (Map specifying the connected region relabel table)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                ncols            (Number of columns in spatial_data)
|                nrows            (Number of rows in spatial_data)
|                nslices          (Number of slices in spatial_data)
|                
|        Output:
|
|         Other: temp_data    (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: September 13, 2002.
| Modifications: (see comments for first do_region_class_relabel function)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void do_region_object_relabel(const short unsigned int& recur_level, const short unsigned int& section, 
                             map<unsigned int,unsigned int>& region_object_relabel_pairs,
                             Spatial& spatial_data, const int& ncols, const int& nrows,
                             const int& nslices, Temp& temp_data)
#else
 void do_region_object_relabel(const short unsigned int& recur_level, const short unsigned int& section,
                             map<unsigned int,unsigned int>& region_object_relabel_pairs,
                             Spatial& spatial_data, const int& ncols, const int& nrows,
                             Temp& temp_data)
#endif
 {
   if (params.debug > 3)
   {
     params.log_fs << "do_region_object_relabel called at recur_level = " << recur_level << endl;
   }

   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of do_region_object_relabel can be called.
#ifdef THREEDIM
     do_region_object_relabel(region_object_relabel_pairs,
                            spatial_data,ncols,nrows,nslices);
#else
     do_region_object_relabel(region_object_relabel_pairs,
                            spatial_data,ncols,nrows);
#endif
   }
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // do_region_object_relabel must be called recursively.
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
     unsigned int int_buf_size = 2*region_object_relabel_pairs.size( );
     check_buf_size(0, 0, int_buf_size, 0, 0, temp_data);
     unsigned int int_buf_position = 0;
     map<unsigned int,unsigned int>::iterator region_object_relabel_pair_iter;
     region_object_relabel_pair_iter = region_object_relabel_pairs.begin( );
     while (region_object_relabel_pair_iter != region_object_relabel_pairs.end( ))
     {
       temp_data.int_buffer[int_buf_position++] = (*region_object_relabel_pair_iter).first;
       temp_data.int_buffer[int_buf_position++] = (*region_object_relabel_pair_iter).second;
       ++region_object_relabel_pair_iter;
     }
   // Send request to the parallel recur_tasks
#ifdef THREEDIM
     parallel_recur_requests((short unsigned int) 4, recur_level, 1, ncols, nrows, nslices,
                             0, int_buf_size, 0, temp_data);
   // Process current task's data section
     do_region_object_relabel((recur_level+1),section,region_object_relabel_pairs,spatial_data,
                              recur_ncols,recur_nrows,recur_nslices,temp_data);
#else
     parallel_recur_requests((short unsigned int) 4, recur_level, 1, ncols, nrows,
                             0, int_buf_size, 0, temp_data);
   // Process current task's data section
     do_region_object_relabel((recur_level+1),section,region_object_relabel_pairs,spatial_data,
                              recur_ncols,recur_nrows,temp_data);
#endif

   // Receive completion confirmation from the parallel tasks
     int region_class_relabel_tag = 104;
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
       if (params.debug > 3)
         params.log_fs << "Waiting for confirmation of completion from task " << recur_section << endl;
#ifdef TIME_IT
       end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
       temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
       MPI::COMM_WORLD.Recv(&int_buf_size, 1, MPI::UNSIGNED, recur_section, region_class_relabel_tag);
#ifdef TIME_IT
       end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
       temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
#else // !PARALLEL
     // In the serial case, process the specified data section
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
         spatial_data.restore_region_object_label_map(recur_section);
#ifdef THREEDIM
       do_region_object_relabel((recur_level+1),recur_section,region_object_relabel_pairs,spatial_data,
                                recur_ncols,recur_nrows,recur_nslices,temp_data);
#else
       do_region_object_relabel((recur_level+1),recur_section,region_object_relabel_pairs,spatial_data,
                                recur_ncols,recur_nrows,temp_data);
#endif
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
         spatial_data.save_region_object_label_map(recur_section);
#endif // !PARALLEL
     }
   }

   if (params.debug > 3)
     params.log_fs << "Exited region_object_relabel " << endl;
   return;
 }
} // namespace HSEGTilton
