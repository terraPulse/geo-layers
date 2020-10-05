/*-----------------------------------------------------------
|
|  Routine Name: lrhseg
|
|       Purpose: Recursive call to lhseg
|
|         Input: region_label_offset (Offset to be added to region_label before exiting)
|                recur_level      (Current recursive level of this task)
|                section          (Section or window processed by this call to lrhseg)
|                ncols            (Current number of columns in data being processed)
|                nrows            (Current number of rows in data being processed)
|                nslices          (Current number of slices in data being processed)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                pixel_data       (Class which holds information pertaining to the pixels processed by this task)
|                
|        Output:
|
|         Other: region_classes   (Class which holds region related information)
|                global_nregions  (Current number of regions globally)
|                max_threshold    (Maximum merging threshold encountered)
|                temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: nregions         (Current number of regions for processed section(s))
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: on December 19, 2002.
| Modifications: January 29, 2003 - Added connected component labeling for (spclust_flag == false) case.
|                February 10, 2003 - Changed region_index to region_label
|                May 20, 2003 - Added option to initialize with Classical Region Growing.
|                June 9, 2003 - Changed certain lists to sets for efficiency.
|                August 27, 2003 - Reorganized program and added user controlled byteswapping
|                September 2, 2003 - Implemented Classical Region Growing for intermediate recursion levels.
|                September 24, 2003 - Eliminated use of the index_data array
|                November 7, 2003 - Modified Params structure and made class member variables all private
|                December 23, 2004 - Changed region label from short unsigned int to unsigned int
|                May 31, 2005 - Added temporary file I/O for faster processing of large data sets
|                October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                February 15, 2007 - Modified region initialization to allow option for initial fast merge process
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                September 5, 2008 - Modified lrhseg call vis-a-vis nregions & max_threshold
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 1, 2011 - Utilized recursion_mask in order to help equalize the dimension sizes at the deepest level of recursion.
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|		 September 18, 2013 - Eliminated the eliminate_artifacts function (based on splitting out and remerging pixels) 
|				      and replaced it with a new artifact_elimination function (based on utilizing edge information).
|                November 12, 2013 - Corrected computation of converge_nregions.
|                November 19, 2013 - Added optional call to region_classes_check function (for debugging).
|                February 7, 2014 - If the region_map_in labeling is complete, initialize from do_region_classes_init instead of first_merge_reg_grow.
|
------------------------------------------------------------*/

#include "hseg.h"
#include <params/params.h>
#include <spatial/spatial.h>
#include <pixel/pixel.h>
#include <region/region_class.h>
#include <index/index.h>
#include <iostream>
#include <algorithm>
#ifdef TIME_IT
#include <ctime>
#endif

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{
#ifdef THREEDIM
 unsigned int lrhseg(const unsigned int& region_label_offset,
                     const short unsigned int& recur_level, const short unsigned int& section,
                     const int& ncols, const int& nrows, const int& nslices, unsigned int& global_nregions,
                     double& max_threshold, Spatial& spatial_data, vector<Pixel>& pixel_data, 
                     vector<RegionClass>& region_classes, vector<RegionClass *>& nghbr_heap, 
                     vector<RegionClass *>& region_heap, Temp& temp_data)
#else
 unsigned int lrhseg(const unsigned int& region_label_offset,
                     const short unsigned int& recur_level, const short unsigned int& section,
                     const int& ncols, const int& nrows, unsigned int& global_nregions,
                     double& max_threshold, Spatial& spatial_data, vector<Pixel>& pixel_data, 
                     vector<RegionClass>& region_classes, vector<RegionClass *>& nghbr_heap, 
                     vector<RegionClass *>& region_heap, Temp& temp_data)
#endif
 {
   int row, col, pixel_ncols, pixel_nrows;
#ifdef THREEDIM
   int slice, pixel_nslices;
#endif
   unsigned int region_label, pixel_index, total_npixels;
#ifdef DEBUG
   unsigned int npixels;
#endif
#ifdef PARALLEL
#ifdef TIME_IT
   float end_time, elapsed_time;
#endif
#endif

   pixel_ncols = ncols;
   if (ncols > params.pixel_ncols)
     pixel_ncols = params.pixel_ncols;
   pixel_nrows = nrows;
   if (nrows > params.pixel_nrows)
     pixel_nrows = params.pixel_nrows;
#ifdef THREEDIM
   pixel_nslices = nslices;
   if (nslices > params.pixel_nslices)
     pixel_nslices = params.pixel_nslices;
#ifdef DEBUG
   npixels = pixel_ncols*pixel_nrows*pixel_nslices;
#endif
   total_npixels = ncols*nrows*nslices;
#else
#ifdef DEBUG
   npixels = pixel_ncols*pixel_nrows;
#endif
   total_npixels = ncols*nrows;
#endif
   unsigned int pixel_data_size = pixel_data.size( );

   if (params.debug > 1)
   {
     params.log_fs << endl << "Entering lrhseg with recur_level = " << recur_level;
#ifdef DEBUG
     params.log_fs << ", first_sec = " << section << ", npixels directly accessible by this task = " << npixels << "," << endl;
     params.log_fs << "total number of pixels this processing window = " << total_npixels << ", ";
     params.log_fs << "and region_label_offset = " << region_label_offset << "." << endl;
#else
     params.log_fs << endl << "with the total number of pixels this processing window = " << total_npixels << ", ";
     params.log_fs << endl;
#endif
   }

#ifndef PARALLEL
   if ((params.nb_sections > 1) && (recur_level == (params.inb_levels-1)))
     restore_pixel_data( section, pixel_data, temp_data);
#endif

   if (recur_level == (params.rnb_levels-1))
   {
     if (params.debug > 3)
     {
       params.log_fs << endl << "Entering lrhseg, dump of the pixel data:" << endl << endl;
       for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
         pixel_data[pixel_index].print(pixel_index);
     }
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "Entering lrhseg, dump of the region labeling of pixel_data:" << endl << endl;
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
             pixel_data[pixel_index].print_region_label();
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     }
     if (params.debug > 3)
     {
       params.log_fs << endl << "Entering lrhseg, dump of the pixel data region labels:" << endl << endl;
       for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
         if (pixel_data[pixel_index].get_region_label( ) != 0)
         {
           params.log_fs << "Element " << pixel_index << " is associated with region label ";
           pixel_data[pixel_index].print_region_label();
           params.log_fs << endl;
         }
     }
#endif // #ifdef DEBUG
   }

   unsigned int region_classes_size, max_region_label, nregions, init_nregions = 0;
   unsigned int compact_region_index, region_index;
   map<unsigned int,unsigned int> region_relabel_pairs;
   map<unsigned int,unsigned int>::iterator region_relabel_pair_iter;
   bool do_region_class_relabel_flag;

   if (recur_level == (params.rnb_levels-1))
   {
    // This is the deepest level of recursion.
    // Make the region_label numbering compact in pixel_data.
     nregions = 0;
     init_nregions = 0;
     if (params.complete_labeling_flag)
     {
#ifdef THREEDIM
       max_region_label = do_region_classes_init(ncols,nrows,nslices,pixel_data,region_classes);
#else
       max_region_label = do_region_classes_init(ncols,nrows,pixel_data,region_classes);
#endif
     // Count the number of regions.  If this matches max_region_label, then the labeling is already compact.
       region_classes_size = region_classes.size();
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         if (region_classes[region_index].get_active_flag())
           nregions++;
       if (nregions == max_region_label)
       {
         if (params.debug > 1)
           params.log_fs << "After initialization from region_map_in, the number of regions = " << nregions << endl;
       }
       else
       {
      // Need to make the region labeling compact.
         compact_region_index = 0;
         vector<RegionClass> compact_region_classes(nregions,RegionClass( ));
         region_classes_size = region_classes.size();
         for (region_index = 0; region_index < region_classes_size; ++region_index)
         {
           if ((region_classes[region_index].get_active_flag( )) &&
               (region_classes[region_index].get_npix( ) > 0))
           {
             compact_region_classes[compact_region_index++] = region_classes[region_index];
           }
           if (compact_region_index == nregions)
             break;
         }

      // Relabel region_classes as necessary to make the labeling compact.
         region_relabel_pairs.clear( );
         for (region_index = 0; region_index < nregions; ++region_index)
         {
           region_classes[region_index] = compact_region_classes[region_index];
           region_label = region_classes[region_index].get_label( );
           region_classes[region_index].set_label(region_index + region_label_offset + 1);
           region_classes[region_index].clear_best_merge( );
           if (region_label != region_classes[region_index].get_label( ))
             region_relabel_pairs.insert(make_pair(region_label,region_classes[region_index].get_label( )));
         }

         if (!region_relabel_pairs.empty( ))
         {
         // Use region_relabel_pairs to renumber the nghbrs_label_set for each region.
          for (region_index = 0; region_index < nregions; ++region_index)
            region_classes[region_index].nghbrs_label_set_renumber(region_relabel_pairs); 

       // Use region_relabel_pairs to renumber region_label in pixel_data.
          do_region_class_relabel(recur_level,section,region_relabel_pairs,pixel_data,temp_data);

       // region_classes is now compact.
          region_relabel_pairs.clear( );
         } // if (!region_relabel_pairs.empty( ))
         region_classes_size = region_classes.size( );
         for (region_index = nregions; region_index < region_classes_size; ++region_index)
         {
          region_classes[region_index].clear( );
          region_classes[region_index].set_label(region_index);
         }
         if (params.debug > 1)
           params.log_fs << "After initialization from region_map_in, and renumbering for compactness, the number of regions = " << nregions << endl;
       }
       init_nregions = nregions;
     } // if (params.complete_labeling_flag)
     else
     {
       do_region_class_relabel_flag = false;
       for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
       {
         if (pixel_data[pixel_index].get_mask())
         {
           region_label = pixel_data[pixel_index].get_region_label( );
           if (region_label == 0)
           {
             init_nregions++;
           }
           else
           {
             region_relabel_pair_iter = region_relabel_pairs.find(region_label);
             if (region_relabel_pair_iter != region_relabel_pairs.end( ))
             {
               region_label = (*region_relabel_pair_iter).second;
             }
             else
             {
               init_nregions++;
               region_relabel_pairs.insert(make_pair(region_label,++nregions));
               if (region_label != nregions)
                 do_region_class_relabel_flag = true;
               region_label = nregions;
             }
             pixel_data[pixel_index].set_region_label(region_label);
           }
         }
       }
       region_relabel_pairs.clear( );
       if (do_region_class_relabel_flag)
       {
         if (params.debug > 1)
           params.log_fs << endl << "After relabeling, nregions = " << nregions << endl;
#ifdef DEBUG
         if (params.debug > 3)
         {
           params.log_fs << endl << "After relabeling, dump of the pixel data region labels:" << endl << endl;
           for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
             if (pixel_data[pixel_index].get_region_label( ) != 0)
             {
               params.log_fs << "Element " << pixel_index << " is associated with region label ";
               pixel_data[pixel_index].print_region_label();
               params.log_fs << endl;
             }
         }
#endif
       }
       else
         if (params.debug > 2)
           params.log_fs << endl << "No relabeling required, nregions = " << nregions << endl;
       do_region_class_relabel_flag = false;

      // Initialize the region_classes incorporating optional first merge region growing
#ifdef THREEDIM
       first_merge_reg_grow(ncols,nrows,nslices,pixel_data,region_classes,nregions);
#else
       first_merge_reg_grow(ncols,nrows,pixel_data,region_classes,nregions);
#endif
       if (params.debug > 1)
         params.log_fs << "After initialization in first_merge_reg_grow, the number of regions = " << nregions << endl;
     } // else if (!params.complete_labeling_flag)
     region_classes_size = region_classes.size( );
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "After initialization in lrhseg, dump of the region data:" << endl << endl;
       for (region_index = 0; region_index < nregions; ++region_index)
         if (region_classes[region_index].get_npix() > 7)
           region_classes[region_index].print();
//         region_classes[region_index].print(region_classes);
       params.log_fs << endl << "Dump of the pixel data region labels:" << endl << endl;
       for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
         if (pixel_data[pixel_index].get_region_label( ) != 0)
         {
           params.log_fs << "Element " << pixel_index << " is associated with region label ";
           pixel_data[pixel_index].print_region_label();
           params.log_fs << endl;
         }
     }
#endif
   } // if (recur_level == (params.rnb_levels-1))

   double threshold = 0.0;
  // Declare arrays which will hold processing window seam information
   bool col_flag, row_flag;
#ifdef THREEDIM
   bool slice_flag;
#endif
#ifdef THREEDIM
   set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag,slice_flag);
#else
   set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag);
#endif
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
#endif

   int stride, nb_sections;
   set_stride_sections(recur_level,stride,nb_sections);

   if (recur_level < (params.rnb_levels-1))
   {
  // Need to set up and make a call to a deeper level of recursion
     short unsigned int recur_section = 0;
     unsigned int recur_region_index, recur_nregions;
     unsigned int recur_region_classes_size, class_label_offset = 0;
     unsigned int recur_pixel_index, recur_npixels;
     int recur_pixel_ncols = pixel_ncols;
     if (col_flag)
       recur_pixel_ncols /= 2;
     int recur_pixel_nrows = 1;
     if (params.nb_dimensions > 1)
     {
       recur_pixel_nrows = pixel_nrows;
       if (row_flag)
         recur_pixel_nrows /= 2;
     }
#ifdef THREEDIM
     int recur_pixel_nslices = 1;
     if (params.nb_dimensions > 2)
     {
       recur_pixel_nslices = pixel_nslices;
       if (slice_flag)
         recur_pixel_nslices /= 2;
     }
#endif
     if (pixel_ncols != ncols)
       recur_pixel_ncols = pixel_ncols;
     if (pixel_nrows != nrows)
       recur_pixel_nrows = pixel_nrows;
#ifdef THREEDIM
     if (pixel_nslices != nslices)
       recur_pixel_nslices = pixel_nslices;
     recur_npixels = recur_pixel_ncols*recur_pixel_nrows*recur_pixel_nslices;
#else
     recur_npixels = recur_pixel_ncols*recur_pixel_nrows;
#endif
     vector<Pixel> recur_pixel_data(recur_npixels,Pixel( ));
     vector<RegionClass> recur_region_classes(region_classes.size( ),RegionClass( ));

     int recur_ncols = ncols;
     if (col_flag)
       recur_ncols /= 2;
     int recur_nrows = 1;
     if (params.nb_dimensions > 1)
     {
       recur_nrows = nrows;
       if (row_flag)
         recur_nrows /= 2;
     }
#ifdef THREEDIM
     int recur_nslices = 1;
     if (params.nb_dimensions > 2)
     {
       recur_nslices = nslices;
       if (slice_flag)
         recur_nslices /= 2;
     }
#endif

     int col_section, row_section, nb_col_sections, nb_row_sections;
     int col_offset=0, row_offset=0;
#ifdef THREEDIM
     int slice_section, nb_slice_sections;
     int slice_offset=0;
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

     short unsigned int proc_section, next_recur_level = recur_level + 1;
     nregions = 0;
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
         switch(proc_section)
         {
           case 0:  col_offset = 0; row_offset = 0;
#ifdef THREEDIM
                    slice_offset = 0;
#endif
                    break;
           case 1:  col_offset = recur_ncols; row_offset = 0;
#ifdef THREEDIM
                    slice_offset = 0;
#endif
                    break;
           case 2:  col_offset = 0; row_offset = recur_nrows;
#ifdef THREEDIM
                    slice_offset = 0;
#endif
                    break;
           case 3:  col_offset = recur_ncols; row_offset = recur_nrows;
#ifdef THREEDIM
                    slice_offset = 0;
#endif
                    break;
#ifdef THREEDIM
           case 4:  col_offset = 0; row_offset = 0; slice_offset = recur_nslices;
                    break;
           case 5:  col_offset = recur_ncols; row_offset = 0; slice_offset = recur_nslices;
                    break;
           case 6:  col_offset = 0; row_offset = recur_nrows; slice_offset = recur_nslices;
                    break;
           case 7:  col_offset = recur_ncols; row_offset = recur_nrows; slice_offset = recur_nslices;
                    break;
#endif
         }
         class_label_offset = nregions;

         recur_region_classes_size = recur_region_classes.size( );
         for (recur_region_index = 0; recur_region_index < recur_region_classes_size; ++recur_region_index)
         {
           recur_region_classes[recur_region_index].clear( );
           recur_region_classes[recur_region_index].set_label(recur_region_index+1);
         }

#ifdef PARALLEL
         if ((recur_level >= (params.inb_levels-1)) || (recur_section == section))
         {
#else
           if (recur_level >= (params.ionb_levels-1))
           {
#endif
#ifdef THREEDIM
             for (slice = 0; slice < recur_pixel_nslices; ++slice)
             {
#endif
              for (row = 0; row < recur_pixel_nrows; ++row)
               for (col = 0; col < recur_pixel_ncols; ++col)
               {
#ifdef THREEDIM
                 pixel_index = (col+col_offset) + (row+row_offset)*pixel_ncols + (slice+slice_offset)*pixel_nrows*pixel_ncols;
                 recur_pixel_index = col + row*recur_pixel_ncols + slice*recur_pixel_nrows*recur_pixel_ncols;
#else
                 pixel_index = (col+col_offset) + (row+row_offset)*pixel_ncols;
                 recur_pixel_index = col + row*recur_pixel_ncols;
#endif
                 recur_pixel_data[recur_pixel_index] = pixel_data[pixel_index];
               }
#ifdef THREEDIM
             }
#endif
#ifndef PARALLEL
           }
#endif
           threshold = 0.0;  // Added July 22, 2013
#ifdef THREEDIM
           recur_nregions = lrhseg(class_label_offset,next_recur_level,recur_section,
                                   recur_ncols,recur_nrows,recur_nslices,
                                   global_nregions,threshold,spatial_data,recur_pixel_data,recur_region_classes,
                                   nghbr_heap,region_heap,temp_data);
#else
           recur_nregions = lrhseg(class_label_offset,next_recur_level,recur_section,
                                   recur_ncols,recur_nrows,
                                   global_nregions,threshold,spatial_data,recur_pixel_data,recur_region_classes,
                                   nghbr_heap,region_heap,temp_data);
#endif
           if (params.debug > 1)
           {
             params.log_fs << "Completed recursive section " << recur_section << " with threshold = " << threshold << endl;
           }
#ifdef PARALLEL
         }
         else
         {
           if (params.debug > 1)
           {
             params.log_fs << "Waiting for results from recursive task " << recur_section << endl;
           }
           recur_nregions = recur_receive(recur_section,next_recur_level,
                                          class_label_offset,recur_pixel_data,recur_region_classes,
                                          global_nregions,threshold,temp_data);
           if (params.debug > 1)
           {
             params.log_fs << "Received results from recursive task " << recur_section << " with threshold = " << threshold << endl;
           }
         }

         if ((recur_level >= (params.onb_levels-1)) || (recur_section == section))
         {
#else
           if (recur_level >= (params.ionb_levels-1))
           {
#endif
#ifdef THREEDIM
             for (slice = 0; slice < recur_pixel_nslices; ++slice)
             {
#endif
              for (row = 0; row < recur_pixel_nrows; ++row)
               for (col = 0; col < recur_pixel_ncols; ++col)
               {
#ifdef THREEDIM
                 pixel_index = (col+col_offset) + (row+row_offset)*pixel_ncols + (slice+slice_offset)*pixel_nrows*pixel_ncols;
                 recur_pixel_index = col + row*recur_pixel_ncols + slice*recur_pixel_nrows*recur_pixel_ncols;
#else
                 pixel_index = (col+col_offset) + (row+row_offset)*pixel_ncols;
                 recur_pixel_index = col + row*recur_pixel_ncols;
#endif
                 pixel_data[pixel_index] = recur_pixel_data[recur_pixel_index];
               }
#ifdef THREEDIM
             }
#endif
#ifndef PARALLEL
           }
           else
           {
             if ((next_recur_level == (params.ionb_levels-1)) && (params.nb_sections > 1))
             {
               spatial_data.save_region_class_label_map(recur_section);
               save_pixel_data(recur_section,recur_pixel_data,temp_data);
             }
           }
#else
         }
#endif

         if (max_threshold < threshold)
           max_threshold = threshold;

         if (((unsigned int) region_classes.size( )) < (nregions + recur_nregions))
           region_classes.resize(nregions + recur_nregions);

         region_index = nregions;
         recur_region_classes_size = recur_region_classes.size( );
         for (recur_region_index = 0; recur_region_index < recur_region_classes_size; ++recur_region_index)
         {
           if (recur_region_classes[recur_region_index].get_active_flag( ))
           {
             region_classes[region_index] = recur_region_classes[recur_region_index];
             region_label = region_index + 1;
             if (recur_region_classes[recur_region_index].get_label( ) != region_label)
             {
               if (params.debug > 0)
               {
                 params.log_fs << "WARNING:  recur_region_classes.label = " << recur_region_classes[recur_region_index].get_label( );
                 params.log_fs << " and region_label = " << region_label << endl;
               }
               else
               {
                 cout << "WARNING:  recur_region_classes.label = " << recur_region_classes[recur_region_index].get_label( );
                 cout << " and region_label = " << region_label << endl;
               }
             }
             region_classes[region_index].set_label(region_label);
             region_classes[region_index].set_merge_region_label(0);
             ++region_index;
           }
           else if (recur_region_index < recur_nregions)
           {
             if (params.debug > 0)
               params.log_fs << "WARNING:  recur_region_index " << recur_region_index << " is not active." << endl;
             else
             {
               cout << "WARNING:  recur_region_index " << recur_region_index << " is not active." << endl;
#ifdef PARALLEL
               cout << "Message from task = " << params.myid << endl;
#endif
             }
           }
         }

         nregions += recur_nregions;
#ifdef DEBUG
         if ((params.debug > 2) && (nregions > 1))
         {
           params.log_fs << endl << "After processing section " << recur_section << ", dump of the region_classes:" << endl << endl;
           region_classes_size = region_classes.size( );
           for (region_index = 0; region_index < region_classes_size; ++region_index)
             if (region_classes[region_index].get_active_flag( ))
               region_classes[region_index].print(region_classes);
             else if (region_index < nregions)
               params.log_fs << endl << "WARNING: RegionClass " << (region_index+1) << " is not active." << endl;
         }
#endif
         recur_section += stride;
       }  // for (row_section = 0; row_section < nb_row_sections; row_section++)
          //  for (col_section = 0; col_section < nb_col_sections; col_section++)
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "After assembling sections, dump of the region_classes:" << endl << endl;
       region_classes_size = region_classes.size( );
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         if (region_classes[region_index].get_active_flag( ))
           region_classes[region_index].print(region_classes);
     }
     if (params.debug > 3)
     {
       params.log_fs << endl << "After assembling sections, dump of the region labeling of pixel_data:" << endl << endl;
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
             pixel_data[pixel_index].print_region_label();
           }
           params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     }
#endif

     recur_region_classes.clear( );
     recur_pixel_data.clear( );

     if (nregions > 0)
     {
     // Initialize the seam_region_classes, and add the neighborhood relationships from along the processing window seams'
     // to the region_classes, all based on the *_seam_index_data
       vector<RegionClass> seam_region_classes(nregions,RegionClass());
#ifdef THREEDIM
       get_seam_index_data(recur_level,section,params.recur_mask_flags[recur_level],pixel_data,ncols,nrows,nslices,col_seam_index_data,
                           row_seam_index_data,slice_seam_index_data,temp_data);
       update_nghbrs_label_set(recur_level,ncols,nrows,nslices,col_seam_index_data,
                               row_seam_index_data,slice_seam_index_data,region_classes);
       if (params.debug > 0)
       {
        params.log_fs << endl << "At recursive level = " << recur_level;
        params.log_fs << ", calling artifact_elimination with the number of regions = " << nregions << ",";
        params.log_fs << endl << " and threshold = " << params.seam_edge_threshold;
       }

       artifact_elimination(recur_level,section,ncols,nrows,nslices,max_threshold,
                            col_seam_index_data,row_seam_index_data,slice_seam_index_data,
                            nregions, seam_region_classes, region_classes, nghbr_heap, pixel_data, temp_data);

       if (params.debug > 1)
       {
        params.log_fs << endl << "After call to artifact_elimination at recursive level = " << recur_level;
        params.log_fs << " with the number of regions = " << nregions;
       } // if (params.debug > 1)
#else
       get_seam_index_data(recur_level,section,params.recur_mask_flags[recur_level],pixel_data,ncols,nrows,col_seam_index_data,
                           row_seam_index_data,temp_data);
       update_nghbrs_label_set(recur_level,ncols,nrows,col_seam_index_data,
                               row_seam_index_data,region_classes);
       if (params.debug > 0)
       {
        params.log_fs << endl << "At recursive level = " << recur_level;
        params.log_fs << ", calling artifact_elimination with the number of regions = " << nregions << ",";
        params.log_fs << endl << " and threshold = " << params.seam_edge_threshold;
       }

       artifact_elimination(recur_level,section,ncols,nrows,max_threshold,
                            col_seam_index_data,row_seam_index_data,
                            nregions, seam_region_classes, region_classes, nghbr_heap, pixel_data, temp_data);

       if (params.debug > 1)
       {
        params.log_fs << endl << "After call to artifact_elimination at recursive level = " << recur_level;
        params.log_fs << " with the number of regions = " << nregions;
       } // if (params.debug > 1)
#endif
       seam_region_classes.clear( );
#ifdef DEBUG
       if (params.debug > 2)
       {
         params.log_fs << endl << "After adding neighbors across the seams and artifact eliminiation, dump of the region_classes:" << endl << endl;
         for (region_index = 0; region_index < nregions; ++region_index)
           region_classes[region_index].print(region_classes);
       }
#endif
     } // if (nregions > 0)

#ifdef PARALLEL
#ifdef TIME_IT
     if ((params.myid == 0) && (params.debug > 2))
     {
       end_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0)
         temp_data.compute += elapsed_time;
       temp_data.start_time = (((float) clock( ))/((float) CLOCKS_PER_SEC));
       params.log_fs << "setup time: " << temp_data.setup << endl;
       params.log_fs << "compute time: " << temp_data.compute << endl;
       params.log_fs << "transfer time: " << temp_data.transfer << endl;
       params.log_fs << "wait time: " << temp_data.wait << endl;
     }
#endif
#endif
     init_nregions = nregions;
   } // if (recur_level < (params.rnb_levels-1))

#ifdef DEBUG
   if (params.debug > 2)
   {
  // Check correctness of region_classes by comparing to what would be obtained from reinitializing region_classes from the pixel_data.
#ifdef THREEDIM
     if (region_classes_check(recur_level, section, ncols, nrows, nslices, pixel_data, region_classes, temp_data))
       params.log_fs << "region_classes_check was successful" << endl;
     else
       params.log_fs << "region_classes_check failed" << endl;
#else
     if (region_classes_check(recur_level, section, ncols, nrows, pixel_data, region_classes, temp_data))
       params.log_fs << "region_classes_check was successful" << endl;
     else
       params.log_fs << "region_classes_check failed" << endl;
#endif
   }
#endif

   if ((nregions > 0) && (params.program_mode == 3))
   {
  // Set up call to lhseg (HSeg) for current recursive level.
#ifdef DEBUG
    if (params.debug > 3)
    {
     params.log_fs << endl << "Dump of the pixel data:" << endl << endl;
     for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
       pixel_data[pixel_index].print(pixel_index);
    }
    if (params.debug > 3)
    {
     params.log_fs << endl << "Dump of the region data:" << endl << endl;
     region_classes_size = region_classes.size( );
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       region_classes[region_index].print(region_classes);
    }
#endif

    unsigned int converge_nregions;
    converge_nregions = params.min_nregions;
    if (recur_level == 0)
    {
      if ((params.chk_nregions_flag) && (params.chk_nregions > converge_nregions))
        converge_nregions = params.chk_nregions;
      if ((params.hseg_out_nregions_flag) && (params.hseg_out_nregions[0] > converge_nregions))
        converge_nregions = params.hseg_out_nregions[0];
    }

    if (nregions > converge_nregions)
    {
// Modified July 11, 2013
//     max_threshold = 0.0;
     threshold = 0.0;
     if (params.debug > 0)
     {
      params.log_fs << endl << "At recursive level = " << recur_level;
      params.log_fs << ", calling HSeg with the number of regions = " << nregions << ",";
      params.log_fs << endl << " converge_nregions = " << converge_nregions << " and max_threshold = " << threshold;
      params.log_fs << endl;
     }

     lhseg(recur_level, false, section, converge_nregions, nregions, threshold, 
           pixel_data, region_classes, nghbr_heap, region_heap, temp_data);
// Added July 11, 2013
     if (max_threshold < threshold)
       max_threshold = threshold;
     if ((nregions <= params.spclust_max) && (params.min_npixels > 1))
     {
      params.min_npixels = 1;
      if (params.debug > 1)
      {
        params.log_fs << "Exiting lhseg, min_npixels readjusted to " << params.min_npixels;
        params.log_fs << " giving region_heap_size = " << nregions << "." << endl;
      }
     }  

     if (params.debug > 1)
     {
      params.log_fs << endl << "After call to HSeg at recursive level = " << recur_level;
      params.log_fs << " with the number of regions = " << nregions;
      params.log_fs << endl << "and max_threshold = " << max_threshold;
#ifdef DEBUG
      if (params.debug > 3)
      {
        params.log_fs << endl << "After call to HSeg, dump of the pixel data:" << endl << endl;
        for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
        {
          pixel_data[pixel_index].print(pixel_index);
        }
      }
      if (params.debug > 2)
      {
        params.log_fs << endl << "After call to HSeg, dump of the region data:" << endl << endl;
        region_classes_size = region_classes.size( );
        for (region_index = 0; region_index < region_classes_size; ++region_index)
          if (region_classes[region_index].get_active_flag( ))
            region_classes[region_index].print(region_classes);
      }
      if (params.debug > 3)
      {
        params.log_fs << endl << "After call to HSeg, dump of the pixel_data.region_label:" << endl << endl;
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
            params.log_fs << pixel_data[pixel_index].get_region_label( ) << " ";
          }
          params.log_fs << endl;
         }
         params.log_fs << endl;
#ifdef THREEDIM
        }
#endif
      }
#else
     params.log_fs << endl;
#endif
     }

    } // if (nregions > converge_nregions)

    region_classes_size = region_classes.size( );
#ifdef DEBUG
    if (params.debug > 3)
    {
      params.log_fs << endl << "Before renumbering, dump of the region data:" << endl << endl;
      for (region_index = 0; region_index < region_classes_size; ++region_index)
        if (region_classes[region_index].get_active_flag( ))
          region_classes[region_index].print(region_classes);
    }
#endif

  // Make the region labeling compact.
    compact_region_index = 0;
    vector<RegionClass> compact_region_classes(nregions,RegionClass( ));
    for (region_index = 0; region_index < region_classes_size; ++region_index)
    {
      if ((region_classes[region_index].get_active_flag( )) &&
          (region_classes[region_index].get_npix( ) > 0))
      {
        compact_region_classes[compact_region_index++] = region_classes[region_index];
      }
      if (compact_region_index == nregions)
        break;
    }

    if (params.sort_flag)
    {
   // Sort region_classes by number of pixels.
      sort(compact_region_classes.begin( ), compact_region_classes.end( ), NpixMoreThan( ));
    }

 // Relabel region_classes as necessary to make the labeling compact.
    region_relabel_pairs.clear( );
    for (region_index = 0; region_index < nregions; ++region_index)
    {
      region_classes[region_index] = compact_region_classes[region_index];
      region_label = region_classes[region_index].get_label( );
      region_classes[region_index].set_label(region_index + region_label_offset + 1);
      region_classes[region_index].clear_best_merge( );
      if (region_label != region_classes[region_index].get_label( ))
        region_relabel_pairs.insert(make_pair(region_label,region_classes[region_index].get_label( )));
    }

    if (!region_relabel_pairs.empty( ))
    {
    // Use region_relabel_pairs to renumber the nghbrs_label_set for each region.
     for (region_index = 0; region_index < nregions; ++region_index)
       region_classes[region_index].nghbrs_label_set_renumber(region_relabel_pairs);

  // Use region_relabel_pairs to renumber region_label in pixel_data.
     do_region_class_relabel(recur_level,section,region_relabel_pairs,pixel_data,temp_data);

  // region_classes is now compact.
     region_relabel_pairs.clear( );
    } // if (!region_relabel_pairs.empty( ))
    region_classes_size = region_classes.size( );
    for (region_index = nregions; region_index < region_classes_size; ++region_index)
    {
     region_classes[region_index].clear( );
     region_classes[region_index].set_label(region_index + region_label_offset + 1);
    }

    if (params.debug > 0)
    {
     unsigned int large_nregions = 0;
     if (params.min_npixels > 1)
     {
       region_classes_size = region_classes.size();
       for (region_index = 0; region_index < region_classes_size; ++region_index)
       {
         if ((region_classes[region_index].get_active_flag()) && 
             (region_classes[region_index].get_npix() >= params.min_npixels))
         {
           large_nregions++;
         }
       }
     }
     if (recur_level > 0)
     {
       params.log_fs << endl << "Completed recursive level = " << recur_level;
       params.log_fs << " in lrhseg with the number of regions = " << nregions;
     }
     else
     {
       params.log_fs << endl << "Returned to main HSeg function with the number of regions = " << nregions;
     }
     params.log_fs << endl << "and max_threshold = " << max_threshold;
     if (params.min_npixels > 1)
       params.log_fs << " (with min_npixels = " << params.min_npixels << ", large_nregions = " << large_nregions << ")";
#ifdef DEBUG
     if (params.debug > 2)
     {
       for (region_index = 0; region_index < nregions; ++region_index)
         if (!region_classes[region_index].get_active_flag( ))
           params.log_fs << "WARNING:  RegionClass " << region_classes[region_index].get_label( ) << " is inactive!!" << endl;
     }
     if (params.debug > 3)
     {
       params.log_fs << endl << "Exiting lrhseg, dump of the region data:" << endl << endl;
       for (region_index = 0; region_index < nregions; ++region_index)
         if (region_classes[region_index].get_active_flag( ))
           region_classes[region_index].print(region_classes,region_label_offset);
         else if (region_index < nregions)
           params.log_fs << "WARNING:  RegionClass " << region_classes[region_index].get_label( ) << " is inactive!!" << endl;
     }
     if (params.debug > 3)
     {
       params.log_fs << endl << "Exiting lrhseg, dump of the region labeling of the pixel_data:" << endl << endl;
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
           pixel_data[pixel_index].print_region_label();
         }
         params.log_fs << endl;
        }
        params.log_fs << endl;
#ifdef THREEDIM
       }
#endif
     }
     if (params.debug > 3)
     {
       params.log_fs << endl << "Exiting lrhseg, dump of the pixel data:" << endl << endl;
       for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
       {
         pixel_data[pixel_index].print(pixel_index);
       }
     }
#else
     params.log_fs << endl;
#endif
    }
    global_nregions -= (init_nregions - nregions);
   } // if ((nregions > 0) && (params.program_mode == 3))
#ifdef PARALLEL
   short unsigned int factor = 1;
   if (recur_level > 0)
   {
     int nb_strides;
#ifdef THREEDIM
     nb_strides = set_recur_flags(params.recur_mask_flags[recur_level-1],col_flag,row_flag,slice_flag);
#else
     nb_strides = set_recur_flags(params.recur_mask_flags[recur_level-1],col_flag,row_flag);
#endif
     factor = nb_strides*nb_sections;
   }
   short unsigned int parent_section = (params.myid/factor)*(factor);
   if ((recur_level < params.inb_levels) && (parent_section != params.myid))
   {
     if (params.debug > 1)
       params.log_fs << "Sending results to task (section) " << parent_section << endl;
     recur_send( parent_section, recur_level, pixel_data, region_classes,
                 nregions, global_nregions, max_threshold, temp_data);
     if (params.debug > 1)
       params.log_fs << "Sent results to task (section) " << parent_section << endl;
   }
#endif
   oparams.percent_complete = (100*(oparams.tot_npixels-global_nregions))/oparams.tot_npixels;
   if (oparams.percent_complete == 100)
     oparams.percent_complete = 99;
#ifdef PARALLEL
   if ((params.myid == 0) && (params.debug > 0))
     params.log_fs << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#else
   if (!params.gtkmm_flag)
     cout << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#endif

   return nregions;
 }
} // namespace HSEGTilton
