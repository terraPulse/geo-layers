/*-----------------------------------------------------------
|
|  Routine Name: do_region_classes_init (regular version)
|
|       Purpose: Initialize region_classes from current pixel_data values
|
|         Input: ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                
|        Output: region_classes      (Class which holds region related information)
|
|         Other:
|
|       Returns: (void)
|
|    Written By: James C. Tilton
|          Date: December 9, 2002
| Modifications: February 10, 2003:  Changed region index to region label
|                September 30, 2003:  Added initialization of nghbrs_label_set
|                December 23, 2004:  Changed region label from short unsigned int to unsigned int
|                May 31, 2005 - Added temporary file I/O for faster processing of large data sets
|                October 21, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|                August 1, 2013 - Revised to accommodate standard deviation and region edge information.
|                November 19, 2013 - Added the region_classes_check function
|                January 8, 2014 - Removed the RegionEdge class.
|
------------------------------------------------------------*/

#include "region_class.h"
#include "region_seam.h"
#include "region_object.h"
#include <params/params.h>
#include <pixel/pixel.h>
#include <spatial/spatial.h>
#include <index/index.h>
#include <iostream>
#include <vector>
#ifdef TIME_IT
#include <ctime>
#endif

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef THREEDIM
 unsigned int do_region_classes_init(const int& ncols, const int& nrows,
                                     const int& nslices, vector<Pixel>& pixel_data,
                                     vector<RegionClass>& region_classes)
#else
 unsigned int do_region_classes_init(const int& ncols, const int& nrows,
                                     vector<Pixel>& pixel_data, vector<RegionClass>& region_classes)
#endif
 {
   unsigned int region_index, region_label, max_region_label;
   unsigned int pixel_index, region_classes_size;
   int col, row;
#ifdef THREEDIM
   int slice;
#endif

   max_region_label = 0;
   region_classes_size = region_classes.size( );
   for (region_index = 0; region_index < region_classes_size; ++region_index)
     region_classes[region_index].nghbrs_label_set_clear();
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
       region_label = 0;
       region_label = pixel_data[pixel_index].get_region_label( );
       if (region_label != 0)
       {
         if (region_label > region_classes_size)
         {
           region_classes_size = region_label;
           region_classes.resize(region_classes_size);
         }
         region_index = region_label - 1;
#ifdef THREEDIM
         region_classes[region_index].init(pixel_data,col,row,slice,ncols,nrows,nslices);
#else
         region_classes[region_index].init(pixel_data,col,row,ncols,nrows);
#endif
         if (region_label > max_region_label)
           max_region_label = region_label;
       }
     }
    }
#ifdef THREEDIM
   }
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting do_region_classes_init, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag( ))
         region_classes[region_index].print(region_classes);
   }
#endif
   return max_region_label;
 }

/*-----------------------------------------------------------
|
|  Routine Name: do_region_classes_init (recursive version)
|
|       Purpose: Initialize region_classes from current pixel_data values with recursive calls as necessary
|
|         Input: recur_level      (Current recursive level of this task)
|                section          (Section or window processed by this call to do_region_classes_init)
|                ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                
|        Output: region_classes      (Class which holds region related information)
|
|         Other: temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton
|          Date: December 9, 2002
| Modifications: (see comments for first do_region_classes_init function)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 unsigned int do_region_classes_init(const short unsigned int& recur_level, const short unsigned int& section,
                                  const int& ncols, const int& nrows, const int& nslices,
                                  vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                                  Temp& temp_data)
#else
 unsigned int do_region_classes_init(const short unsigned int& recur_level, const short unsigned int& section,
                                  const int& ncols, const int& nrows,
                                  vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                                  Temp& temp_data)
#endif
 {
   unsigned int region_index, max_region_label;

   if (params.debug > 2)
   {
     params.log_fs << "do_region_classes_init called at recur_level = " << recur_level;
     params.log_fs << " with section = " << section << endl;
   }

   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of do_region_classes_init can be called.
#ifdef THREEDIM
     max_region_label = do_region_classes_init(ncols,nrows,nslices,pixel_data,region_classes);
#else
     max_region_label = do_region_classes_init(ncols,nrows,pixel_data,region_classes);
#endif
#ifdef DEBUG
     unsigned int region_classes_size;
     region_classes_size = region_classes.size( );
     if (params.debug > 2)
     {
       params.log_fs << "Found max_region_label = " << max_region_label;
       params.log_fs << " in current task in do_region_classes_init " << endl;
     }
     if (params.debug > 3)
     {
       params.log_fs << endl << "After call to do_region_classes_init for current task, dump of the region data:" << endl << endl;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         if (region_classes[region_index].get_active_flag( ))
           region_classes[region_index].print(region_classes);
     }
#endif
   }
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // do_region_classes_init must be called recursively.
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
     unsigned int index, region_label, region_classes_size;
   // Send request to the parallel recur_tasks
#ifdef THREEDIM
     parallel_recur_requests((short unsigned int) 5,recur_level,0,ncols,nrows,nslices,0,0,0,temp_data);
   // Process current task's data section
     max_region_label = do_region_classes_init((recur_level+1),section,recur_ncols,recur_nrows,recur_nslices,
                                               pixel_data,region_classes,temp_data);
#else
     parallel_recur_requests((short unsigned int) 5,recur_level,0,ncols,nrows,0,0,0,temp_data);
   // Process current task's data section
     max_region_label = do_region_classes_init((recur_level+1),section,recur_ncols,recur_nrows,
                                                pixel_data,region_classes,temp_data);
#endif
     region_classes_size = region_classes.size( );

     unsigned int nregions;
     unsigned int int_buf_size = 0, float_buf_size = 0, double_buf_size = 0;
     int region_classes_init_tag = 105;
   // Receive region_classes information from the parallel recur_tasks
     int min_section = section + stride;
#ifdef TIME_IT
     float end_time, elapsed_time;
#endif
#else // PARALLEL
     int min_section = section;
     max_region_label = 0;
#endif // !PARALLEL
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
       MPI::COMM_WORLD.Recv(&region_index, 1, MPI::UNSIGNED, recur_section, region_classes_init_tag);
#ifdef TIME_IT
       end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
       temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
       if (region_index > max_region_label)
         max_region_label = region_index;
       if (params.debug > 2)
       {
         params.log_fs << "Received region_classes_init results from task " << recur_section;
         params.log_fs << " with max_region_label = " << max_region_label << endl;
       }
       MPI::COMM_WORLD.Recv(&nregions, 1, MPI::UNSIGNED, recur_section, region_classes_init_tag);
       MPI::COMM_WORLD.Recv(&int_buf_size, 1, MPI::UNSIGNED, recur_section, region_classes_init_tag);
       MPI::COMM_WORLD.Recv(&float_buf_size, 1, MPI::UNSIGNED, recur_section, region_classes_init_tag);
       MPI::COMM_WORLD.Recv(&double_buf_size, 1, MPI::UNSIGNED, recur_section, region_classes_init_tag);
       if (nregions > 0)
       {
         check_buf_size(0,0,int_buf_size,float_buf_size,double_buf_size,temp_data);
         MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, recur_section, region_classes_init_tag);
         MPI::COMM_WORLD.Recv(temp_data.float_buffer, float_buf_size, MPI::FLOAT, recur_section, region_classes_init_tag);
         MPI::COMM_WORLD.Recv(temp_data.double_buffer, double_buf_size, MPI::DOUBLE, recur_section, region_classes_init_tag);
#ifdef TIME_IT
         end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
         elapsed_time = end_time - temp_data.start_time;
         if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
         temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
         if (max_region_label > region_classes_size)
         {
           region_classes_size = max_region_label;
           region_classes.resize(region_classes_size,RegionClass( ));
         }
         for (region_index = 0; region_index < max_region_label; ++region_index)
           region_classes[region_index].set_label(region_index+1);
         unsigned int int_buf_position = 0, float_buf_position = 0, double_buf_position = 0;
         for (index = 0; index < nregions; ++index)
         {
           region_label = temp_data.int_buffer[int_buf_position++];
           region_index = region_label - 1;
           region_classes[region_index].set_active_flag(true);
           region_classes[region_index].update_all_data(temp_data,int_buf_position,float_buf_position,double_buf_position);
         }
       }
#else // !PARALLEL
     // In the serial case, process the specified data section
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
         restore_pixel_data(recur_section,pixel_data,temp_data);
#ifdef THREEDIM
       region_index = do_region_classes_init((recur_level+1),recur_section,recur_ncols,recur_nrows,recur_nslices,
                                             pixel_data,region_classes,temp_data);
#else
       region_index = do_region_classes_init((recur_level+1),recur_section,recur_ncols,recur_nrows,
                                             pixel_data,region_classes,temp_data);
#endif
       if (region_index > max_region_label)
         max_region_label = region_index;
#endif // !PARALLEL
     } // for (recur_section = min_section; recur_section < max_section; recur_section += stride)

   // Obtain the neighborhood information from along the processing window seams
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
   region_classes_size = region_classes.size( );
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting do_region_classes_init, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag( ))
         region_classes[region_index].print(region_classes);
   }
   if (params.debug > 2)
   {
     params.log_fs << endl << "Exiting do_region_classes_init, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag( ))
       {
         params.log_fs << "For region label " << region_classes[region_index].get_label( );
         params.log_fs << ", npix = " << region_classes[region_index].get_npix( ) << endl;
        }
   }
#endif
   return max_region_label;
 }

#ifdef THREEDIM
 bool region_classes_check(const short unsigned int& recur_level, const short unsigned int& section,
                           const int& ncols, const int& nrows, const int& nslices,
                           vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                           Temp& temp_data)
#else
 bool region_classes_check(const short unsigned int& recur_level, const short unsigned int& section,
                           const int& ncols, const int& nrows,
                           vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                           Temp& temp_data)
#endif
 {
    unsigned int region_index, region_classes_size = region_classes.size();
    vector<RegionClass> check_region_classes(region_classes_size,RegionClass( ));
#ifdef THREEDIM
    do_region_classes_init(recur_level,section,ncols,nrows,nslices,
                           pixel_data,check_region_classes,temp_data);
#else
    do_region_classes_init(recur_level,section,ncols,nrows,
                           pixel_data,check_region_classes,temp_data);
#endif

    bool compare_flag = true;
    for (region_index = 0; region_index < region_classes_size; ++region_index)
    {
      compare_flag = true;
      if (region_classes[region_index].get_active_flag( ))
        compare_flag = compare_region_classes(region_classes[region_index], check_region_classes[region_index]);
      if (!compare_flag)
        break;
    }

    if (compare_flag)
      return true;
    else
    {
      if (params.debug > 0)
      {
        params.log_fs << "WARNING: Mismatch found between region_classes and check_region_classes in region_classes_check function" << endl;
        params.log_fs << "Contents of region_classes:" << endl;
        region_classes[region_index].print(region_classes);
        params.log_fs << "Contents of check_region_classes:" << endl;
        check_region_classes[region_index].print(check_region_classes);
      }
      else
      {
        cout << "WARNING: Mismatch found between region_classes and check_region_classes in region_classes_check function" << endl;
      }

      return false;
    }
 }

/*-----------------------------------------------------------
|
|  Routine Name: seam_region_classes_init
|
|       Purpose: Initialize the seam_region_classes with the col/row/slice_seam_index_data.
|
|         Input: ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                seam_index_data  (Image data information from the processing window column/row/slice seam)
|                
|        Output: seam_region_classes  (Class which holds region related information)
|
|         Other:
|
|       Returns: (void)
|
|    Written By: James C. Tilton
|          Date: February 3, 2014
| Modifications: March 27, 2014 - Modified to allow initialization of multiple region pairs.
|
------------------------------------------------------------*/

#ifdef THREEDIM
 void seam_region_classes_init(const bool& col_flag, const bool& row_flag, const bool& slice_flag,
                               const int& ncols, const int& nrows, const int& nslices,
                               vector<Index>& seam_index_data, vector<RegionClass>& seam_region_classes)
#else
 void seam_region_classes_init(const bool& col_flag, const bool& row_flag, const int& ncols, const int& nrows,
                               vector<Index>& seam_index_data, vector<RegionClass>& seam_region_classes)
#endif
 {
   short unsigned int nbdir;
   unsigned int region_index, region_label, region_classes_size, nghbr_label;
   unsigned int seam_index, seam_nbindex, seam_npix;
   int col, row, nbcol, nbrow;
#ifdef THREEDIM
   int slice, nbslice;
#endif
   int start, end;
   float seam_edge_value;
   set<unsigned int> nghbrs_label_set;
   set<unsigned int>::iterator nghbrs_label_set_iter;
   map<unsigned int, RegionSeam>::iterator  nghbrs_label_seam_map_iter;

   region_classes_size = seam_region_classes.size( );
   for (region_index = 0; region_index < region_classes_size; ++region_index)
   {
     seam_region_classes[region_index].clear( );
     region_label = region_index+1;
     seam_region_classes[region_index].set_label(region_label);
   }

   end = params.seam_size/2;
   start = end - 1;

   if (col_flag)
   {
#ifdef THREEDIM
     for (slice = 0; slice < nslices; slice++)
     {
#endif
       for (row = 0; row < nrows; row++)
       {
         for (col = start; col <= end; col++)
         {
#ifdef THREEDIM
           seam_index = row + slice*nrows + col*nslices*nrows;
#else
           seam_index = row + col*nrows;
#endif
           region_label = seam_index_data[seam_index].get_region_class_label( );
           if ((region_label != 0) && (seam_index_data[seam_index].get_edge_mask()))
           {
             nghbrs_label_set.clear();
             for (nbdir = 0; nbdir < params.maxnbdir; ++nbdir)
             {
#ifdef THREEDIM
               find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
               if ((nbslice>=0)&&(nbrow>=0)&&(nbcol>=0)&&(nbslice<nslices)&&(nbrow<nrows)&&(nbcol<ncols))
               {
                 seam_nbindex = nbrow + nbslice*nrows + nbcol*nslices*nrows;
#else
               find_nghbr(col,row,nbdir,nbcol,nbrow);
               if ((nbrow>=0)&&(nbcol>=0)&&(nbrow<nrows)&&(nbcol<ncols))
               {
                 seam_nbindex = nbrow + nbcol*nrows;
#endif
                 nghbr_label = seam_index_data[seam_nbindex].get_region_class_label();
                 if ((nghbr_label != 0) && (region_label != nghbr_label))
                   nghbrs_label_set.insert(nghbr_label);
               }
             } // for (nbdir = 0; nbdir < params.maxnbdir; ++nbdir)
           // Restrict to locations with only one neighbor - which is directly across the seam
             if (nghbrs_label_set.size() == 1)
             {
             // Check that this one neighbor is directly across the seam
               if (col == start)
                 nbcol = col + 1;
               else
                 nbcol = col - 1;
#ifdef THREEDIM
               seam_nbindex = nbrow + nbslice*nrows + nbcol*nslices*nrows;
#else
               seam_nbindex = nbrow + nbcol*nrows;
#endif
               nghbr_label = seam_index_data[seam_nbindex].get_region_class_label();
               nghbrs_label_set_iter = nghbrs_label_set.begin();
               if (nghbr_label == (*nghbrs_label_set_iter))
               {
                 if (region_label > region_classes_size) // Should not occur!
                 {
                   region_classes_size = region_label;
                   seam_region_classes.resize(region_classes_size);
                 }
                 seam_npix = 1;
                 seam_edge_value = seam_index_data[seam_index].get_edge_value();
                 RegionSeam region_seam(seam_npix,seam_edge_value);
                 region_index = region_label - 1;
                 nghbrs_label_seam_map_iter = seam_region_classes[region_index].nghbrs_label_seam_map.find(nghbr_label);
                 if (nghbrs_label_seam_map_iter == seam_region_classes[region_index].nghbrs_label_seam_map.end())
                   seam_region_classes[region_index].nghbrs_label_seam_map.insert(make_pair(nghbr_label,region_seam));
                 else
                   (*nghbrs_label_seam_map_iter).second += region_seam;
                 seam_region_classes[region_index].set_active_flag(true);
               } // if (nghbr_label == (*nghbrs_label_set_iter))
             } // if (nghbrs_label_set.size() == 1)
           } // if ((region_label != 0) && (seam_index_data[seam_index].get_edge_mask()))
         } // for (col = start; col <= end; col++)
       } // for (row = 0; row < nrows; row++)
#ifdef THREEDIM
     } // for (slice = 0; slice < nslices; slice++)
#endif
   } // if (col_flag)

   if (row_flag)
   {
#ifdef THREEDIM
     for (slice = 0; slice < nslices; slice++)
     {
#endif
       for (row = start; row <=end; row++)
       {
         for (col = 0; col < ncols; col++)
         {
#ifdef THREEDIM
           seam_index = col + slice*ncols + row*nslices*ncols;
#else
           seam_index = col + row*ncols;
#endif
           region_label = seam_index_data[seam_index].get_region_class_label( );
           if ((region_label != 0) && (seam_index_data[seam_index].get_edge_mask()))
           {
             nghbrs_label_set.clear();
             for (nbdir = 0; nbdir < params.maxnbdir; ++nbdir)
             {
#ifdef THREEDIM
               find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
               if ((nbslice>=0)&&(nbrow>=0)&&(nbcol>=0)&&(nbslice<nslices)&&(nbrow<nrows)&&(nbcol<ncols))
               {
                 seam_nbindex = nbcol + nbslice*ncols + nbrow*nslices*ncols;
#else
               find_nghbr(col,row,nbdir,nbcol,nbrow);
               if ((nbrow>=0)&&(nbcol>=0)&&(nbrow<nrows)&&(nbcol<ncols))
               {
                 seam_nbindex = nbcol + nbrow*ncols;
#endif
                 nghbr_label = seam_index_data[seam_nbindex].get_region_class_label();
                 if ((nghbr_label != 0) && (region_label != nghbr_label))
                   nghbrs_label_set.insert(nghbr_label);
               }
             } // for (nbdir = 0; nbdir < params.maxnbdir; ++nbdir)
           // Restrict to locations with only one neighbor - which is directly across the seam
             if (nghbrs_label_set.size() == 1)
             {
             // Check that this one neighbor is directly across the seam
               if (row == start)
                 nbrow = row + 1;
               else
                 nbrow = row - 1;
#ifdef THREEDIM
               seam_nbindex = nbcol + nbslice*ncols + nbrow*nslices*ncols;
#else
               seam_nbindex = nbcol + nbrow*ncols;
#endif
               nghbr_label = seam_index_data[seam_nbindex].get_region_class_label();
               nghbrs_label_set_iter = nghbrs_label_set.begin();
               if (nghbr_label == (*nghbrs_label_set_iter))
               {
                 if (region_label > region_classes_size) // Should not occur!
                 {
                   region_classes_size = region_label;
                   seam_region_classes.resize(region_classes_size);
                 }
                 seam_npix = 1;
                 seam_edge_value = seam_index_data[seam_index].get_edge_value();
                 RegionSeam region_seam(seam_npix,seam_edge_value);
                 region_index = region_label - 1;
                 nghbrs_label_seam_map_iter = seam_region_classes[region_index].nghbrs_label_seam_map.find(nghbr_label);
                 if (nghbrs_label_seam_map_iter == seam_region_classes[region_index].nghbrs_label_seam_map.end())
                   seam_region_classes[region_index].nghbrs_label_seam_map.insert(make_pair(nghbr_label,region_seam));
                 else
                   (*nghbrs_label_seam_map_iter).second += region_seam;
                 seam_region_classes[region_index].set_active_flag(true);
               } // if (nghbr_label == (*nghbrs_label_set_iter))
             } // if (nghbrs_label_set.size() == 1)
           } // if ((region_label != 0) && (seam_index_data[seam_index].get_edge_mask()))
         } // for (col = 0; col < ncols; col++)
       } // for (row = start; row <=end; row++)
#ifdef THREEDIM
     } // for (slice = 0; slice < nslices; slice++)
#endif
   } // if (row_flag)

#ifdef THREEDIM
   if (slice_flag)
   {
     for (slice = start; slice <= end; slice++)
     {
       for (row = 0; row < nrows; row++)
       {
         for (col = 0; col < ncols; col++)
         {
           seam_index = col + row*ncols + slice*nrows*ncols;
           region_label = seam_index_data[seam_index].get_region_class_label( );
           if ((region_label != 0) && (seam_index_data[seam_index].get_edge_mask()))
           {
             nghbrs_label_set.clear();
             for (nbdir = 0; nbdir < params.maxnbdir; ++nbdir)
             {
               find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
               if ((nbslice>=0)&&(nbrow>=0)&&(nbcol>=0)&&(nbslice<nslices)&&(nbrow<nrows)&&(nbcol<ncols))
               {
                 seam_nbindex = nbcol + nbrow*ncols + nbslice*nrows*ncols;
                 nghbr_label = seam_index_data[seam_nbindex].get_region_class_label();
                 if ((nghbr_label != 0) && (region_label != nghbr_label))
                   nghbrs_label_set.insert(nghbr_label);
               }
             } // for (nbdir = 0; nbdir < params.maxnbdir; ++nbdir)
           // Restrict to locations with only one neighbor - which is directly across the seam
             if (nghbrs_label_set.size() == 1)
             {
             // Check that this one neighbor is directly across the seam
               if (slice == start)
                 nbslice = slice + 1;
               else
                 nbslice = slice - 1;
               seam_nbindex = nbcol + row*ncols + slice*nrows*ncols;
               nghbr_label = seam_index_data[seam_nbindex].get_region_class_label();
               nghbrs_label_set_iter = nghbrs_label_set.begin();
               if (nghbr_label == (*nghbrs_label_set_iter))
               {
                 if (region_label > region_classes_size) // Should not occur!
                 {
                   region_classes_size = region_label;
                   seam_region_classes.resize(region_classes_size);
                 }
                 seam_npix = 1;
                 seam_edge_value = seam_index_data[seam_index].get_edge_value();
                 RegionSeam region_seam(seam_npix,seam_edge_value);
                 region_index = region_label - 1;
                 nghbrs_label_seam_map_iter = seam_region_classes[region_index].nghbrs_label_seam_map.find(nghbr_label);
                 if (nghbrs_label_seam_map_iter == seam_region_classes[region_index].nghbrs_label_seam_map.end())
                   seam_region_classes[region_index].nghbrs_label_seam_map.insert(make_pair(nghbr_label,region_seam));
                 else
                   (*nghbrs_label_seam_map_iter).second += region_seam;
                 seam_region_classes[region_index].set_active_flag(true);
               } // if (nghbr_label == (*nghbrs_label_set_iter))
             } // if (nghbrs_label_set.size() == 1)
           } // if ((region_label != 0) && (seam_index_data[seam_index].get_edge_mask()))
         } // for (col = 0; col < ncols; col++)
       } // for (row = 0; row < nrows; row++)
     } // for (slice = start; slice <= end; slice++)
   } // if (slice_flag)
#endif

   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: do_region_objects_init (regular version)
|
|       Purpose: Initialize region_objects from current pixel_data and spatial_data values
|
|         Input: ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                
|        Output: region_objects (Class which holds connected region related information)
|
|         Other:
|
|       Returns: (void)
|
|    Written By: James C. Tilton
|          Date: December 9, 2002
| Modifications: (see comments for first do_region_classes_init function)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void do_region_objects_init(const int& ncols, const int& nrows, const int& nslices,
                             vector<Pixel>& pixel_data, Spatial& spatial_data,
                             vector<RegionObject>& region_objects)
#else
 void do_region_objects_init(const int& ncols, const int& nrows,
                             vector<Pixel>& pixel_data, Spatial& spatial_data,
                             vector<RegionObject>& region_objects)
#endif
 {
   unsigned int region_object_index, region_object_label, pixel_index;
   int col, row;

#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Entering do_region_objects_init, dump of spatial_data.region_object_label:" << endl << endl;
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
//         params.log_fs << spatial_data.region_object_label_map[pixel_index] << " ";
       }
       params.log_fs << endl;
      }
      params.log_fs << endl;
#ifdef THREEDIM
     }
#endif
   }
#endif

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
       region_object_label = spatial_data.get_region_object_label(pixel_index);
       if (region_object_label != 0)
       {
         region_object_index = region_object_label - 1;
         region_objects[region_object_index].init(&pixel_data[pixel_index]);
       }
     }
#ifdef THREEDIM
   }
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting region_objects_init, dump of the connected region data:" << endl << endl;
     unsigned int region_objects_size = region_objects.size( );
     for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
       if (region_objects[region_object_index].get_active_flag( ))
         region_objects[region_object_index].print(region_objects);
   }
#endif
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: do_region_objects_init (recursive version)
|
|       Purpose: Initialize region_objects from current pixel_data and spatial_data values with recursive calls as necessary
|
|         Input: recur_level      (Current recursive level of this task)
|                section          (Section or window processed by this call to do_region_objects_init)
|                ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                
|        Output: region_objects (Class which holds connected region related information)
|
|         Other: temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton
|          Date: December 9, 2002
| Modifications: (see comments for first do_region_classes_init function)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 unsigned int do_region_objects_init(const short unsigned int& recur_level, const short unsigned int& section,
                                     const int& ncols, const int& nrows, const int& nslices, vector<Pixel>& pixel_data,
                                     Spatial& spatial_data, vector<RegionObject>& region_objects,
                                     Temp& temp_data)
#else
 unsigned int do_region_objects_init(const short unsigned int& recur_level, const short unsigned int& section,
                                     const int& ncols, const int& nrows, vector<Pixel>& pixel_data,
                                     Spatial& spatial_data, vector<RegionObject>& region_objects,
                                     Temp& temp_data)
#endif
 {
   unsigned int region_object_index, region_objects_size, nb_objects;

   if (params.debug > 2)
   {
     params.log_fs << "do_region_objects_init called at recur_level = " << recur_level << endl;
   }

   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of do_region_objects_init can be called.
#ifdef THREEDIM
     do_region_objects_init(ncols,nrows,nslices,pixel_data,spatial_data,region_objects);
#else
     do_region_objects_init(ncols,nrows,pixel_data,spatial_data,region_objects);
#endif
   }
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // do_region_objects_init must be called recursively.
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
     nb_objects = region_objects.size( );
   // Send request to the parallel recur_tasks
#ifdef THREEDIM
     parallel_recur_requests((short unsigned int) 5,recur_level,nb_objects,ncols,nrows,nslices,0,0,0,temp_data);
   // Process current task's data section
     do_region_objects_init((recur_level+1),section,recur_ncols,recur_nrows,recur_nslices,
                             pixel_data,spatial_data,region_objects,temp_data);
#else
     parallel_recur_requests((short unsigned int) 5,recur_level,nb_objects,ncols,nrows,0,0,0,temp_data);
   // Process current task's data section
     do_region_objects_init((recur_level+1),section,recur_ncols,recur_nrows,
                             pixel_data,spatial_data,region_objects,temp_data);
#endif
     region_objects_size = region_objects.size( );
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "After recursive call to do_region_objects_init, dump of the region_object data:" << endl << endl;
       for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
         if (region_objects[region_object_index].get_active_flag( ))
           region_objects[region_object_index].print(region_objects);
     }
#endif

     unsigned int index, int_buf_size = 0, double_buf_size = 0;
     int region_classes_init_tag = 105;
   // Receive region_objects information from the parallel recur_tasks
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
       MPI::COMM_WORLD.Recv(&int_buf_size, 1, MPI::UNSIGNED, recur_section, region_classes_init_tag);
#ifdef TIME_IT
       end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
       temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
       MPI::COMM_WORLD.Recv(&double_buf_size, 1, MPI::UNSIGNED, recur_section, region_classes_init_tag);
       if (params.debug > 2)
       {
         params.log_fs << "Received region_objects_init results from task " << recur_section;
         params.log_fs << " with int_buf_size = " << int_buf_size << " and double_buf_size = " << double_buf_size << endl;
       }
       if (int_buf_size > 0)
       {
         check_buf_size(0,0,int_buf_size,0,double_buf_size,temp_data);
         MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, recur_section, region_classes_init_tag);
         MPI::COMM_WORLD.Recv(temp_data.double_buffer, double_buf_size, MPI::DOUBLE, recur_section, region_classes_init_tag);
#ifdef TIME_IT
         end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
         elapsed_time = end_time - temp_data.start_time;
         if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
         temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
         nb_objects = int_buf_size/2;
         int_buf_size = double_buf_size = 0;
         for (index = 0; index < nb_objects; ++index)
         {
           region_object_index = temp_data.int_buffer[int_buf_size++] - 1;
           region_objects[region_object_index].set_active_flag(true);
           region_objects[region_object_index].update_data(temp_data,int_buf_size,double_buf_size);
         }
       }
#else // !PARALLEL
     // In the serial case, process the specified data section
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
       {
         restore_pixel_data(recur_section,pixel_data,temp_data);
         spatial_data.restore_region_object_label_map(recur_section);
       }
#ifdef THREEDIM
       do_region_objects_init((recur_level+1),recur_section,
                                recur_ncols,recur_nrows,recur_nslices,pixel_data,spatial_data,
                                region_objects,temp_data);
#else
       do_region_objects_init((recur_level+1),recur_section,
                                recur_ncols,recur_nrows,pixel_data,spatial_data,
                                region_objects,temp_data);
#endif
#endif // !PARALLEL
     } // for (recur_section = min_section; recur_section < max_section; recur_section += stride)
   }
   nb_objects = 0;
   region_objects_size = region_objects.size( );
   for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
     if (region_objects[region_object_index].get_active_flag( ))
       nb_objects++;
#ifdef DEBUG
   if (params.debug > 2)
   {
     params.log_fs << "Completed region_objects initialization with " << nb_objects;
     params.log_fs << " active regions" << endl;
   }
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting do_region_objects_init, dump of the region_object data:" << endl << endl;
     for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
       if (region_objects[region_object_index].get_active_flag( ))
         region_objects[region_object_index].print(region_objects);
   }
#endif
   return nb_objects;
 }
} // namespace HSEGTilton
