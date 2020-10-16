/*-----------------------------------------------------------
|
|  Routine Name: set_offsets - Set array position offsets.
|
|       Purpose: Calculates array position offsets for data sections
|                processed by the various recur_tasks for the parallel case
|                and various sections for the serial case.
|
|         Input: recur_level      (Lowest recursive level at which this section (or task) is active)
|                stride           (Number of data sections (or tasks) covered by each child section (or task))
|                nb_sections      (Number of data sections (or tasks) covered by this section (or task))
|                section          (Section (or task) for which offsets are being computed)
|                init_col_offset  (Initial column offset)
|                init_row_offset  (Initial row offset)
|                init_slice_offset(Initial slice offset)
|                ncols            (Number of columns in covered data section)
|                nrows            (Number of rows in covered data section)
|                nslices          (Number of slices in covered data section)
|
|        Output: col_offset       (Column offset for covered data section)
|                row_offset       (Row offset for covered data section)
|                slice_offset     (Slice offset for covered data section)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: September 17, 2002.
| Modifications: November 5, 2003 - Modified Params structure and made class member variables all private
|                June 15, 2005 - Added temporary file I/O for faster processing of large data sets
|                October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                May 12, 2008 - Modified to be a friend function for the globally defined Params class object
|                April 1, 2011 - Utilized recursion_mask in order to help equalize the dimension sizes at the deepest level of recursion.
|
------------------------------------------------------------*/

#include "params.h"

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef THREEDIM
  void set_offsets(const int& recur_level, const int& nb_sections, const int& section,
                   const int& init_col_offset, const int& init_row_offset, const int& init_slice_offset,
                   const int& ncols, const int& nrows, const int& nslices)
#else
  void set_offsets(const int& recur_level, const int& nb_sections, const int& section,
                   const int& init_col_offset, const int& init_row_offset,
                   const int& ncols, const int& nrows)
#endif
  {

    if (nb_sections > 1)
    {
      int next_recur_level = recur_level + 1;
      int next_nb_sections, nb_strides, stride, recur_section;
      int col_section, row_section, nb_col_sections, nb_row_sections;
      int recur_ncols = ncols;
      int recur_nrows = nrows;
      bool col_flag, row_flag;
#ifdef THREEDIM
      int slice_section, nb_slice_sections;
      int recur_nslices = nslices;
      bool slice_flag;
#endif
#ifdef THREEDIM
      nb_strides = set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag,slice_flag);
#else
      nb_strides = set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag);
#endif
      stride = nb_sections/nb_strides;
      next_nb_sections = nb_sections;
      nb_col_sections = 1;
      if (col_flag)
      {
        recur_ncols /= 2;
        nb_col_sections = 2;
        next_nb_sections /= 2;
      }
      nb_row_sections = 1;
      if (row_flag)
      {
        recur_nrows /= 2;
        nb_row_sections = 2;
        next_nb_sections /= 2;
      }
#ifdef THREEDIM
      nb_slice_sections = 1;
      if (slice_flag)
      {
        recur_nslices /= 2;
        nb_slice_sections = 2;
        next_nb_sections /= 2;
      }
#endif
      recur_section = section;
#ifdef THREEDIM
      for (slice_section = 0; slice_section < nb_slice_sections; slice_section++)
#endif
       for (row_section = 0; row_section < nb_row_sections; row_section++)
        for (col_section = 0; col_section < nb_col_sections; col_section++)
        {
#ifdef THREEDIM
          set_offsets(next_recur_level,next_nb_sections,recur_section,
                      (init_col_offset + col_section*recur_ncols),
                      (init_row_offset + row_section*recur_nrows),
                      (init_slice_offset + slice_section*recur_nslices),
                      recur_ncols,recur_nrows,recur_nslices);
#else
          set_offsets(next_recur_level,next_nb_sections,recur_section,
                      (init_col_offset + col_section*recur_ncols),
                      (init_row_offset + row_section*recur_nrows),
                      recur_ncols,recur_nrows);
#endif
          recur_section += stride;
        }
    }
    else
    {
#ifdef PARALLEL
      params.col_offset[section - params.myid] = init_col_offset;
      if (params.row_offset_flag > 0)
        params.row_offset[section - params.myid] = init_row_offset;
#ifdef THREEDIM
      if (params.slice_offset_flag > 0)
        params.slice_offset[section - params.myid] = init_slice_offset;
#endif
#else
      params.col_offset[section] = init_col_offset;
      if (params.row_offset_flag > 0)
        params.row_offset[section] = init_row_offset;
#ifdef THREEDIM
      if (params.slice_offset_flag > 0)
        params.slice_offset[section] = init_slice_offset;
#endif
#endif
    }

    return;
  }

/*-----------------------------------------------------------
|
|  Routine Name: set_strides_sections - Set stride and nb_sections (nb_tasks for PARALLEL).
|
|       Purpose: Calculates values for stride and nb_sections.
|
|         Input: recur_level      (Current recursive level)
|                nb_recur_levels  (Number of recursive levels at deepest level of recursion considered)
|
|        Output: stride           (Spacing between data sections (or tasks) covered by each child task)
|                nb_sections      (Total number of data sections (or tasks) covered by this task)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: April 7, 2011
| Modifications: 
|
------------------------------------------------------------*/

  void set_stride_sections(const int& recur_level, int& stride, int& nb_sections)
  {
    int nb_recur_levels;
#ifdef PARALLEL
    nb_recur_levels = params.inb_levels;
#else
    nb_recur_levels = params.ionb_levels;
#endif
    int level, nb_strides = 1;
    stride = 0;
    nb_sections = 1;
    if (recur_level < (nb_recur_levels-1))
    {
      for (level = (nb_recur_levels-2); level >= recur_level; level--)
      {
        nb_strides = 2;
        if ((params.recur_mask_flags[level] == 3) || (params.recur_mask_flags[level] == 5) || (params.recur_mask_flags[level] == 6))
          nb_strides = 4;
        if (params.recur_mask_flags[level] == 7)
          nb_strides = 8;
        nb_sections *= nb_strides;
      }
      stride = nb_sections/nb_strides;
    }

    return;
  }

#ifdef THREEDIM
  int set_recur_flags(const unsigned char& recur_mask_flag, bool& col_flag, bool& row_flag, bool& slice_flag)
#else
  int set_recur_flags(const unsigned char& recur_mask_flag, bool& col_flag, bool& row_flag)
#endif
  {
  // recur_mask_flag designations: 1 => col only, 2 => row only, 3=> col & row,
  //                               4=> slice only, 5=> col & slice, 6 => row & slice, 7 => col, row & slice.
    col_flag = ((recur_mask_flag == 1) || (recur_mask_flag == 3) || (recur_mask_flag == 5) || (recur_mask_flag == 7));
    row_flag = ((recur_mask_flag == 2) || (recur_mask_flag == 3) || (recur_mask_flag == 6) || (recur_mask_flag == 7));
#ifdef THREEDIM
    slice_flag = ((recur_mask_flag == 4) || (recur_mask_flag == 5) || (recur_mask_flag == 6) || (recur_mask_flag == 7));
#endif
    int nb_strides = 1;
    if (col_flag)
      nb_strides *= 2;
    if (row_flag)
      nb_strides *= 2;
#ifdef THREEDIM
    if (slice_flag)
      nb_strides *= 2;
#endif

    return nb_strides;
  }

} // namespace HSEGTilton
