/*-----------------------------------------------------------
|
|  Routine Name: boundary_map (regular version)
|
|       Purpose: Initialize or update boundary_map values
|
|         Input: ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                nslevels         (Number of segmentation levels saved)
|                spatial_data     (Class which holds information pertaining to the spatial data)
|                
|        Output:
|
|       Returns:
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: November 27, 2003
| Modifications: March 2, 2004 - Changed boundary_flag to boundary_map
|                August 9, 2004 - Boundary map definition modified to make image edge pixels region boundary pixels
|                June 2, 2005 - Added temporary file I/O for faster processing of large data sets
|                November 23, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                December 15, 2005 - Undid August 9, 2004 modification.
|                January 30, 2006 - Combined boundary_map_init and boundary_map_update into one function: boundary_map
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|                February 24, 2012 - Upgraded boundary_map from unsigned char to short unsigned int
|
------------------------------------------------------------*/

#include "spatial.h"
#include <params/params.h>
#include <index/index.h>
#ifdef TIME_IT
#include <ctime>
#endif

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef THREEDIM
  void boundary_map(const int& ncols, const int& nrows,
                    const int& nslices, const short unsigned int& nslevels,
                    Spatial& spatial_data)
#else
  void boundary_map(const int& ncols, const int& nrows,
                    const short unsigned int& nslevels,
                    Spatial& spatial_data)
#endif
  {
   int col, row;
#ifdef THREEDIM
   int slice;
#endif
   unsigned int pixel_index;
   if (nslevels == 0)
   {
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
         spatial_data.boundary_map[pixel_index] = 0;
       }
#ifdef THREEDIM
     }
#endif
   }

   short unsigned int nbdir, maxnbdir = params.maxnbdir;
   switch (params.nb_dimensions)
   {
     case 1:  if (maxnbdir > 2)
                maxnbdir = 2;
              break;
     case 2:  if (maxnbdir > 8)
                maxnbdir = 8;
              break;
#ifdef THREEDIM
     case 3:  if (maxnbdir > 26)
                maxnbdir = 26;
              break;
#endif
   }
   unsigned int region_label, nbregion_label;
   unsigned int nbpixel_index=0;
   int nbcol, nbrow;
#ifdef THREEDIM
   int nbslice;
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
       region_label = spatial_data.region_class_label_map[pixel_index];
       if (region_label > 0)
       {
         for (nbdir = 0; nbdir < maxnbdir; ++nbdir)
         {
           nbregion_label = 0;
#ifdef THREEDIM
           find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
           if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
               (nbslice < nslices) && (nbrow<nrows) && (nbcol<ncols))
           {
             nbpixel_index = nbcol + nbrow*ncols + nbslice*nrows*ncols;
#else
           find_nghbr(col,row,nbdir,nbcol,nbrow);
           if ((nbrow>=0) && (nbcol>=0) &&
               (nbrow<nrows) && (nbcol<ncols))
           {
             nbpixel_index = nbcol + nbrow*ncols;
#endif
             nbregion_label = spatial_data.region_class_label_map[nbpixel_index];
//             if ((nbregion_label > 0) && (nbregion_label != region_label))
             if (nbregion_label != region_label)
               spatial_data.boundary_map[pixel_index] = (short unsigned int) (nslevels+1);
           }
         } // for (nbdir = 0; nbdir < params.maxnbdir; ++nbdir)
       }
     }
#ifdef THREEDIM
   }
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After updating boundary_map values, dump of boundary_map" << endl << endl;
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
         params.log_fs << " " << spatial_data.boundary_map[pixel_index];
       }
       params.log_fs << endl;
      }
      params.log_fs << endl;
#ifdef THREEDIM
     }
#endif
   }
#endif
   return;
  }

/*-----------------------------------------------------------
|
|  Routine Name: boundary_map (recursive version)
|
|       Purpose: Initialize or update boundary_map values
|
|         Input: recur_level      (Current recursive level of this task)
|                section          (Section or window processed by this call to lrhseg)
|                ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                nslevels         (Number of segmentation levels saved)
|                spatial_data     (Class which holds information pertaining to the spatial data)
|                
|        Output:
|
|         Other: temp_data        (buffers used in communications between parallel tasks)

|       Returns:
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: November 27, 2003
| Modifications: March 2, 2004 - Changed boundary_flag to boundary_map
|                August 9, 2004 - Boundary map definition modified to make image edge pixels region boundary pixels
|                June 2, 2005 - Added temporary file I/O for faster processing of large data sets
|                November 23, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                December 15, 2005 - Undid August 9, 2004 modification.
|                January 30, 2006 - Combined boundary_map_init and boundary_map_update into one function: boundary_map
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|
------------------------------------------------------------*/
#ifdef THREEDIM
  void boundary_map(const short unsigned int& recur_level, const short unsigned int& section, 
                    const int& ncols, const int& nrows, const int& nslices, const short unsigned int& nslevels,
                    Spatial& spatial_data, Temp& temp_data)
#else
  void boundary_map(const short unsigned int& recur_level, const short unsigned int& section,
                    const int& ncols, const int& nrows, const short unsigned int& nslevels,
                    Spatial& spatial_data, Temp& temp_data)
#endif
  {
   if (params.debug > 3)
   {
     params.log_fs << "Entering boundary_map, recur_level = " << recur_level << ", section = " << section;
#ifdef THREEDIM
     params.log_fs << "ncols = " << ncols << ", nrows = " << nrows << ", nslices = " << nslices;
#else
     params.log_fs << "ncols = " << ncols << ", nrows = " << nrows;
#endif
     params.log_fs << ", and nslevels = " << nslevels << endl;
   }

   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of boundary_map can be called.
#ifdef THREEDIM
     boundary_map(ncols,nrows,nslices,nslevels,spatial_data);
#else
     boundary_map(ncols,nrows,nslevels,spatial_data);
#endif
   }
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // boundary_map must be called recursively.
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
     parallel_recur_requests((short unsigned int) 18, recur_level, (unsigned int) nslevels, 
                             ncols, nrows, nslices, 0, 0, 0, temp_data);
   // Process current task's data section
     boundary_map((recur_level+1), section, recur_ncols, recur_nrows, recur_nslices, nslevels, spatial_data, temp_data);
#else
   // Send request to the parallel recur_tasks
     parallel_recur_requests((short unsigned int) 18, recur_level, (unsigned int) nslevels, 
                             ncols, nrows, 0, 0, 0, temp_data);
   // Process current task's data section
     boundary_map((recur_level+1), section, recur_ncols, recur_nrows, nslevels, spatial_data, temp_data);
#endif
    // Receive confirmation of boundary map initialization from the recursive tasks
     int boundary_map_tag = 118;
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
       if (params.debug > 3)
         params.log_fs << "Waiting for boundary map update confirmation from task " << recur_section << endl;
#ifdef TIME_IT
       end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
       temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
       MPI::COMM_WORLD.Recv(&value, 1, MPI::UNSIGNED, recur_section, boundary_map_tag);
#ifdef TIME_IT
       end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
       temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
       if (params.debug > 3)
        {
         params.log_fs << "boundary map update completed on task " << recur_section << endl;
       }
#else // !PARALLEL
     // In the serial case, process the specified data section
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
       {
         spatial_data.restore_region_class_label_map(recur_section);
         if (nslevels > 0)
           spatial_data.restore_boundary_map(recur_section);
       }
#ifdef THREEDIM
       boundary_map((recur_level+1), recur_section, recur_ncols, recur_nrows, recur_nslices, nslevels, spatial_data, temp_data);
#else
       boundary_map((recur_level+1), recur_section, recur_ncols, recur_nrows, nslevels, spatial_data, temp_data);
#endif
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
         spatial_data.save_boundary_map(recur_section);
#endif // !PARALLEL
     }

    // Set the boundary map values along the processing window seams
    // seam flag designations: 1 => col only, 2 => row only, 3=> col & row,
    // 4=> slice only, 5=> col & slice, 6 => row & slice, 7 => col, row & slice
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

     get_seam_index_data(recur_level,section,params.recur_mask_flags[recur_level],spatial_data,ncols,nrows,nslices,
                         col_seam_index_data,row_seam_index_data,
                         slice_seam_index_data,temp_data);
     boundary_map_seams(recur_level,section,nb_sections,params.recur_mask_flags[recur_level],nslevels,spatial_data,
                        ncols,nrows,nslices,col_seam_index_data,
                        row_seam_index_data,slice_seam_index_data,temp_data);
#else
     get_seam_index_data(recur_level,section,params.recur_mask_flags[recur_level],spatial_data,ncols,nrows,
                         col_seam_index_data,row_seam_index_data,
                         temp_data);
     boundary_map_seams(recur_level,section,nb_sections,params.recur_mask_flags[recur_level],nslevels,spatial_data,
                        ncols,nrows,col_seam_index_data,
                        row_seam_index_data,temp_data);
#endif
   }
   return;
  }

/*-----------------------------------------------------------
|
|  Routine Name: boundary_map_seams
|
|       Purpose: Updates the boundary_map along processing window seams
|
|         Input: recur_level      (Current recursive level of this task)
|                section          (Section or window processed by this call to lrhseg)
|                nb_sections      (Number of data sections (or tasks) covered by this task)
|                seam_flag        (Seam selection flag)
|                nslevels         (Number of segmentation levels saved)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                col_seam_index_data   (Index class information from the processing window column seam)
|                row_seam_index_data   (Index class information from the processing window row seam)
|                slice_seam_index_data (Index class information from the processing window slice seam)
|                
|        Output:
|
|       Returns:
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: October 10, 2003
| Modifications: (See comments for first instance of boundary_map)
|
------------------------------------------------------------*/
#ifdef THREEDIM
  void boundary_map_seams(const short unsigned int& recur_level, const short unsigned int& section,
                          const short unsigned int& nb_sections, const unsigned char& seam_flag,
                          const short unsigned int& nslevels, Spatial& spatial_data,
                          const int& ncols, const int& nrows, const int& nslices,
                          vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                          vector<Index>& slice_seam_index_data, Temp& temp_data)
#else
  void boundary_map_seams(const short unsigned int& recur_level, const short unsigned int& section,
                          const short unsigned int& nb_sections, const unsigned char& seam_flag,
                          const short unsigned int& nslevels, Spatial& spatial_data,
                          const int& ncols, const int& nrows,
                          vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                          Temp& temp_data)
#endif
  {
    bool col_seam_flag, row_seam_flag;
    int col, row;
#ifdef THREEDIM
    int slice;
    bool slice_seam_flag;
#endif

#ifdef THREEDIM
    set_recur_flags(seam_flag,col_seam_flag,row_seam_flag,slice_seam_flag);
#else
    set_recur_flags(seam_flag,col_seam_flag,row_seam_flag);
#endif

#ifdef DEBUG
    if (params.debug > 2)
    {
     if (nslevels == 0)
       params.log_fs << "Performing boundary map initialization along the processing window seam " << endl;
     else
       params.log_fs << "Performing boundary map update along the processing window seam " << endl;
#ifdef THREEDIM
     params.log_fs << "with ncols = " << ncols << ", nrows = " << nrows << ", and nslices = " << nslices << endl;
#else
     params.log_fs << "with ncols = " << ncols << ", and nrows = " << nrows << endl;
#endif
     if (col_seam_flag)
       params.log_fs << "Using col_seam_index_data" << endl;
     if (row_seam_flag)
       params.log_fs << "Using row_seam_index_data" << endl;
#ifdef THREEDIM
     if (slice_seam_flag)
       params.log_fs << "Using slice_seam_index_data" << endl;
#endif
    }
    int value;
    if (params.debug > 3)
    {
      if (col_seam_flag)
      {
        params.log_fs << endl << "Before setting boundary_map along the seams, col_seam_index_data.region_label:" << endl << endl;
#ifdef THREEDIM
        for (slice = 0; slice < nslices; slice++)
        {
#endif
          for (row = 0; row < nrows; row++)
          {
            for (col = 0; col < params.seam_size; col++)
            {
#ifdef THREEDIM
              value = (int) col_seam_index_data[row + slice*nrows + col*nslices*nrows].get_region_class_label();
#else
              value = (int) col_seam_index_data[row + col*nrows].get_region_class_label();
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
      if (row_seam_flag)
      {
        params.log_fs << endl << "Before setting boundary_map along the seams, row_seam_index_data.region_label:" << endl << endl;
#ifdef THREEDIM
        for (slice = 0; slice < nslices; slice++)
        {
#endif
          for (row = 0; row < params.seam_size; row++)
          {
            for (col = 0; col < ncols; col++)
            {
#ifdef THREEDIM
              value = (int) row_seam_index_data[col + slice*ncols + row*nslices*ncols].get_region_class_label();
#else
              value = (int) row_seam_index_data[col + row*ncols].get_region_class_label();
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
      if (slice_seam_flag)
      {
        params.log_fs << endl << "Before setting boundary_map along the seams, slice_seam_index_data.region_label:" << endl << endl;
        for (slice = 0; slice < params.seam_size; slice++)
        {
          for (row = 0; row < nrows; row++)
          {
            for (col = 0; col < ncols; col++)
            {
              value = (int) slice_seam_index_data[col + row*ncols + slice*nrows*ncols].get_region_class_label();
              if (value < 10)
                params.log_fs << "0" << value << " ";
              else
                params.log_fs << value << " ";
            }
            params.log_fs << endl;
          }
          params.log_fs << endl;
        }
        params.log_fs << endl;
      }
#endif
      if (col_seam_flag)
      {
        params.log_fs << endl << "Before setting boundary_map along the seams, col_seam_index_data.boundary_map:" << endl << endl;
#ifdef THREEDIM
        for (slice = 0; slice < nslices; slice++)
        {
#endif
          for (row = 0; row < nrows; row++)
          {
            for (col = 0; col < params.seam_size; col++)
            {
#ifdef THREEDIM
              value = (int) col_seam_index_data[row + slice*nrows + col*nslices*nrows].get_boundary_map();
#else
              value = (int) col_seam_index_data[row + col*nrows].get_boundary_map();
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
      if (row_seam_flag)
      {
        params.log_fs << endl << "Before setting boundary_map along the seams, row_seam_index_data.boundary_map:" << endl << endl;
#ifdef THREEDIM
        for (slice = 0; slice < nslices; slice++)
        {
#endif
          for (row = 0; row < params.seam_size; row++)
          {
            for (col = 0; col < ncols; col++)
            {
#ifdef THREEDIM
              value = (int) row_seam_index_data[col + slice*ncols + row*nslices*ncols].get_boundary_map();
#else
              value = (int) row_seam_index_data[col + row*ncols].get_boundary_map();
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
      if (slice_seam_flag)
      {
        params.log_fs << endl << "Before setting boundary_map along the seams, slice_seam_index_data.boundary_map:" << endl << endl;
        for (slice = 0; slice < params.seam_size; slice++)
        {
          for (row = 0; row < nrows; row++)
          {
            for (col = 0; col < ncols; col++)
            {
              value = (int) slice_seam_index_data[col + row*ncols + slice*nrows*ncols].get_boundary_map();
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
      if (col_seam_flag)
      {
        params.log_fs << endl << "Before setting boundary_map along the seams, col_seam_index_data.section:" << endl << endl;
#ifdef THREEDIM
        for (slice = 0; slice < nslices; slice++)
        {
#endif
          for (row = 0; row < nrows; row++)
          {
            for (col = 0; col < params.seam_size; col++)
            {
#ifdef THREEDIM
              value = (int) col_seam_index_data[row + slice*nrows + col*nslices*nrows].get_section();
#else
              value = (int) col_seam_index_data[row + col*nrows].get_section();
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
      if (row_seam_flag)
      {
        params.log_fs << endl << "Before setting boundary_map along the seams, row_seam_index_data.section:" << endl << endl;
#ifdef THREEDIM
        for (slice = 0; slice < nslices; slice++)
        {
#endif
          for (row = 0; row < params.seam_size; row++)
          {
            for (col = 0; col < ncols; col++)
            {
#ifdef THREEDIM
              value = (int) row_seam_index_data[col + slice*ncols + row*nslices*ncols].get_section();
#else
              value = (int) row_seam_index_data[col + row*ncols].get_section();
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
      if (slice_seam_flag)
      {
        params.log_fs << endl << "Before setting boundary_map along the seams, slice_seam_index_data.section:" << endl << endl;
        for (slice = 0; slice < params.seam_size; slice++)
        {
          for (row = 0; row < nrows; row++)
          {
            for (col = 0; col < ncols; col++)
            {
              value = (int) slice_seam_index_data[col + row*ncols + slice*nrows*ncols].get_section();
              if (value < 10)
                params.log_fs << "0" << value << " ";
              else
                params.log_fs << value << " ";
            }
            params.log_fs << endl;
          }
          params.log_fs << endl;
        }
        params.log_fs << endl;
      }
#endif
    }
#endif
    short unsigned int nbdir, maxnbdir = params.maxnbdir;
    switch (params.nb_dimensions)
    {
      case 1:  if (maxnbdir > 2)
                 maxnbdir = 2;
               break;
      case 2:  if (maxnbdir > 8)
                 maxnbdir = 8;
               break;
#ifdef THREEDIM
      case 3:  if (maxnbdir > 26)
                 maxnbdir = 26;
               break;
#endif
    }
    unsigned int region_label, nbregion_label;
    unsigned int col_seam_index=0, col_seam_nbindex=0;
    unsigned int row_seam_index=0, row_seam_nbindex=0;
    int nbcol, nbrow;
#ifdef THREEDIM
    int nbslice;
    unsigned int slice_seam_index=0, slice_seam_nbindex=0;
#endif
    if (col_seam_flag)
    {
#ifdef THREEDIM
      for (slice = 0; slice < nslices; slice++)
      {
#endif
       for (row = 0; row < nrows; row++)
        for (col = 0; col < params.seam_size; col++)
        {
#ifdef THREEDIM
          col_seam_index = row + slice*nrows + col*nslices*nrows;
#else
          col_seam_index = row + col*nrows;
#endif
          region_label = col_seam_index_data[col_seam_index].get_region_class_label();
          if (region_label > 0)
          {
            for (nbdir = 0; nbdir < maxnbdir; nbdir++)
            {
              nbregion_label = 0;
#ifdef THREEDIM
              find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
              if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
                  (nbslice < nslices) && (nbrow<nrows) && (nbcol<params.seam_size))
              {
                col_seam_nbindex = nbrow + nbslice*nrows + nbcol*nslices*nrows;
#else
              find_nghbr(col,row,nbdir,nbcol,nbrow);
              if ((nbrow>=0) && (nbcol>=0) &&
                  (nbrow<nrows) && (nbcol<params.seam_size))
              {
                col_seam_nbindex = nbrow + nbcol*nrows;
#endif
                nbregion_label = col_seam_index_data[col_seam_nbindex].get_region_class_label();
//                if ((nbregion_label > 0) && (nbregion_label != region_label))
                if (nbregion_label != region_label)
                  col_seam_index_data[col_seam_index].set_boundary_map(nslevels+1);
              }
            }
          }
        }
#ifdef THREEDIM
      }
#endif
    } // if (col_seam_flag)
    if (row_seam_flag)
    {
   // Need to update middle section of row_seam_index_data from col_seam_index_data
#ifdef THREEDIM
      for (slice = 0; slice < nslices; slice++)
      {
#endif
       for (row = 0; row < params.seam_size; row++)
        for (col = 0; col < params.seam_size; col++)
        {
#ifdef THREEDIM
          col_seam_index = (row + nrows/2 - (params.seam_size/2)) + slice*nrows + col*nslices*nrows;
          row_seam_index = (col + ncols/2 - (params.seam_size/2)) + slice*ncols + row*nslices*ncols;
#else
          col_seam_index = (row + nrows/2 - (params.seam_size/2)) + col*nrows;
          row_seam_index = (col + ncols/2 - (params.seam_size/2)) + row*ncols;
#endif
          row_seam_index_data[row_seam_index].set_boundary_map(col_seam_index_data[col_seam_index].get_boundary_map());
        }
#ifdef THREEDIM
      }
      for (slice = 0; slice < nslices; slice++)
      {
#endif
       for (row = 0; row < params.seam_size; row++)
        for (col = 0; col < ncols; col++)
        {
#ifdef THREEDIM
          row_seam_index = col + slice*ncols + row*nslices*ncols;
#else
          row_seam_index = col + row*ncols;
#endif
          region_label = row_seam_index_data[row_seam_index].get_region_class_label();
          if (region_label > 0)
          {
            for (nbdir = 0; nbdir < maxnbdir; nbdir++)
            {
              nbregion_label = 0;
#ifdef THREEDIM
              find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
              if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
                  (nbslice<nslices) && (nbrow<params.seam_size) && (nbcol<ncols))
              {
                row_seam_nbindex = nbcol + nbslice*ncols + nbrow*nslices*ncols;
#else
              find_nghbr(col,row,nbdir,nbcol,nbrow);
              if ((nbrow>=0) && (nbcol>=0) &&
                  (nbrow<params.seam_size) && (nbcol<ncols))
              {
                row_seam_nbindex = nbcol + nbrow*ncols;
#endif
                nbregion_label = row_seam_index_data[row_seam_nbindex].get_region_class_label();
//                if ((nbregion_label > 0) && (nbregion_label != region_label))
                if (nbregion_label != region_label)
                  row_seam_index_data[row_seam_index].set_boundary_map(nslevels+1);
              }
            }
          }
        }
#ifdef THREEDIM
      }
#endif
    } // if (row_seam_flag)
#ifdef THREEDIM
    if (slice_seam_flag)
    {
   // Need to update middle section of slice_seam_index_data from col_seam_index_data and row_seam_index_data
      for (slice = 0; slice < params.seam_size; slice++)
       for (row = 0; row < nrows; row++)
        for (col = 0; col < params.seam_size; col++)
        {
          col_seam_index = row + (slice + nslices/2 - (params.seam_size/2))*nrows + col*nslices*nrows;
          slice_seam_index = (col + ncols/2 - (params.seam_size/2)) + row*ncols + slice*nrows*ncols;
          slice_seam_index_data[slice_seam_index].set_boundary_map(col_seam_index_data[col_seam_index].get_boundary_map());
        }
      for (slice = 0; slice < params.seam_size; slice++)
       for (row = 0; row < params.seam_size; row++)
        for (col = 0; col < ncols; col++)
        {
          row_seam_index = col + (slice + nslices/2 - (params.seam_size/2))*ncols + row*nslices*ncols;
          slice_seam_index = col + (row + nrows/2 - (params.seam_size/2))*ncols + slice*nrows*ncols;
          slice_seam_index_data[slice_seam_index].set_boundary_map(row_seam_index_data[row_seam_index].get_boundary_map());
        }
      for (slice = 0; slice < params.seam_size; slice++)
       for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          slice_seam_index = col + row*ncols + slice*nrows*ncols;
          region_label = slice_seam_index_data[slice_seam_index].get_region_class_label();
          if (region_label > 0)
          {
            for (nbdir = 0; nbdir < maxnbdir; nbdir++)
            {
              nbregion_label = 0;
              find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
              if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
                  (nbslice<params.seam_size) && (nbrow<nrows) && (nbcol<ncols))
              {
                slice_seam_nbindex = nbcol + nbrow*ncols + nbslice*nrows*ncols;
                nbregion_label = slice_seam_index_data[slice_seam_nbindex].get_region_class_label();
//                if ((nbregion_label > 0) && (nbregion_label != region_label))
                if (nbregion_label != region_label)
                  slice_seam_index_data[slice_seam_index].set_boundary_map(nslevels+1);
              }
            }
          }
        }
    } // if (slice_seam_flag)
#endif
#ifdef DEBUG
    if (params.debug > 3)
    {
      if (col_seam_flag)
      {
        params.log_fs << endl << "After setting boundary_map along the seams, col_seam_index_data.boundary_map:" << endl << endl;
#ifdef THREEDIM
        for (slice = 0; slice < nslices; slice++)
        {
#endif
          for (row = 0; row < nrows; row++)
          {
            for (col = 0; col < params.seam_size; col++)
            {
#ifdef THREEDIM
              value = (int) col_seam_index_data[row + slice*nrows + col*nslices*nrows].get_boundary_map();
#else
              value = (int) col_seam_index_data[row + col*nrows].get_boundary_map();
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
      if (row_seam_flag)
      {
        params.log_fs << endl << "After setting boundary_map along the seams, row_seam_index_data.boundary_map:" << endl << endl;
#ifdef THREEDIM
        for (slice = 0; slice < nslices; slice++)
        {
#endif
          for (row = 0; row < params.seam_size; row++)
          {
            for (col = 0; col < ncols; col++)
            {
#ifdef THREEDIM
              value = (int) row_seam_index_data[col + slice*ncols + row*nslices*ncols].get_boundary_map();
#else
              value = (int) row_seam_index_data[col + row*ncols].get_boundary_map();
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
      if (slice_seam_flag)
      {
        params.log_fs << endl << "After setting boundary_map along the seams, slice_seam_index_data.boundary_map:" << endl << endl;
        for (slice = 0; slice < params.seam_size; slice++)
        {
          for (row = 0; row < nrows; row++)
          {
            for (col = 0; col < ncols; col++)
            {
              value = (int) slice_seam_index_data[col + row*ncols + slice*nrows*ncols].get_boundary_map();
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
#endif
    unsigned int short_buf_size, int_buf_size;
#ifdef THREEDIM
    short_buf_size = int_buf_size = params.seam_size*(nslices*nrows + nslices*ncols + nrows*ncols);
#else // THREEDIM
    short_buf_size = int_buf_size = params.seam_size*(ncols + nrows);
#endif // THREEDIM
    check_buf_size(0,short_buf_size,int_buf_size,0,0,temp_data);
#ifdef PARALLEL
    bool section_flags[nb_sections];
#endif
    int min_section = section;
    int max_section = section + nb_sections;
    int onb_section;
    unsigned int index_index;
    for (onb_section = (max_section-1); onb_section >= min_section; --onb_section)
    {
#ifdef PARALLEL
      section_flags[onb_section - section] = false;
#endif
      short_buf_size = int_buf_size = 0;
      if (col_seam_flag)
      {
#ifdef THREEDIM
        for (slice = 0; slice < nslices; slice++)
        {
#endif
         for (row = 0; row < nrows; row++)
          for (col = 0; col < params.seam_size; col++)
          {
#ifdef THREEDIM
            index_index = row + slice*nrows + col*nslices*nrows;
#else
            index_index = row + col*nrows;
#endif
            if (col_seam_index_data[index_index].get_section() == onb_section)
            {
              temp_data.short_buffer[short_buf_size++] = col_seam_index_data[index_index].get_boundary_map();
              temp_data.int_buffer[int_buf_size++] = col_seam_index_data[index_index].get_pixel_index();
            }
          }
#ifdef THREEDIM
        }
#endif
      } // if (col_seam_flag)
      if (row_seam_flag)
      {
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
        for (row = 0; row < params.seam_size; row++)
         for (col = 0; col < ncols; col++)
         {
#ifdef THREEDIM
          index_index = col + slice*ncols + row*nslices*ncols;
#else
          index_index = col + row*ncols;
#endif
          if (row_seam_index_data[index_index].get_section() == onb_section)
          {
            temp_data.short_buffer[short_buf_size++] = row_seam_index_data[index_index].get_boundary_map();
            temp_data.int_buffer[int_buf_size++] = row_seam_index_data[index_index].get_pixel_index();
          }
        }
#ifdef THREEDIM
       }
#endif
      } // if (row_seam_flag)
#ifdef THREEDIM
      if (slice_seam_flag)
      {
       for (slice = 0; slice < params.seam_size; slice++)
        for (row = 0; row < nrows; row++)
         for (col = 0; col < ncols; col++)
         {
          index_index = col + row*ncols + slice*nrows*ncols;
          if (slice_seam_index_data[index_index].get_section() == onb_section)
          {
            temp_data.short_buffer[short_buf_size++] = slice_seam_index_data[index_index].get_boundary_map();
            temp_data.int_buffer[int_buf_size++] = slice_seam_index_data[index_index].get_pixel_index();
          }
         }
      } // if (slice_seam_flag)
#endif
      if (int_buf_size > 0)
      {
#ifdef PARALLEL
        section_flags[onb_section - section] = true;
        if (onb_section == section)
          spatial_data.set_boundary_map(nslevels, int_buf_size,
                                        temp_data);
        else
          parallel_request((short unsigned int) 19, onb_section,
                           nslevels, 0, short_buf_size, int_buf_size, 0, temp_data);
#else
        if (params.nb_sections > 1)
          spatial_data.restore_boundary_map(onb_section);
        spatial_data.set_boundary_map(nslevels, int_buf_size, temp_data);
        if (params.nb_sections > 1)
          spatial_data.save_boundary_map(onb_section);
#endif
      }
    }

#ifdef PARALLEL
    int set_boundary_map_tag = 119;
#ifdef TIME_IT
    float end_time, elapsed_time;
    end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
    elapsed_time = end_time - temp_data.start_time;
    if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
    temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
    for (onb_section = (min_section+1); onb_section < max_section; ++onb_section)
      if (section_flags[onb_section - section])
      {
        if (params.debug > 2)
          params.log_fs << "Waiting for confirmation of boundary_map update from onb_section = " << onb_section << endl;
        MPI::COMM_WORLD.Recv(&int_buf_size, 1, MPI::UNSIGNED, onb_section, set_boundary_map_tag);
        if (params.debug > 2)
          params.log_fs << "Received confirmation of boundary_map update from onb_section = " << onb_section << endl;
      }
#ifdef TIME_IT
    end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
    elapsed_time = end_time - temp_data.start_time;
    if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
    temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
#endif

    return;
  }
} // namespace HSEGTilton
