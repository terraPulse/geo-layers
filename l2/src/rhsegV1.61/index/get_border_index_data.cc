/*-----------------------------------------------------------
|
|  Routine Name: get_border_index_data (regular version)
|
|       Purpose: Gets the col_border_index_data, row_border_index_data and slice_border_index_data values
|                from pixel_data
|
|         Input: recur_level      (Current recursive level)
|                section          (Section or window processed by this call to get_border_index_data)
|                border_flag      (Border selection flag)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                ncols            (Number of columns in data being processed)
|                nrows            (Number of rows in data being processed)
|                nslices          (Number of slices in data being processed)
|
|        Output: col_border_index_data (Index class information from the processing window column border)
|                row_border_index_data (Index class information from the processing window row border)
|                slice_border_index_data (Index class information from the processing window slice border)
|
|         Other:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: October 7, 2003.
| Modifications: November 13, 2003 - Made class member variables private.
|                December 22, 2004 - Changed region label from short unsigned int to unsigned int
|                June 15, 2005 - Added temporary file I/O for faster processing of large data sets
|                October 30, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|                April 25, 2011 - Added border_flag to designate: 1 => col only, 2 => row only, 3=> col & row, 
|                                 4=> slice only, 5=> col & slice, 6 => row & slice, 7 => col, row & slice
|
------------------------------------------------------------*/

#include "index.h"
#include <params/params.h>
#include <pixel/pixel.h>
#include <spatial/spatial.h>
#ifdef TIME_IT
#include <ctime>
#endif

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef THREEDIM
 void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                            const unsigned char& border_flag, vector<Pixel>& pixel_data,
                            const int& ncols, const int& nrows, const int& nslices,
                            vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                            vector<Index>& slice_border_index_data)
#else
 void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                            const unsigned char& border_flag, vector<Pixel>& pixel_data,
                            const int& ncols, const int& nrows,
                            vector<Index>& col_border_index_data, vector<Index>& row_border_index_data)
#endif
 {
   unsigned int pixel_index;
   int col, row;
#ifdef THREEDIM
   int slice;
#endif

  // Set recursive image sizes
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

  // Set border_flags
   bool col_border_flag, row_border_flag;
#ifdef THREEDIM
   bool slice_border_flag;
#endif

#ifdef THREEDIM
   set_recur_flags(border_flag,col_border_flag,row_border_flag,slice_border_flag);
#else
   set_recur_flags(border_flag,col_border_flag,row_border_flag);
#endif

#ifdef DEBUG
   if (params.debug > 2)
   {
     params.log_fs << endl << "Entering get_border_index_data, ncols = " << ncols;
#ifdef THREEDIM
     params.log_fs << ", nrows = " << nrows << " and nslices = " << nslices << endl << endl;
#else
     params.log_fs << " and nrows = " << nrows << endl << endl;
#endif
     if (col_border_flag)
       params.log_fs << "Requesting col_border_index_data" << endl;
     if (row_border_flag)
       params.log_fs << "Requesting row_border_index_data" << endl;
#ifdef THREEDIM
     if (slice_border_flag)
       params.log_fs << "Requesting slice_border_index_data" << endl;
#endif
   }
   if (params.debug > 3)
   {
     if (col_border_flag)
     {
       params.log_fs << endl << "Entering get_border_index_data, pixel_data.region_label:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < (params.seam_size/2); col++)
           {
#ifdef THREEDIM
             pixel_index = col + row*ncols + slice*nrows*ncols;
#else
             pixel_index = col + row*ncols;
#endif
             params.log_fs << pixel_data[pixel_index].get_region_label( ) << " ";
           }
           for (col = (params.seam_size/2); col < params.seam_size; col++)
           {
#ifdef THREEDIM
             pixel_index = (col+ncols-params.seam_size) + row*ncols + slice*nrows*ncols;
#else
             pixel_index = (col+ncols-params.seam_size) + row*ncols;
#endif
             params.log_fs << pixel_data[pixel_index].get_region_label( ) << " ";
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (col_border_flag)
     if (row_border_flag)
     {
       params.log_fs << endl << "Entering get_border_index_data, pixel_data.region_label:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < (params.seam_size/2); row++)
         {
           for (col = 0; col < ncols; col++)
           {
#ifdef THREEDIM
             pixel_index = col + row*ncols + slice*nrows*ncols;
#else
             pixel_index = col + row*ncols;
#endif
             params.log_fs << pixel_data[pixel_index].get_region_label( ) << " ";
           }
         }
         for (row = (params.seam_size/2); row < params.seam_size; row++)
         {
           for (col = 0; col < ncols; col++)
           {
#ifdef THREEDIM
             pixel_index = col + (row+nrows-params.seam_size)*ncols + slice*nrows*ncols;
#else
             pixel_index = col + (row+nrows-params.seam_size)*ncols;
#endif
             params.log_fs << pixel_data[pixel_index].get_region_label( ) << " ";
           }
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
       params.log_fs << endl;
#endif
     } // if (row_border_flag)
#ifdef THREEDIM
     if (slice_border_flag)
     {
       params.log_fs << endl << "Entering get_border_index_data, pixel_data.region_label:" << endl << endl;
       for (slice = 0; slice < (params.seam_size/2); slice++)
       {
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < ncols; col++)
           {
             pixel_index = col + row*ncols + slice*nrows*ncols;
             params.log_fs << pixel_data[pixel_index].get_region_label( ) << " ";
           }
           params.log_fs << endl;
         }
       }
       for (slice = (params.seam_size/2); slice < params.seam_size; slice++)
       {
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < ncols; col++)
           {
             pixel_index = col + row*ncols + (slice+nslices-params.seam_size)*nrows*ncols;
             params.log_fs << pixel_data[pixel_index].get_region_label( ) << " ";
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
       }
     } // if (slice_border_flag)
#endif
   }
#endif
   if (col_border_flag)
   {
    // Set values for col_border_index_data
#ifdef THREEDIM
     for (slice = 0; slice < nslices; slice++)
     {
#endif
       for (row = 0; row < nrows; row++)
       {
         for (col = 0; col < (params.seam_size/2); col++)
         {
#ifdef THREEDIM
           pixel_index = col + row*ncols + slice*nrows*ncols;
           col_border_index_data[row + slice*nrows + col*nslices*nrows].set_data(section,pixel_data,pixel_index);
#else
           pixel_index = col + row*ncols;
           col_border_index_data[row + col*nrows].set_data(section,pixel_data,pixel_index);
#endif
         }
         for (col = (params.seam_size/2); col < params.seam_size; col++)
         {
#ifdef THREEDIM
           pixel_index = (col+ncols-params.seam_size) + row*ncols + slice*nrows*ncols;
           col_border_index_data[row + slice*nrows + col*nslices*nrows].set_data(section,pixel_data,pixel_index);
#else
           pixel_index = (col+ncols-params.seam_size) + row*ncols;
           col_border_index_data[row + col*nrows].set_data(section,pixel_data,pixel_index);
#endif
         }
       }
#ifdef THREEDIM
     }
#endif
   } // if (col_border_flag)
   if (row_border_flag)
   {
    // Set values for row_border_index_data
#ifdef THREEDIM
     for (slice = 0; slice < nslices; slice++)
     {
#endif
       for (row = 0; row < (params.seam_size/2); row++)
       {
         for (col = 0; col < ncols; col++)
         {
#ifdef THREEDIM
           pixel_index = col + row*ncols + slice*nrows*ncols;
           row_border_index_data[col + slice*ncols + row*nslices*ncols].set_data(section,pixel_data,pixel_index);
#else
           pixel_index = col + row*ncols;
           row_border_index_data[col + row*ncols].set_data(section,pixel_data,pixel_index);
#endif
         }
       }
       for (row = (params.seam_size/2); row < params.seam_size; row++)
       {
         for (col = 0; col < ncols; col++)
         {
#ifdef THREEDIM
           pixel_index = col + (row+nrows-params.seam_size)*ncols + slice*nrows*ncols;
           row_border_index_data[col + slice*ncols + row*nslices*ncols].set_data(section,pixel_data,pixel_index);
#else
           pixel_index = col + (row+nrows-params.seam_size)*ncols;
           row_border_index_data[col + row*ncols].set_data(section,pixel_data,pixel_index);
#endif
         }
       }
#ifdef THREEDIM
     }
#endif
   } // if (row_border_flag)
#ifdef THREEDIM
   if (slice_border_flag)
   {
    // Set values for slice_border_index_data
     for (slice = 0; slice < (params.seam_size/2); slice++)
     {
       for (row = 0; row < nrows; row++)
       {
         for (col = 0; col < ncols; col++)
         {
           pixel_index = col + row*ncols + slice*nrows*ncols;
           slice_border_index_data[col + row*ncols + slice*nrows*ncols].set_data(section,pixel_data,pixel_index);
         }
       }
     }
     for (slice = (params.seam_size/2); slice < params.seam_size; slice++)
     {
       for (row = 0; row < nrows; row++)
       {
         for (col = 0; col < ncols; col++)
         {
           pixel_index = col + row*ncols + (slice+nslices-params.seam_size)*nrows*ncols;
           slice_border_index_data[col + row*ncols + slice*nrows*ncols].set_data(section,pixel_data,pixel_index);
         }
       }
     }
   } // if (slice_border_flag)
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     if (col_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data, col_border_index_data:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < params.seam_size; col++)
           {
#ifdef THREEDIM
              params.log_fs << col_border_index_data[row + slice*nrows + col*nslices*nrows].get_region_class_label( ) << " ";
#else
              params.log_fs << col_border_index_data[row + col*nrows].get_region_class_label( ) << " ";
#endif
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (col_border_flag)
     if (row_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data, row_border_index_data:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < params.seam_size; row++)
         {
           for (col = 0; col < ncols; col++)
           {
#ifdef THREEDIM
             params.log_fs << row_border_index_data[col + slice*ncols + row*nslices*ncols].get_region_class_label( ) << " ";
#else
             params.log_fs << row_border_index_data[col + row*ncols].get_region_class_label( ) << " ";
#endif
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (row_border_flag)
#ifdef THREEDIM
     if (slice_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data, slice_border_index_data:" << endl << endl;
       for (slice = 0; slice < params.seam_size; slice++)
       {
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < ncols; col++)
           {
             params.log_fs << slice_border_index_data[col + row*ncols + slice*nrows*ncols].get_region_class_label( ) << " ";
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
       }
     } // if (slice_border_flag)
#endif
   }
   if (params.debug > 2)
   {
     params.log_fs << endl << "Exiting get_border_index_data" << endl << endl;
   }
#endif
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: get_border_index_data (recursive version)
|
|       Purpose: Gets the col_border_index_data, row_border_index_data and slice_border_index_data values
|                from pixel_data
|
|         Input: recur_level      (Current recursive level)
|                section          (Section or window processed by this call to get_border_index_data)
|                border_flag      (Border selection flag)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                ncols            (Number of columns in data being processed)
|                nrows            (Number of rows in data being processed)
|                nslices          (Number of slices in data being processed)
|                
|        Output: col_border_index_data (Index class information from the processing window column border)
|                row_border_index_data (Index class information from the processing window row border)
|                slice_border_index_data (Index class information from the processing window slice border)1
|
|         Other: temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: October 7, 2003.
| Modifications: (See comments for first instance of get_border_index_data.)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                            const unsigned char& border_flag, vector<Pixel>& pixel_data,
                            const int& ncols, const int& nrows, const int& nslices,
                            vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                            vector<Index>& slice_border_index_data, Temp& temp_data)
#else
 void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                            const unsigned char& border_flag, vector<Pixel>& pixel_data,
                            const int& ncols, const int& nrows,
                            vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                            Temp& temp_data)
#endif
 {
   int col, row;
#ifdef THREEDIM
   int slice;
#endif

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

  // Set border_flags
   bool col_border_flag, row_border_flag;
#ifdef THREEDIM
   bool slice_border_flag;
#endif

#ifdef THREEDIM
   set_recur_flags(border_flag,col_border_flag,row_border_flag,slice_border_flag);
#else
   set_recur_flags(border_flag,col_border_flag,row_border_flag);
#endif

   if (params.debug > 2)
   {
     params.log_fs << "Calling get_border_index_data(P) for section = " << section << " with ncols = " << ncols << ", nrows = " << nrows;
#ifdef THREEDIM
     params.log_fs << " and nslices = " << nslices << endl << "at recur_level = " << recur_level << endl;
#else
     params.log_fs << endl << " at recur_level = " << recur_level << endl;
#endif
     if (col_border_flag)
       params.log_fs << "Requesting col_border_index_data" << endl;
     if (row_border_flag)
       params.log_fs << "Requesting row_border_index_data" << endl;
#ifdef THREEDIM
     if (slice_border_flag)
       params.log_fs << "Requesting slice_border_index_data" << endl;
#endif
   }

   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of get_border_index_data can be called.
#ifdef THREEDIM
     get_border_index_data(recur_level,section,border_flag,pixel_data,ncols,nrows,nslices,
                           col_border_index_data,row_border_index_data,slice_border_index_data);
#else
     get_border_index_data(recur_level,section,border_flag,pixel_data,ncols,nrows,
                           col_border_index_data,row_border_index_data);
#endif
   }
   else // if (recur_level < (params.onb_levels-1))
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and get_border_index_data must be called recursively.
     unsigned int recur_index_data_size = 1;
     if (col_border_flag)
#ifdef THREEDIM
       recur_index_data_size = recur_nrows*recur_nslices*params.seam_size;
#else
       recur_index_data_size = recur_nrows*params.seam_size;
#endif
     vector<Index> recur_col_border_index_data(recur_index_data_size);
     recur_index_data_size = 1;
     if (row_border_flag)
#ifdef THREEDIM
       recur_index_data_size = recur_ncols*recur_nslices*params.seam_size;
#else
       recur_index_data_size = recur_ncols*params.seam_size;
#endif
     vector<Index> recur_row_border_index_data(recur_index_data_size);
#ifdef THREEDIM
     recur_index_data_size = 1;
     if (slice_border_flag)
       recur_index_data_size = recur_ncols*recur_nrows*params.seam_size;
     vector<Index> recur_slice_border_index_data(recur_index_data_size);
#endif

     int recur_section, min_section;
     int stride, nb_sections;
     set_stride_sections(recur_level,stride,nb_sections);
#ifdef PARALLEL
     unsigned int value = border_flag;
#ifdef THREEDIM
   // Send request to the parallel recursive tasks
     parallel_recur_requests((short unsigned int) 6,recur_level,value,ncols,nrows,nslices,
                             0,0,0,temp_data);
   // Process current task's data section
     get_border_index_data((recur_level+1),section,border_flag,pixel_data,recur_ncols,recur_nrows,recur_nslices,
                           recur_col_border_index_data,recur_row_border_index_data,
                           recur_slice_border_index_data,temp_data);
#else
   // Send request to the parallel recursive tasks
     parallel_recur_requests((short unsigned int) 6,recur_level,value,ncols,nrows,
                             0,0,0,temp_data);
   // Process current task's data section
     get_border_index_data((recur_level+1),section,border_flag,pixel_data,recur_ncols,recur_nrows,
                           recur_col_border_index_data,recur_row_border_index_data,
                           temp_data);
#endif

     if (col_border_flag)
     {
#ifdef THREEDIM
       for (slice = 0; slice < recur_nslices; slice++)
       {
        for (row = 0; row < recur_nrows; row++)
         for (col = 0; col < (params.seam_size/2); col++)
           col_border_index_data[row + slice*nrows + col*nslices*nrows] =
                                recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
       }
#else
        for (row = 0; row < recur_nrows; row++)
         for (col = 0; col < (params.seam_size/2); col++)
           col_border_index_data[row + col*nrows] =
                                recur_col_border_index_data[row + col*recur_nrows];
#endif
     } // if (col_border_flag)
     if (row_border_flag)
     {
#ifdef THREEDIM
       for (slice = 0; slice < recur_nslices; slice++)
        for (row = 0; row < (params.seam_size/2); row++)
         for (col = 0; col < recur_ncols; col++)
           row_border_index_data[col + slice*ncols + row*nslices*ncols] =
                                recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
#else
        for (row = 0; row < (params.seam_size/2); row++)
         for (col = 0; col < recur_ncols; col++)
           row_border_index_data[col + row*ncols] =
                                recur_row_border_index_data[col + row*recur_ncols];
#endif
     } // if (row_border_flag)
#ifdef THREEDIM
     if (slice_border_flag)
     {
       for (slice = 0; slice < (params.seam_size/2); slice++)
        for (row = 0; row < recur_nrows; row++)
         for (col = 0; col < recur_ncols; col++)
           slice_border_index_data[col + row*ncols + slice*nrows*ncols] =
                                  recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
     } // if (slice_border_flag)
#endif

     unsigned int short_buf_size = 0, int_buf_size = 0, float_buf_size = 0;
     short unsigned int short_factor = 1, int_factor = 2, float_factor = 1;
     if (col_border_flag)
     {
#ifdef THREEDIM
       short_buf_size = short_factor*params.seam_size*recur_nrows*recur_nslices;
       int_buf_size = int_factor*params.seam_size*recur_nrows*recur_nslices;
       float_buf_size = float_factor*params.seam_size*recur_nrows*recur_nslices;
#else
       short_buf_size = short_factor*params.seam_size*recur_nrows;
       int_buf_size = int_factor*params.seam_size*recur_nrows;
       float_buf_size = float_factor*params.seam_size*recur_nrows;
#endif
     } // if (col_border_flag)
     if (row_border_flag)
     {
#ifdef THREEDIM
       short_buf_size += short_factor*params.seam_size*recur_ncols*recur_nslices;
       int_buf_size += int_factor*params.seam_size*recur_ncols*recur_nslices;
       float_buf_size += float_factor*params.seam_size*recur_ncols*recur_nslices;
#else
       short_buf_size += short_factor*params.seam_size*recur_ncols;
       int_buf_size += int_factor*params.seam_size*recur_ncols;
       float_buf_size += float_factor*params.seam_size*recur_ncols;
#endif
     } // if (row_border_flag)
#ifdef THREEDIM
     if (slice_border_flag)
     {
       short_buf_size += short_factor*params.seam_size*recur_ncols*recur_nrows;
       int_buf_size += int_factor*params.seam_size*recur_ncols*recur_nrows;
       float_buf_size += float_factor*params.seam_size*recur_ncols*recur_nrows;
     } // if (slice_border_flag)
#endif
     check_buf_size(0,short_buf_size,int_buf_size,float_buf_size,0,temp_data);

     unsigned int short_buf_position, int_buf_position, float_buf_position;
   // Receive border_index_data information from the parallel recur_tasks
     int index, border_index_data_tag = 106;
     min_section = section + stride;
     unsigned int proc_section = 1;
#ifdef TIME_IT
     float end_time, elapsed_time;
#endif
#else // PARALLEL
     min_section = section;
     unsigned int proc_section = 0;
#endif // !PARALLEL

     int col_section, row_section, nb_col_sections, nb_row_sections;
#ifdef THREEDIM
     int slice_section, nb_slice_sections;
#endif
     nb_col_sections = 1;
     if (col_flag)
       nb_col_sections = 2;
     nb_row_sections = 1;
     if (row_flag)
       nb_row_sections = 2;
#ifdef THREEDIM
     nb_slice_sections = 1;
     if (slice_flag)
       nb_slice_sections = 2;
#endif

     recur_section = section;
#ifdef THREEDIM
     for (slice_section = 0; slice_section < nb_slice_sections; slice_section++)
#endif
      for (row_section = 0; row_section < nb_row_sections; row_section++)
       for (col_section = 0; col_section < nb_col_sections; col_section++)
       {
         proc_section = col_section;
         if (params.nb_dimensions > 1)
           proc_section += 2*row_section;
#ifdef THREEDIM
         if (params.nb_dimensions > 2)
           proc_section += 2*2*slice_section;
#endif

         if (recur_section >= min_section)
         {
#ifdef PARALLEL
           if (params.debug > 2)
             params.log_fs << "Waiting for border_index_data information from task " << recur_section << endl;
#ifdef TIME_IT
           end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
           elapsed_time = end_time - temp_data.start_time;
           if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
           temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
           MPI::COMM_WORLD.Recv(&short_buf_size, 1, MPI::UNSIGNED, recur_section, border_index_data_tag);
           end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
           elapsed_time = end_time - temp_data.start_time;
           if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
           temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
           MPI::COMM_WORLD.Recv(temp_data.short_buffer, short_buf_size, MPI::UNSIGNED_SHORT, recur_section, border_index_data_tag);
           MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, recur_section, border_index_data_tag);
           if (params.edge_image_flag)
             MPI::COMM_WORLD.Recv(temp_data.float_buffer, float_buf_size, MPI::FLOAT, recur_section, border_index_data_tag);
#ifdef TIME_IT
           end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
           elapsed_time = end_time - temp_data.start_time;
           if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
           temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
           short_buf_position = int_buf_position = float_buf_position = 0;
           if (col_border_flag)
           {
#ifdef THREEDIM
             for (index = 0; index < (params.seam_size*recur_nrows*recur_nslices); ++index)
#else
             for (index = 0; index < (params.seam_size*recur_nrows); ++index)
#endif
               recur_col_border_index_data[index].set_data(false,temp_data,short_buf_position,int_buf_position,float_buf_position);
           } // if (col_border_flag)
           if (row_border_flag)
           {
#ifdef THREEDIM
             for (index = 0; index < (params.seam_size*recur_ncols*recur_nslices); ++index)
#else
             for (index = 0; index < (params.seam_size*recur_ncols); ++index)
#endif
               recur_row_border_index_data[index].set_data(false,temp_data,short_buf_position,int_buf_position,float_buf_position);
           } //if (row_border_flag)
#ifdef THREEDIM
           if (slice_border_flag)
           {
             for (index = 0; index < (params.seam_size*recur_ncols*recur_nrows); ++index)
               recur_slice_border_index_data[index].set_data(false,temp_data,short_buf_position,int_buf_position,float_buf_position);
           } // if (slice_border_flag)
#endif
#else // !PARALLEL
         // In the serial case, process the specified data section
           if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
             restore_pixel_data(recur_section,pixel_data,temp_data);
#ifdef THREEDIM
           get_border_index_data((recur_level+1),recur_section,border_flag,pixel_data,
                                 recur_ncols,recur_nrows,recur_nslices,
                                 recur_col_border_index_data,recur_row_border_index_data,
                                 recur_slice_border_index_data,temp_data);
#else
           get_border_index_data((recur_level+1),recur_section,border_flag,pixel_data,
                                 recur_ncols,recur_nrows,
                                 recur_col_border_index_data,recur_row_border_index_data,
                                 temp_data);
#endif
#endif // !PARALLEL
           switch(proc_section)
           {
#ifdef THREEDIM
             case 0:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)
                            col_border_index_data[row + slice*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + slice*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } //if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = 0; slice < (params.seam_size/2); slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + row*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } // if (slice_border_flag)
                      break;
             case 1:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + slice*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + slice*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } //if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = 0; slice < (params.seam_size/2); slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + recur_ncols + row*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } //if (row_border_flag)
                      break;
             case 2:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)    
                            col_border_index_data[row + recur_nrows + slice*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + slice*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } //if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = 0; slice < (params.seam_size/2); slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + (row+recur_nrows)*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } //if (row_border_flag)
                      break;
             case 3:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + recur_nrows + slice*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + slice*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } //if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = 0; slice < (params.seam_size/2); slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + recur_ncols + (row+recur_nrows)*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } //if (row_border_flag)
                      break;
             case 4:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)
                            col_border_index_data[row + (slice+recur_nslices)*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + (slice+recur_nslices)*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } //if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = (params.seam_size/2); slice < params.seam_size; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + row*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } //if (row_border_flag)
                      break;
             case 5:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + (slice+recur_nslices)*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + (slice+recur_nslices)*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } //if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = (params.seam_size/2); slice < params.seam_size; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + recur_ncols + row*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } //if (row_border_flag)
                      break;
             case 6:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)
                            col_border_index_data[row + recur_nrows + (slice+recur_nslices)*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + (slice+recur_nslices)*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } //if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = (params.seam_size/2); slice < params.seam_size; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + (row+recur_nrows)*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } //if (row_border_flag)
                      break;
             case 7:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + recur_nrows + (slice+recur_nslices)*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + (slice+recur_nslices)*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } //if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = (params.seam_size/2); slice < params.seam_size; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + recur_ncols + (row+recur_nrows)*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } //if (row_border_flag)
                      break;
#else
             case 0:  if (col_border_flag)
                      {
                        for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)
                            col_border_index_data[row + col*nrows] =
                                                 recur_col_border_index_data[row + col*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + row*ncols] =
                                                 recur_row_border_index_data[col + row*recur_ncols];
                      } //if (row_border_flag)
                      break;
             case 1:  if (col_border_flag)
                      {
                        for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + col*nrows] =
                                                 recur_col_border_index_data[row + col*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + row*ncols] =
                                                 recur_row_border_index_data[col + row*recur_ncols];
                      } //if (row_border_flag)
                      break;
             case 2:  if (col_border_flag)
                      {
                        for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)
                            col_border_index_data[row + recur_nrows + col*nrows] =
                                                 recur_col_border_index_data[row + col*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + row*ncols] =
                                                 recur_row_border_index_data[col + row*recur_ncols];
                      } //if (row_border_flag)
                      break;
             case 3:  if (col_border_flag)
                      {
                        for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + recur_nrows + col*nrows] =
                                                 recur_col_border_index_data[row + col*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + row*ncols] =
                                                 recur_row_border_index_data[col + row*recur_ncols];
                      } //if (row_border_flag)
                      break;
#endif
           }
         } // if (recur_section >= min_section))
         recur_section += stride;
       } // for (row_section = 0; row_section < nb_row_sections; row_section++)
          //  for (col_section = 0; col_section < nb_col_sections; col_section++)
   }
#ifdef DEBUG
   if (params.debug > 3)
   {
     if (col_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P), col_border_index_data:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < params.seam_size; col++)
           {
#ifdef THREEDIM
              params.log_fs << col_border_index_data[row + slice*nrows + col*nslices*nrows].get_region_class_label( ) << " ";
#else
              params.log_fs << col_border_index_data[row + col*nrows].get_region_class_label( ) << " ";
#endif
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (col_border_flag)
     if (row_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P), row_border_index_data:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
        for (row = 0; row < params.seam_size; row++)
        {
         for (col = 0; col < ncols; col++)
         {
#ifdef THREEDIM
            params.log_fs << row_border_index_data[col + slice*ncols + row*nslices*ncols].get_region_class_label( ) << " ";
#else
            params.log_fs << row_border_index_data[col + row*ncols].get_region_class_label( ) << " ";
#endif
         }
         params.log_fs << endl;
        }
        params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (row_border_flag)
#ifdef THREEDIM
     if (slice_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P), slice_border_index_data:" << endl << endl;
       for (slice = 0; slice < params.seam_size; slice++)
       {
        for (row = 0; row < nrows; row++)
        {
         for (col = 0; col < ncols; col++)
         {
            params.log_fs << slice_border_index_data[col + row*ncols + slice*nrows*ncols].get_region_class_label( ) << " ";
         }
         params.log_fs << endl;
        }
        params.log_fs << endl;
       }
     } // if (slice_border_flag)
#endif
   }
   if (params.debug > 2)
   {
     params.log_fs << endl << "Exiting get_border_index_data(P-V1)" << endl;
   }
#endif
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: get_border_index_data (regular version)
|
|       Purpose: Gets the col_border_index_data, row_border_index_data and slice_border_index_data values
|                from spatial_data
|
|         Input: recur_level      (Current recursive level)
|                section          (Section or window processed by this call to get_border_index_data)
|                border_flag      (Border selection flag)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                ncols            (Current number of columns in data being processed)
|                nrows            (Current number of rows in data being processed)
|                nslices          (Current number of slices in data being processed)
|
|        Output: col_border_index_data   (Index class information from the processing window column border)
|                row_border_index_data   (Index class information from the processing window row border)
|                slice_border_index_data (Index class information from the processing window slice border)
|
|         Other:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|          Date: October 6, 2003
| Modifications: (See comments for first instance of get_border_index_data.)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                            const unsigned char& border_flag, Spatial& spatial_data,
                            const int& ncols, const int& nrows, const int& nslices,
                            vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                            vector<Index>& slice_border_index_data)
#else
 void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                            const unsigned char& border_flag, Spatial& spatial_data,
                            const int& ncols, const int& nrows,
                            vector<Index>& col_border_index_data, vector<Index>& row_border_index_data)
#endif
 {
   unsigned int pixel_index;
   int col, row;
#ifdef THREEDIM
   int slice;
#endif

  // Set recursive image sizes
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

  // Set border_flags
   bool col_border_flag, row_border_flag;
#ifdef THREEDIM
   bool slice_border_flag;
#endif

#ifdef THREEDIM
   set_recur_flags(border_flag,col_border_flag,row_border_flag,slice_border_flag);
#else
   set_recur_flags(border_flag,col_border_flag,row_border_flag);
#endif

#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Calling get_border_index_data (V2) for section = " << section << " with ncols = " << ncols;
#ifdef THREEDIM
     params.log_fs << ", nrows = " << nrows << " and nslices = " << nslices << endl << endl;
#else
     params.log_fs << " and nrows = " << nrows << endl << endl;
#endif
     if (col_border_flag)
       params.log_fs << "Requesting col_border_index_data" << endl;
     if (row_border_flag)
       params.log_fs << "Requesting row_border_index_data" << endl;
#ifdef THREEDIM
     if (slice_border_flag)
       params.log_fs << "Requesting slice_border_index_data" << endl;
#endif
   }
#endif
   if (col_border_flag)
   {
    // Set values for col_border_index_data
#ifdef THREEDIM
     for (slice = 0; slice < nslices; slice++)
     {
#endif
       for (row = 0; row < nrows; row++)
       {
         for (col = 0; col < (params.seam_size/2); col++)
         {
#ifdef THREEDIM
           pixel_index = col + row*ncols + slice*nrows*ncols;
           col_border_index_data[row + slice*nrows + col*nslices*nrows].set_data(section,spatial_data,pixel_index);
#else
           pixel_index = col + row*ncols;
           col_border_index_data[row + col*nrows].set_data(section,spatial_data,pixel_index);
#endif
         }
         for (col = (params.seam_size/2); col < params.seam_size; col++)
         {
#ifdef THREEDIM
           pixel_index = (col+ ncols-params.seam_size) + row*ncols + slice*nrows*ncols;
           col_border_index_data[row + slice*nrows + col*nslices*nrows].set_data(section,spatial_data,pixel_index);
#else
           pixel_index = (col+ ncols-params.seam_size) + row*ncols;
           col_border_index_data[row + col*nrows].set_data(section,spatial_data,pixel_index);
#endif
         }
       }
#ifdef THREEDIM
     }
#endif
   } // if (col_border_flag)
   if (row_border_flag)
   {
  // Set values for row_border_index_data
#ifdef THREEDIM
     for (slice = 0; slice < nslices; slice++)
     {
#endif
       for (row = 0; row < (params.seam_size/2); row++)
       {
         for (col = 0; col < ncols; col++)
         {
#ifdef THREEDIM
           pixel_index = col + row*ncols + slice*nrows*ncols;
           row_border_index_data[col + slice*ncols + row*nslices*ncols].set_data(section,spatial_data,pixel_index);
#else
           pixel_index = col + row*ncols;
           row_border_index_data[col + row*ncols].set_data(section,spatial_data,pixel_index);
#endif
         }
       }
       for (row = (params.seam_size/2); row < params.seam_size; row++)
       {
         for (col = 0; col < ncols; col++)
         {
#ifdef THREEDIM
           pixel_index = col + (row+nrows-params.seam_size)*ncols + slice*nrows*ncols;
           row_border_index_data[col + slice*ncols + row*nslices*ncols].set_data(section,spatial_data,pixel_index);
#else
           pixel_index = col + (row+nrows-params.seam_size)*ncols;
           row_border_index_data[col + row*ncols].set_data(section,spatial_data,pixel_index);
#endif
         }
       }
#ifdef THREEDIM
     }
#endif
   } // if (row_border_flag)
#ifdef THREEDIM
   if (slice_border_flag)
   {
  // Set values for row_border_index_data
     for (slice = 0; slice < (params.seam_size/2); slice++)
     {
       for (row = 0; row < nrows; row++)
       {
         for (col = 0; col < ncols; col++)
         {
           pixel_index = col + row*ncols + slice*nrows*ncols;
           slice_border_index_data[col + row*ncols + slice*nrows*ncols].set_data(section,spatial_data,pixel_index);
         }
       }
     }
     for (slice = (params.seam_size/2); slice < params.seam_size; slice++)
     {
       for (row = 0; row < nrows; row++)
       {
         for (col = 0; col < ncols; col++)
         {
           pixel_index = col + row*ncols + (slice+nslices-params.seam_size)*nrows*ncols;
           slice_border_index_data[col + row*ncols + slice*nrows*ncols].set_data(section,spatial_data,pixel_index);
         }
       }
     }
   } // if (slice_border_flag)
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     int value;
     if (col_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(V2), col_border_index_data.region_class_label:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < params.seam_size; col++)
           {
#ifdef THREEDIM
             value = col_border_index_data[row + slice*nrows + col*nslices*nrows].get_region_class_label( );
#else
             value = col_border_index_data[row + col*nrows].get_region_class_label( );
#endif
             if (value < 10)
               params.log_fs << "0" << value << " ";
             else
               params.log_fs << value << " ";
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     }
     if (row_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(V2), row_border_index_data.region_class_label:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < params.seam_size; row++)
         {
           for (col = 0; col < ncols; col++)
           {
#ifdef THREEDIM
             value = row_border_index_data[col + slice*ncols + row*nslices*ncols].get_region_class_label( );
#else
             value = row_border_index_data[col + row*ncols].get_region_class_label( );
#endif
             if (value < 10)
               params.log_fs << "0" << value << " ";
             else
               params.log_fs << value << " ";
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     }
#ifdef THREEDIM
     if (slice_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(V2), slice_border_index_data.region_class_label:" << endl << endl;
       for (slice = 0; slice < params.seam_size; slice++)
       {
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < ncols; col++)
           {
             value = slice_border_index_data[col + row*ncols + slice*nrows*ncols].get_region_class_label( );
             if (value < 10)
               params.log_fs << "0" << value << " ";
             else
               params.log_fs << value << " ";
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
       }
     }
#endif
     if (col_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(V2), col_border_index_data.boundary_map:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < params.seam_size; col++)
           {
#ifdef THREEDIM
             value = (int) col_border_index_data[row + slice*nrows + col*nslices*nrows].get_boundary_map( );
#else
             value = (int) col_border_index_data[row + col*nrows].get_boundary_map( );
#endif
             if (value < 10)
               params.log_fs << "0" << value << " ";
             else
               params.log_fs << value << " ";
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     }
     if (row_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(V2), row_border_index_data.boundary_map:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < params.seam_size; row++)
         {
           for (col = 0; col < ncols; col++)
           {
#ifdef THREEDIM
             value = (int) row_border_index_data[col + slice*ncols + row*nslices*ncols].get_boundary_map( );
#else
             value = (int) row_border_index_data[col + row*ncols].get_boundary_map( );
#endif
             if (value < 10)
               params.log_fs << "0" << value << " ";
             else
               params.log_fs << value << " ";
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     }
#ifdef THREEDIM
     if (slice_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(V2), slice_border_index_data.boundary_map:" << endl << endl;
       for (slice = 0; slice < params.seam_size; slice++)
       {
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < ncols; col++)
           {
             value = (int) slice_border_index_data[col + row*ncols + slice*nrows*ncols].get_boundary_map( );
             if (value < 10)
               params.log_fs << "0" << value << " ";
             else
               params.log_fs << value << " ";
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
       }
     }
#endif
   }
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting get_border_index_data (V2)" << endl << endl;
   }
#endif

   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: get_border_index_data (recursive version)
|
|       Purpose: Gets the col_border_index_data, row_border_index_data and slice_border_index_data values
|                from spatial_data
|
|         Input: recur_level      (Current recursive level)
|                section          (Section or window processed by this call to get_border_index_data)
|                border_flag      (Border selection flag)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                ncols            (Current number of columns in data being processed)
|                nrows            (Current number of rows in data being processed)
|                nslices          (Current number of slices in data being processed)
|                
|        Output: col_border_index_data (index_class information from the processing window column border)
|                row_border_index_data (Index class information from the processing window row border)
|                slice_border_index_data (Index class information from the processing window slice border)
|
|         Other: temp_data    (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: October 7, 2003.
| Modifications: (See comments for first instance of get_border_index_data.)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                            const unsigned char& border_flag, Spatial& spatial_data,
                            const int& ncols, const int& nrows, const int& nslices,
                            vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                            vector<Index>& slice_border_index_data, Temp& temp_data)
#else
 void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                            const unsigned char& border_flag, Spatial& spatial_data,
                            const int& ncols, const int& nrows,
                            vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                            Temp& temp_data)
#endif
 {
   int col, row;
#ifdef THREEDIM
   int slice;
#endif

  // Set recursive image sizes
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

  // Set border_flags
   bool col_border_flag, row_border_flag;
#ifdef THREEDIM
   bool slice_border_flag;
#endif

#ifdef THREEDIM
   set_recur_flags(border_flag,col_border_flag,row_border_flag,slice_border_flag);
#else
   set_recur_flags(border_flag,col_border_flag,row_border_flag);
#endif

#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Entering get_border_index_data(P-V2), ncols = " << ncols;
#ifdef THREEDIM
     params.log_fs << ", nrows = " << nrows << " and nslices = " << nslices << endl << "at recur_level = " << recur_level << endl;
#else
     params.log_fs << " and nrows = " << nrows << endl << "at recur_level = " << recur_level << endl;
#endif
   }
#endif

   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of get_border_index_data can be called.
#ifdef THREEDIM
     get_border_index_data(recur_level,section,border_flag,spatial_data,ncols,nrows,nslices,
                           col_border_index_data,row_border_index_data,slice_border_index_data);
#else
     get_border_index_data(recur_level,section,border_flag,spatial_data,ncols,nrows,
                           col_border_index_data,row_border_index_data);
#endif
   }
   else // if (recur_level < (params.onb_levels-1))
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // get_border_index_data must be called recursively.
     unsigned int recur_index_data_size = 1;
     if (col_border_flag)
#ifdef THREEDIM
       recur_index_data_size = recur_nrows*recur_nslices*params.seam_size;
#else
       recur_index_data_size = recur_nrows*params.seam_size;
#endif
     vector<Index> recur_col_border_index_data(recur_index_data_size);
     recur_index_data_size = 1;
     if (row_border_flag)
#ifdef THREEDIM
       recur_index_data_size = recur_ncols*recur_nslices*params.seam_size;
#else
       recur_index_data_size = recur_ncols*params.seam_size;
#endif
     vector<Index> recur_row_border_index_data(recur_index_data_size);
#ifdef THREEDIM
     recur_index_data_size = 1;
     if (slice_border_flag)
       recur_index_data_size = recur_ncols*recur_nrows*params.seam_size;
     vector<Index> recur_slice_border_index_data(recur_index_data_size);
#endif

     int stride, nb_sections;
     set_stride_sections(recur_level,stride,nb_sections);
#ifdef PARALLEL
     unsigned int value = border_flag;
#ifdef THREEDIM
   // Send request to the parallel recursive tasks
     parallel_recur_requests((short unsigned int) 9,recur_level,value,ncols,nrows,nslices,
                             0,0,0,temp_data);
   // Process current task's data section
     get_border_index_data((recur_level+1),section,border_flag,spatial_data,recur_ncols,recur_nrows,recur_nslices,
                           recur_col_border_index_data,recur_row_border_index_data,
                           recur_slice_border_index_data,temp_data);
#else
   // Send request to the parallel recursive tasks
     parallel_recur_requests((short unsigned int) 9,recur_level,value,ncols,nrows,
                             0,0,0,temp_data);
   // Process current task's data section
     get_border_index_data((recur_level+1),section,border_flag,spatial_data,recur_ncols,recur_nrows,
                           recur_col_border_index_data,recur_row_border_index_data,
                           temp_data);
#endif

     if (col_border_flag)
     {
#ifdef THREEDIM
       for (slice = 0; slice < recur_nslices; slice++)
        for (row = 0; row < recur_nrows; row++)
         for (col = 0; col < (params.seam_size/2); col++)
           col_border_index_data[row + slice*nrows + col*nslices*nrows] =
                                recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
#else
       for (row = 0; row < recur_nrows; row++)
         for (col = 0; col < (params.seam_size/2); col++)
           col_border_index_data[row + col*nrows] =
                                recur_col_border_index_data[row + col*recur_nrows];
#endif
     }
     if (row_border_flag)
     {
#ifdef THREEDIM
       for (slice = 0; slice < recur_nslices; slice++)
        for (row = 0; row < (params.seam_size/2); row++)
         for (col = 0; col < recur_ncols; col++)
           row_border_index_data[col + slice*ncols + row*nslices*ncols] =
                                recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
#else
       for (row = 0; row < (params.seam_size/2); row++)
         for (col = 0; col < recur_ncols; col++)
           row_border_index_data[col + row*ncols] =
                                recur_row_border_index_data[col + row*recur_ncols];
#endif
     }
#ifdef THREEDIM
     if (slice_border_flag)
     {
       for (slice = 0; slice < (params.seam_size/2); slice++)
        for (row = 0; row < recur_nrows; row++)
         for (col = 0; col < recur_ncols; col++)
           slice_border_index_data[col + row*ncols + slice*nrows*ncols] =
                                  recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
     }
#endif
     unsigned int short_buf_size = 0, int_buf_size = 0;
     short unsigned int short_factor = 2, int_factor = 3;
     if (col_border_flag)
     {
#ifdef THREEDIM
       short_buf_size = short_factor*params.seam_size*recur_nrows*recur_nslices;
       int_buf_size = int_factor*params.seam_size*recur_nrows*recur_nslices;
#else
       short_buf_size = short_factor*params.seam_size*recur_nrows;
       int_buf_size = int_factor*params.seam_size*recur_nrows;
#endif
     }
     if (row_border_flag)
     {
#ifdef THREEDIM
       short_buf_size += short_factor*params.seam_size*recur_ncols*recur_nslices;
       int_buf_size += int_factor*params.seam_size*recur_ncols*recur_nslices;
#else
       short_buf_size += short_factor*params.seam_size*recur_ncols;
       int_buf_size += int_factor*params.seam_size*recur_ncols;
#endif
     }
#ifdef THREEDIM
     if (slice_border_flag)
     {
       short_buf_size += short_factor*params.seam_size*recur_ncols*recur_nrows;
       int_buf_size += int_factor*params.seam_size*recur_ncols*recur_nrows;
     }
#endif
     check_buf_size(0,short_buf_size,int_buf_size,0,0,temp_data);

     unsigned int short_buf_position, int_buf_position, float_buf_position;
   // Receive border_index_data information from the parallel recur_tasks
     int index, border_index_data_tag2 = 109;
     int min_section = section + stride;
     unsigned int proc_section = 1;
#ifdef TIME_IT
     float end_time, elapsed_time;
#endif
#else
     int min_section = section;
     unsigned int proc_section = 0;
#endif

     int col_section, row_section, nb_col_sections, nb_row_sections;
#ifdef THREEDIM
     int slice_section, nb_slice_sections;
#endif
     nb_col_sections = 1;
     if (col_flag)
       nb_col_sections = 2;
     nb_row_sections = 1;
     if (row_flag)
       nb_row_sections = 2;
#ifdef THREEDIM
     nb_slice_sections = 1;
     if (slice_flag)
       nb_slice_sections = 2;
#endif

     int recur_section = section;
#ifdef THREEDIM
     for (slice_section = 0; slice_section < nb_slice_sections; slice_section++)
#endif
      for (row_section = 0; row_section < nb_row_sections; row_section++)
       for (col_section = 0; col_section < nb_col_sections; col_section++)
       {
         proc_section = col_section;
         if (params.nb_dimensions > 1)
           proc_section += 2*row_section;
#ifdef THREEDIM
         if (params.nb_dimensions > 2)
           proc_section += 2*2*slice_section;
#endif

         if (recur_section >= min_section)
         {
#ifdef PARALLEL
           if (params.debug > 3)
             params.log_fs << "Waiting for border_index_data information from task " << recur_section << endl;
#ifdef TIME_IT
           end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
           elapsed_time = end_time - temp_data.start_time;
           if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
           temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
           MPI::COMM_WORLD.Recv(&short_buf_size, 1, MPI::UNSIGNED, recur_section, border_index_data_tag2);
           end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
           elapsed_time = end_time - temp_data.start_time;
           if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
           temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
           MPI::COMM_WORLD.Recv(temp_data.short_buffer, short_buf_size, MPI::UNSIGNED_SHORT, recur_section, border_index_data_tag2);
           MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, recur_section, border_index_data_tag2);
#ifdef TIME_IT
           end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
           elapsed_time = end_time - temp_data.start_time;
           if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
           temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
#endif
           short_buf_position = int_buf_position = float_buf_position = 0;
           if (col_border_flag)
           {
#ifdef THREEDIM
             for (index = 0; index < (params.seam_size*recur_nrows*recur_nslices); ++index)
#else
             for (index = 0; index < (params.seam_size*recur_nrows); ++index)
#endif
               recur_col_border_index_data[index].set_data(true,temp_data,short_buf_position,int_buf_position,float_buf_position);
           } // if (col_border_flag)
           if (row_border_flag)
           {
#ifdef THREEDIM
             for (index = 0; index < (params.seam_size*recur_ncols*recur_nslices); ++index)
#else
             for (index = 0; index < (params.seam_size*recur_ncols); ++index)
#endif
               recur_row_border_index_data[index].set_data(true,temp_data,short_buf_position,int_buf_position,float_buf_position);
           } // if (row_border_flag)
#ifdef THREEDIM
           if (slice_border_flag)
           {
             for (index = 0; index < (params.seam_size*recur_ncols*recur_nrows); ++index)
               recur_slice_border_index_data[index].set_data(true,temp_data,short_buf_position,int_buf_position,float_buf_position);
           } // if (slice_border_flag)
#endif
#else // !PARALLEL
         // In the serial case, process the specified data section
           if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
           {
             spatial_data.restore_region_class_label_map(recur_section);
             if (params.region_nb_objects_flag) // Added this check May 23, 2011
               spatial_data.restore_region_object_label_map(recur_section);
             spatial_data.restore_boundary_map(recur_section);
           }
#ifdef THREEDIM
           get_border_index_data((recur_level+1),recur_section,border_flag,spatial_data,recur_ncols,recur_nrows,recur_nslices,
                                 recur_col_border_index_data,recur_row_border_index_data,
                                 recur_slice_border_index_data,temp_data);
#else
           get_border_index_data((recur_level+1),recur_section,border_flag,spatial_data,recur_ncols,recur_nrows,
                                 recur_col_border_index_data,recur_row_border_index_data,
                                 temp_data);
#endif
#endif // !PARALLEL
           switch(proc_section)
           {
#ifdef THREEDIM
             case 0:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)
                            col_border_index_data[row + slice*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + slice*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } // if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = 0; slice < (params.seam_size/2); slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + row*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } // if (slice_border_flag)
                      break;
             case 1:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + slice*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + slice*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } // if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = 0; slice < (params.seam_size/2); slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + recur_ncols + row*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } // if (slice_border_flag)
                      break;
             case 2:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)
                            col_border_index_data[row + recur_nrows + slice*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + slice*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } // if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = 0; slice < (params.seam_size/2); slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + (row+recur_nrows)*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } // if (slice_border_flag)
                      break;
             case 3:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + recur_nrows + slice*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + slice*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } // if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = 0; slice < (params.seam_size/2); slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + recur_ncols + (row+recur_nrows)*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } // if (slice_border_flag)
                      break;
             case 4:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)
                            col_border_index_data[row + (slice+recur_nslices)*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + (slice+recur_nslices)*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } // if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = (params.seam_size/2); slice < params.seam_size; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + row*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } // if (slice_border_flag)
                      break;
             case 5:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + (slice+recur_nslices)*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + (slice+recur_nslices)*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } // if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = (params.seam_size/2); slice < params.seam_size; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + recur_ncols + row*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } // if (slice_border_flag)
                      break;
             case 6:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)
                            col_border_index_data[row + recur_nrows + (slice+recur_nslices)*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + (slice+recur_nslices)*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } // if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = (params.seam_size/2); slice < params.seam_size; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + (row+recur_nrows)*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } // if (slice_border_flag)
                      break;
             case 7:  if (col_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + recur_nrows + (slice+recur_nslices)*nrows + col*nslices*nrows] =
                                                 recur_col_border_index_data[row + slice*recur_nrows + col*recur_nslices*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (slice = 0; slice < recur_nslices; slice++)
                         for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + (slice+recur_nslices)*ncols + row*nslices*ncols] =
                                                 recur_row_border_index_data[col + slice*recur_ncols + row*recur_nslices*recur_ncols];
                      } // if (row_border_flag)
                      if (slice_border_flag)
                      {
                        for (slice = (params.seam_size/2); slice < params.seam_size; slice++)
                         for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < recur_ncols; col++)
                            slice_border_index_data[col + recur_ncols + (row+recur_nrows)*ncols + slice*nrows*ncols] =
                                                   recur_slice_border_index_data[col + row*recur_ncols + slice*recur_nrows*recur_ncols];
                      } // if (slice_border_flag)
                      break;
#else
             case 0:  if (col_border_flag)
                      {
                        for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)
                            col_border_index_data[row + col*nrows] =
                                                 recur_col_border_index_data[row + col*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + row*ncols] =
                                                 recur_row_border_index_data[col + row*recur_ncols];
                      } // if (row_border_flag)
                      break;
             case 1:  if (col_border_flag)
                      {
                        for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + col*nrows] =
                                                 recur_col_border_index_data[row + col*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (row = 0; row < (params.seam_size/2); row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + row*ncols] =
                                                 recur_row_border_index_data[col + row*recur_ncols];
                      } // if (row_border_flag)
                      break;
             case 2:  if (col_border_flag)
                      {
                        for (row = 0; row < recur_nrows; row++)
                          for (col = 0; col < (params.seam_size/2); col++)
                            col_border_index_data[row + recur_nrows + col*nrows] =
                                                 recur_col_border_index_data[row + col*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + row*ncols] =
                                                 recur_row_border_index_data[col + row*recur_ncols];
                      } // if (row_border_flag)
                      break;
             case 3:  if (col_border_flag)
                      {
                        for (row = 0; row < recur_nrows; row++)
                          for (col = (params.seam_size/2); col < params.seam_size; col++)
                            col_border_index_data[row + recur_nrows + col*nrows] =
                                                 recur_col_border_index_data[row + col*recur_nrows];
                      } // if (col_border_flag)
                      if (row_border_flag)
                      {
                        for (row = (params.seam_size/2); row < params.seam_size; row++)
                          for (col = 0; col < recur_ncols; col++)
                            row_border_index_data[col + recur_ncols + row*ncols] =
                                                 recur_row_border_index_data[col + row*recur_ncols];
                      } // if (row_border_flag)
                      break;
#endif
           }
         } // if (recur_section >= min_section)
         recur_section += stride;
       } // for (row_section = 0; row_section < nb_row_sections; row_section++)
         //  for (col_section = 0; col_section < nb_col_sections; col_section++)
   }

#ifdef DEBUG
   if (params.debug > 3)
   {
     if (col_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P-V2), col_border_index_data.region_label:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < params.seam_size; col++)
           {
#ifdef THREEDIM
             params.log_fs << col_border_index_data[row + slice*nrows + col*nslices*nrows].get_region_class_label( ) << " ";
#else
             params.log_fs << col_border_index_data[row + col*nrows].get_region_class_label( ) << " ";
#endif
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (col_border_flag)
     if (row_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P-V2), row_border_index_data.region_label" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < params.seam_size; row++)
         {
           for (col = 0; col < ncols; col++)
           {
#ifdef THREEDIM
             params.log_fs << row_border_index_data[col + slice*ncols + row*nslices*ncols].get_region_class_label( ) << " ";
#else
             params.log_fs << row_border_index_data[col + row*ncols].get_region_class_label( ) << " ";
#endif
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (row_border_flag)
#ifdef THREEDIM
     if (slice_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P-V2), slice_border_index_data.region_label:" << endl << endl;
       for (slice = 0; slice < params.seam_size; slice++)
       {
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < ncols; col++)
           {
             params.log_fs << slice_border_index_data[col + row*ncols + slice*nrows*ncols].get_region_class_label( ) << " ";
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
       }
     } // if (slice_border_flag)
#endif
     if (params.region_objects_flag)
     {
       if (col_border_flag)
       {
         params.log_fs << endl << "Exiting get_border_index_data(P-V2), col_border_index_data.region_object_label:" << endl << endl;
#ifdef THREEDIM
         for (slice = 0; slice < nslices; slice++)
         {
#endif
           for (row = 0; row < nrows; row++)
           {
             for (col = 0; col < params.seam_size; col++)
             {
#ifdef THREEDIM
               params.log_fs << col_border_index_data[row + slice*nrows + col*nslices*nrows].get_region_object_label( ) << " ";
#else
               params.log_fs << col_border_index_data[row + col*nrows].get_region_object_label( ) << " ";
#endif
             }
             params.log_fs << endl;
           }
           params.log_fs << endl;
#ifdef THREEDIM
         }
#endif
       } // if (col_border_flag)
       if (row_border_flag)
       {
         params.log_fs << endl << "Exiting get_border_index_data(P-V2), row_border_index_data.region_object_label" << endl << endl;
#ifdef THREEDIM
         for (slice = 0; slice < nslices; slice++)
         {
#endif
           for (row = 0; row < params.seam_size; row++)
           {
             for (col = 0; col < ncols; col++)
             {
#ifdef THREEDIM
               params.log_fs << row_border_index_data[col + slice*ncols + row*nslices*ncols].get_region_object_label( ) << " ";
#else
               params.log_fs << row_border_index_data[col + row*ncols].get_region_object_label( ) << " ";
#endif
             }
             params.log_fs << endl;
           }
           params.log_fs << endl;
#ifdef THREEDIM
         }
#endif
       } // if (row_border_flag)
#ifdef THREEDIM
       if (slice_border_flag)
       {
         params.log_fs << endl << "Exiting get_border_index_data(P-V2), slice_border_index_data.region_object_label:" << endl << endl;
         for (slice = 0; slice < params.seam_size; slice++)
         {
           for (row = 0; row < nrows; row++)
           {
             for (col = 0; col < ncols; col++)
             {
               params.log_fs << slice_border_index_data[col + row*ncols + slice*nrows*ncols].get_region_object_label( ) << " ";
             }
             params.log_fs << endl;
           }
           params.log_fs << endl;
         }
       } // if (slice_border_flag)
#endif
     } // if (params.object_labels_map_flag)
     if (col_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P-V2), col_border_index_data.boundary_map:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < params.seam_size; col++)
           {
#ifdef THREEDIM
             params.log_fs << col_border_index_data[row + slice*nrows + col*nslices*nrows].get_boundary_map( ) << " ";
#else
             params.log_fs << col_border_index_data[row + col*nrows].get_boundary_map( ) << " ";
#endif
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (col_border_flag)
     if (row_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P-V2), row_border_index_data.boundary_map" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
        for (row = 0; row < params.seam_size; row++)
        {
         for (col = 0; col < ncols; col++)
         {
#ifdef THREEDIM
            params.log_fs << row_border_index_data[col + slice*ncols + row*nslices*ncols].get_boundary_map( ) << " ";
#else
            params.log_fs << row_border_index_data[col + row*ncols].get_boundary_map( ) << " ";
#endif
         }
         params.log_fs << endl;
        }
        params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (row_border_flag)
#ifdef THREEDIM
     if (slice_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P-V2), slice_border_index_data.boundary_map:" << endl << endl;
       for (slice = 0; slice < params.seam_size; slice++)
       {
        for (row = 0; row < nrows; row++)
        {
         for (col = 0; col < ncols; col++)
         {
            params.log_fs << slice_border_index_data[col + row*ncols + slice*nrows*ncols].get_boundary_map( ) << " ";
         }
         params.log_fs << endl;
        }
        params.log_fs << endl;
       }
     } // if (slice_border_flag)
#endif
     if (col_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P-V2), col_border_index_data.section:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < params.seam_size; col++)
           {
#ifdef THREEDIM
              params.log_fs << col_border_index_data[row + slice*nrows + col*nslices*nrows].get_section( ) << " ";
#else
              params.log_fs << col_border_index_data[row + col*nrows].get_section( ) << " ";
#endif
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (col_border_flag)
     if (row_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P-V2), row_border_index_data.section" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
        for (row = 0; row < params.seam_size; row++)
        {
         for (col = 0; col < ncols; col++)
         {
#ifdef THREEDIM
            params.log_fs << row_border_index_data[col + slice*ncols + row*nslices*ncols].get_section( ) << " ";
#else
            params.log_fs << row_border_index_data[col + row*ncols].get_section( ) << " ";
#endif
         }
         params.log_fs << endl;
        }
        params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     } // if (row_border_flag)
#ifdef THREEDIM
     if (slice_border_flag)
     {
       params.log_fs << endl << "Exiting get_border_index_data(P-V2), slice_border_index_data.section:" << endl << endl;
       for (slice = 0; slice < params.seam_size; slice++)
       {
        for (row = 0; row < nrows; row++)
        {
         for (col = 0; col < ncols; col++)
         {
            params.log_fs << slice_border_index_data[col + row*ncols + slice*nrows*ncols].get_section( ) << " ";
         }
         params.log_fs << endl;
        }
        params.log_fs << endl;
       }
     } // if (slice_border_flag)
#endif
   }
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting get_border_index_data(P-V2)" << endl;
   }
#endif
   return;
 }

} // namespace HSEGTilton
