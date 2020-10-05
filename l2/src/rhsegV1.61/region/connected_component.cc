/*-----------------------------------------------------------
|
|  Routine Name: connected_component_init (regular version)
|
|       Purpose: Performs initial connected component labeling
|
|         Input: ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                region_objects   (Class which holds region object related information)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                
|        Output:
|
|       Returns: nb_objects    (Number of connected regions after connected component labeling)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: October 10, 2003
| Modifications: August 9, 2004 - Boundary map definition modified to make image edge pixels region boundary pixels
|                May 31, 2005 - Added temporary file I/O for faster processing of large data sets
|                November 22, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                December 16, 2005 - Undid August 9, 2004 modification
|                January 4, 2006 - Corrected error in counting number of connected regions
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|                May 16, 2011 - Added option for 4nn region object finding for 8nn or more analysis for HSWO, HSEG and RHSEG.
|                February 7, 2014 - Revised connected_component_init (regular version) to be more efficient.
|
------------------------------------------------------------*/

#include "region_object.h"
#include <params/params.h>
#include <spatial/spatial.h>
#include <pixel/pixel.h>
#include <index/index.h>
#include <map>
#if (defined(TIME_IT) || defined(DEBUG))
#include <ctime>
#endif

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef THREEDIM
 void connected_component_init(const int& ncols, const int& nrows, const int& nslices, unsigned int& nb_objects,
                               vector<RegionObject>& region_objects, Spatial& spatial_data)
#else
 void connected_component_init(const int& ncols, const int& nrows, unsigned int& nb_objects,
                               vector<RegionObject>& region_objects, Spatial& spatial_data)
#endif
 {
   int col, row;
#ifdef THREEDIM
   int slice;
#endif
   unsigned int pixel_index;

   if (params.debug > 2)
   {
     params.log_fs << "Performing connected component labeling";
     params.log_fs << " with initial number of regions = " << nb_objects << endl;
   }

#ifdef DEBUG
   time_t now;
   if (params.debug > 2)
   {
     now = time(NULL);
     params.log_fs << "Performing connected component labeling";
     params.log_fs << " at " << ctime(&now) << endl;
   }

   if (params.debug > 3)
   {
     params.log_fs << endl << "Entering connected_component, dump of region_class_label_map:" << endl << endl;
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
           params.log_fs << spatial_data.region_class_label_map[pixel_index] << " ";
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
       spatial_data.region_object_label_map[pixel_index] = 0;
     }
#ifdef THREEDIM
   }
#endif

   short unsigned int nbdir, maxnbdir = params.object_maxnbdir;
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
   unsigned int nbpixel_index=0;
   unsigned int region_class_label, nbregion_class_label;
   unsigned int region_object_label, nbregion_object_label, max_label = nb_objects;
   map<unsigned int,unsigned int> region_object_relabel_pairs;
   map<unsigned int,unsigned int>::iterator region_object_relabel_pairs_iter;

   int nbcol, nbrow;
#ifdef THREEDIM
   int nbslice;
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
       region_class_label = spatial_data.region_class_label_map[pixel_index];
       if (region_class_label != 0)
       {
         region_object_label = spatial_data.region_object_label_map[pixel_index];
         if (region_object_label == 0)
         {
           region_object_label = ++max_label;
           spatial_data.region_object_label_map[pixel_index] = region_object_label;
         }
         for (nbdir = 0; nbdir < params.object_maxnbdir; nbdir++)
         {
           nbregion_class_label = 0;
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
             nbregion_class_label = spatial_data.region_class_label_map[nbpixel_index];
           // Take care of spatial_data.boundary_map
             if ((nbdir < maxnbdir) &&
                 (region_class_label > 0) && (nbregion_class_label > 0) &&
                 (nbregion_class_label != region_class_label))
             {
               spatial_data.boundary_map[pixel_index] = 1;
               spatial_data.boundary_map[nbpixel_index] = 1;
             }
           }
           if (nbregion_class_label == region_class_label)
           {
             nbregion_object_label = spatial_data.region_object_label_map[nbpixel_index];
             if (nbregion_object_label == 0)
             {
               spatial_data.region_object_label_map[nbpixel_index] = region_object_label;
             }
             else if (region_object_label < nbregion_object_label)
             {
               spatial_data.region_object_label_map[nbpixel_index] = region_object_label;
               region_object_relabel_pairs.insert(make_pair(nbregion_object_label,region_object_label));
             }
             else if (region_object_label > nbregion_object_label)
             {
               spatial_data.region_object_label_map[pixel_index] = nbregion_object_label;
               region_object_relabel_pairs.insert(make_pair(region_object_label,nbregion_object_label));
               region_object_label = nbregion_object_label;
             }
           } // if (nbregion_class_label == region_class_label)
         } // for (nbdir = 0; nbdir < params.object_maxnbdir; nbdir++)
       } // if (region_class_label != 0)
     } // for (col = 0; col < ncols; col++)
    } // for (row = 0; row < nrows; row++)
#ifdef THREEDIM
   } // for (slice = 0; slice < nslices; slice++)
#endif

   while (!region_object_relabel_pairs.empty())
   {
     if (params.debug > 2)
       params.log_fs << "Doing clean up with region_object_relabels_pairs.size() = " << region_object_relabel_pairs.size() << endl;
    // Clean up labeling
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
         region_object_label = spatial_data.region_object_label_map[pixel_index];
         if (region_object_label != 0)
         {
           region_object_relabel_pairs_iter = region_object_relabel_pairs.find(region_object_label);
           while (region_object_relabel_pairs_iter != region_object_relabel_pairs.end())
           {
             region_object_label = (*region_object_relabel_pairs_iter).second;
             region_object_relabel_pairs_iter = region_object_relabel_pairs.find(region_object_label);
           }
           spatial_data.region_object_label_map[pixel_index] = region_object_label;
         }
       } // for (col = 0; col < ncols; col++)
      } // for (row = 0; row < nrows; row++)
#ifdef THREEDIM
     } // for (slice = 0; slice < nslices; slice++)
#endif

     region_object_relabel_pairs.clear();
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
         region_class_label = spatial_data.region_class_label_map[pixel_index];
         region_object_label = spatial_data.region_object_label_map[pixel_index];
         if (region_class_label != 0)
         {
           for (nbdir = 0; nbdir < params.object_maxnbdir; nbdir++)
           {
             nbregion_class_label = 0;
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
               nbregion_class_label = spatial_data.region_class_label_map[nbpixel_index];
             }
             if (nbregion_class_label == region_class_label)
             {
               nbregion_object_label = spatial_data.region_object_label_map[nbpixel_index];
               if (region_object_label < nbregion_object_label)
               {
                 spatial_data.region_object_label_map[nbpixel_index] = region_object_label;
                 region_object_relabel_pairs.insert(make_pair(nbregion_object_label,region_object_label));
               }
               else if (region_object_label > nbregion_object_label)
               {
                 spatial_data.region_object_label_map[pixel_index] = nbregion_object_label;
                 region_object_relabel_pairs.insert(make_pair(region_object_label,nbregion_object_label));
                 region_object_label = nbregion_object_label;
               }
             } // if (nbregion_class_label == region_class_label)
           } // for (nbdir = 0; nbdir < params.object_maxnbdir; nbdir++)
         } // if (region_class_label != 0)
       } // for (col = 0; col < ncols; col++)
      } // for (row = 0; row < nrows; row++)
#ifdef THREEDIM
     } // for (slice = 0; slice < nslices; slice++)
#endif
   } // while (!region_object_labels_pairs.empty())

#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After connected component labeling algorithm, dump of connected region label:" << endl << endl;
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
         params.log_fs << spatial_data.region_object_label_map[pixel_index] << " ";
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
   if (params.debug > 2)
   {
     params.log_fs << "Before compacting the region_object labeling, ";
     params.log_fs << "max_label = " << max_label << endl;
   }
 // Make the region_class_label numbering compact
   region_object_relabel_pairs.clear();
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
       region_object_label = spatial_data.region_object_label_map[pixel_index];
       if (region_object_label != 0)
       {
         region_object_relabel_pairs_iter = region_object_relabel_pairs.find(region_object_label);
         if (region_object_relabel_pairs_iter != region_object_relabel_pairs.end())
         {
           region_object_label = (*region_object_relabel_pairs_iter).second;
         }
         else
         {
           region_object_relabel_pairs.insert(make_pair(region_object_label,++nb_objects));
           region_object_label = nb_objects;
         }
         spatial_data.region_object_label_map[pixel_index] = region_object_label;
       }
     }
    }
#ifdef THREEDIM
   }
#endif
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After compacting the labeling, dump of connected region label:" << endl << endl;
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
         params.log_fs << spatial_data.region_object_label_map[pixel_index] << " ";
       }
       params.log_fs << endl;
      }
      params.log_fs << endl;
#ifdef THREEDIM
     }
     params.log_fs << endl;
#endif
   }
#endif // DEBUG

   unsigned int region_object_index, region_objects_size;
   region_objects_size = nb_objects;
   region_objects.resize(region_objects_size,RegionObject());
   for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
   {
     region_objects[region_object_index].clear();
     region_object_label = region_object_index + 1;
     region_objects[region_object_index].set_label_active(region_object_label);
     region_objects[region_object_index].set_merge_region_label(region_object_label);
   }

#ifdef DEBUG
   if (params.debug > 2)
   {
     now = time(NULL);
     params.log_fs << "Compact labeling completed";
     params.log_fs << ", with " << nb_objects << " regions at " << ctime(&now) << endl;
   }
#else
   if (params.debug > 2)
   {
     params.log_fs << "Connected component labeling completed";
     params.log_fs << ", with " << nb_objects << " regions" << endl;
   }
#endif
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: connected_component_init (recursive version)
|
|       Purpose: Performs initial connected component labeling
|
|         Input: recur_level      (Current recursive level of this task)
|                section          (Section or window processed by this call to lrhseg)
|                ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                region_objects (Class which holds connected region related information)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                
|        Output:
|
|         Other: temp_data        (buffers used in communications between parallel tasks)

|       Returns: nb_objects    (Number of connected regions after connected component labeling)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: October 10, 2003
| Modifications: (See comments for first instance of connected_component_init)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void connected_component_init(const short unsigned int& recur_level, const short unsigned int& section,
                               const int& ncols, const int& nrows, const int& nslices,
                               unsigned int& nb_objects, vector<RegionObject>& region_objects,
                               Spatial& spatial_data, Temp& temp_data)
#else
 void connected_component_init(const short unsigned int& recur_level, const short unsigned int& section,
                               const int& ncols, const int& nrows,
                               unsigned int& nb_objects, vector<RegionObject>& region_objects,
                               Spatial& spatial_data, Temp& temp_data)
#endif
 {
#ifdef DEBUG
   time_t now;
   if (params.debug > 2)
   {
     now = time(NULL);
     params.log_fs << "Performing connected component labeling at recur_level = " << recur_level << endl;
     params.log_fs << "at " << ctime(&now) << endl;
   }
#else
   if (params.debug > 2)
   {
     params.log_fs << "Performing connected component labeling at recur_level = " << recur_level;
     params.log_fs << " with section = " << section << endl;
   }
#endif

   unsigned int region_object_index, region_object_label, region_objects_size;
   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of connected_component_init can be called.
#ifdef THREEDIM
     connected_component_init(ncols,nrows,nslices,nb_objects,region_objects,spatial_data);
#else
     connected_component_init(ncols,nrows,nb_objects,region_objects,spatial_data);
#endif
#ifdef DEBUG
     if (params.debug > 2)
     {
       now = time(NULL);
       params.log_fs << "Connected component labeling completed";
       params.log_fs << ", with " << nb_objects << " regions at " << ctime(&now) << endl;
     }
#else
     if (params.debug > 2)
     {
       params.log_fs << "Connected component labeling completed";
       params.log_fs << ", with " << nb_objects << " regions" << endl;
     }
#endif
   } // if (recur_level >= (params.onb_levels-1))
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // connected_component_init must be called recursively.
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
     short unsigned int next_recur_level = recur_level + 1;
     int stride, nb_sections;
     set_stride_sections(recur_level,stride,nb_sections);
#ifdef PARALLEL
     unsigned int recur_nb_objects;
#ifdef THREEDIM
   // Send request to the parallel recur_tasks
     parallel_recur_requests((short unsigned int) 16, recur_level, 0, ncols, nrows, nslices, 0, 0, 0, temp_data);
   // Process current task's data section
     connected_component_init(next_recur_level, section, recur_ncols, recur_nrows, recur_nslices, nb_objects,
                              region_objects, spatial_data, temp_data);
#else
   // Send request to the parallel recur_tasks
     parallel_recur_requests((short unsigned int) 16, recur_level, 0, ncols, nrows, 0, 0, 0, temp_data);
   // Process current task's data section
     connected_component_init(next_recur_level, section, recur_ncols, recur_nrows, nb_objects,
                              region_objects, spatial_data, temp_data);
#endif
    // Receive nb_objects result from connected component labeling, and submit class_label_offset request
     int conn_comp_init_tag = 116;
     int min_section = section + stride;
#ifdef TIME_IT
     float end_time, elapsed_time;
#endif
#else
     int min_section = section;
#endif
     int max_section = section + nb_sections;
     int recur_section;
     for (recur_section = min_section; recur_section < max_section; recur_section += stride)
     {
#ifdef PARALLEL
       if (params.debug > 2)
         params.log_fs << "Waiting for connected component labeling result from task " << recur_section << endl;
#ifdef TIME_IT
       end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
       temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
       MPI::COMM_WORLD.Recv(&recur_nb_objects, 1, MPI::UNSIGNED, recur_section, conn_comp_init_tag);
#ifdef TIME_IT
       end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
       temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
       if (params.debug > 2)
       {
         params.log_fs << "Connected component labeling completed on task " << recur_section;
         params.log_fs << " with " << recur_nb_objects << " regions" << endl;
       }
      // Make do_class_label_offset request to recursive subtask.
#ifdef THREEDIM
       parallel_recur_request((short unsigned int) 1,next_recur_level,
                              recur_section,0,nb_objects,0,0,0,0,0,temp_data);
#else
       parallel_recur_request((short unsigned int) 1,next_recur_level,
                              recur_section,0,nb_objects,0,0,0,0,temp_data);
#endif
       nb_objects += recur_nb_objects;
#else // !PARALLEL
     // In the serial case, process the specified data section
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
         spatial_data.restore_region_class_label_map(recur_section);
#ifdef THREEDIM
       connected_component_init(next_recur_level, recur_section, recur_ncols, recur_nrows, recur_nslices, nb_objects,
                                region_objects, spatial_data, temp_data);
#else
       connected_component_init(next_recur_level, recur_section, recur_ncols, recur_nrows, nb_objects,
                                region_objects, spatial_data, temp_data);
#endif
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
       {
         spatial_data.save_region_object_label_map(recur_section);
         spatial_data.save_boundary_map(recur_section);
       }
#endif // !PARALLEL
       if (params.debug > 2)
       {
         params.log_fs << "For section " << recur_section << ", number of regions so far = " << nb_objects << endl;
       }
     }
#ifdef PARALLEL
    // Receive confirmation of completion of region label offset from sub_tasks.
#ifdef TIME_IT
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
     for (recur_section = min_section; recur_section < max_section; recur_section += stride)
     {
       int class_label_offset_tag = 40;
       MPI::COMM_WORLD.Recv(&region_object_index, 1, MPI::UNSIGNED, recur_section, class_label_offset_tag);
     }
#ifdef TIME_IT
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
#endif
     if (params.debug > 2)
     {
       params.log_fs << "Connected component labeling completed on each subsection resulting in " << nb_objects;
       params.log_fs << " regions." << endl;
     }

   // Do connected component labeling along the processing window seams
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

     get_seam_index_data(recur_level,section,params.recur_mask_flags[recur_level],spatial_data,
                         ncols,nrows,nslices,col_seam_index_data,row_seam_index_data,
                         slice_seam_index_data,temp_data);
#else
     get_seam_index_data(recur_level,section,params.recur_mask_flags[recur_level],spatial_data,
                         ncols,nrows,col_seam_index_data,row_seam_index_data,
                         temp_data);
#endif
     region_objects_size = nb_objects;
     region_objects.resize(region_objects_size,RegionObject());
     for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
     {
       region_objects[region_object_index].clear();
       region_object_label = region_object_index + 1;
       region_objects[region_object_index].set_label_active(region_object_label);
       region_objects[region_object_index].set_merge_region_label(region_object_label);
     }
#ifdef THREEDIM
     connected_component_seams(recur_level,section,nb_sections,params.recur_mask_flags[recur_level],0,ncols,nrows,nslices,
                               col_seam_index_data,row_seam_index_data,slice_seam_index_data,
                               region_objects,spatial_data,temp_data);
#else
     connected_component_seams(recur_level,section,nb_sections,params.recur_mask_flags[recur_level],0,ncols,nrows,
                               col_seam_index_data,row_seam_index_data,
                               region_objects,spatial_data,temp_data);
#endif
     map<unsigned int,unsigned int> region_object_relabel_pairs;
     unsigned int merge_region_object_index, merge_region_label;
     for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
     {
       region_object_label = region_object_index + 1;
       merge_region_label = region_objects[region_object_index].get_merge_region_label();
       while (merge_region_label != region_object_label)
       {
         region_object_label = merge_region_label;
         merge_region_object_index = merge_region_label - 1;
         merge_region_label = region_objects[merge_region_object_index].get_merge_region_label();
       }
       region_objects[region_object_index].set_merge_region_label(region_object_label);
     }
     for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
     {
       region_object_label = region_object_index + 1;
       merge_region_label = region_objects[region_object_index].get_merge_region_label();
       if (merge_region_label != region_object_label)
         region_object_relabel_pairs.insert(make_pair(region_object_label,merge_region_label));
     }
     if (!(region_object_relabel_pairs.empty()))
#ifdef THREEDIM
       do_region_object_relabel(recur_level, section, region_object_relabel_pairs,
                              spatial_data, ncols, nrows, nslices, temp_data);
#else
       do_region_object_relabel(recur_level, section, region_object_relabel_pairs,
                              spatial_data, ncols, nrows, temp_data);
#endif
   // Renumber region_objects for a compact labeling, set up region_object_relabel_pairs.
     region_object_relabel_pairs.clear();
     nb_objects = 0;
     for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
       if (region_objects[region_object_index].get_active_flag())
       {
         nb_objects++;
         region_object_label = region_objects[region_object_index].get_label();
         if (region_object_label != nb_objects)
           region_object_relabel_pairs.insert(make_pair(region_object_label,nb_objects));
       }
   // Use region_object_relabel_pairs to renumber region_object_label_map in spatial_data.
     if (!(region_object_relabel_pairs.empty()))
#ifdef THREEDIM
       do_region_object_relabel(recur_level, section, region_object_relabel_pairs,
                              spatial_data, ncols, nrows, nslices, temp_data);
#else
       do_region_object_relabel(recur_level, section, region_object_relabel_pairs,
                              spatial_data, ncols, nrows, temp_data);
#endif
     region_objects_size = nb_objects;
     region_objects.resize(region_objects_size,RegionObject());
     for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
     {
       region_objects[region_object_index].clear();
       region_object_label = region_object_index+1;
       region_objects[region_object_index].set_label(region_object_label);
       region_objects[region_object_index].set_merge_region_label(region_object_label);
     }

     if (params.debug > 2)
     {
       params.log_fs << "Connected component completed along the processing window seams";
       params.log_fs << ", with the number of regions = " << nb_objects <<  endl;
     }
   }
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: connected_component (regular version)
|
|       Purpose: Updates a connected component labeling
|
|         Input: ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                nslevels         (Number of segmentation levels saved)
|                region_objects (Class which holds connected region related information)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                
|        Output:
|
|       Returns:
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: October 10, 2003
| Modifications: (See comments for first instance of connected_component_init)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void connected_component(const int& ncols, const int& nrows, const int& nslices,
                          const short unsigned int& nslevels, vector<RegionObject>& region_objects,
                          Spatial& spatial_data)
#else
 void connected_component(const int& ncols, const int& nrows,
                          const short unsigned int& nslevels, vector<RegionObject>& region_objects,
                          Spatial& spatial_data)
#endif
 {
   int col, row;
#ifdef THREEDIM
   int slice;
#endif
   unsigned int pixel_index;
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Entering connected_component, dump of region_class_label_map:" << endl << endl;
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
         params.log_fs << spatial_data.region_class_label_map[pixel_index] << " ";
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
     params.log_fs << endl << "Entering connected_component, dump of region_object_label_map:" << endl << endl;
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
         params.log_fs << spatial_data.region_object_label_map[pixel_index] << " ";
       }
       params.log_fs << endl;
      }
      params.log_fs << endl;
#ifdef THREEDIM
     }
#endif
   }
#endif
   short unsigned int nbdir, maxnbdir = params.object_maxnbdir;
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
   unsigned int region_class_label, nbregion_class_label, merge_region_label;
   unsigned int region_object_label, nbregion_object_label, region_object_index, nbregion_object_index;
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
       region_class_label = spatial_data.region_class_label_map[pixel_index];
       if (region_class_label > 0)
       {
         for (nbdir = 0; nbdir < params.object_maxnbdir; ++nbdir)
         {
           nbregion_class_label = 0;
#ifdef THREEDIM
           find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
           if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
               (nbslice<nslices) && (nbrow<nrows) && (nbcol<ncols))
           {
             nbpixel_index = nbcol + nbrow*ncols + nbslice*nrows*ncols;
#else
           find_nghbr(col,row,nbdir,nbcol,nbrow);
           if ((nbrow>=0) && (nbcol>=0) &&
               (nbrow<nrows) && (nbcol<ncols))
           {
             nbpixel_index = nbcol + nbrow*ncols;
#endif
             nbregion_class_label = spatial_data.region_class_label_map[nbpixel_index];
             if ((nbdir < maxnbdir) && (nbregion_class_label > 0) && (nbregion_class_label != region_class_label))
               spatial_data.boundary_map[pixel_index] = (short unsigned int) (nslevels+1);
           }
           if (nbregion_class_label == region_class_label)
           {
             region_object_label = spatial_data.region_object_label_map[pixel_index];
             region_object_index = region_object_label - 1;
             while (region_objects[region_object_index].merge_region_label != region_object_label)
             {
               region_object_label = region_objects[region_object_index].merge_region_label;
               region_object_index = region_object_label - 1;
             }
             nbregion_object_label = spatial_data.region_object_label_map[nbpixel_index];
             nbregion_object_index = nbregion_object_label - 1;
             while (region_objects[nbregion_object_index].merge_region_label != nbregion_object_label)
             {
               nbregion_object_label = region_objects[nbregion_object_index].merge_region_label;
               nbregion_object_index = nbregion_object_label - 1;
             }
            // Can't have region_object_label == 0
             if (region_object_label != nbregion_object_label)
             {
               merge_region_label = region_object_label;
               if (nbregion_object_label < merge_region_label)
                 merge_region_label = nbregion_object_label;
               if (region_object_label != merge_region_label)
               {
                 region_objects[region_object_index].active_flag = false;
                 region_objects[region_object_index].merge_region_label = merge_region_label;
#ifdef DEBUG
                 if (params.debug > 2)
                 {
#ifdef THREEDIM
                   params.log_fs << "At (col,row,slice) (" << col << "," << row << "," << slice << "), region_object label ";
#else
                   params.log_fs << "At (col,row) (" << col << "," << row << "), region_object label ";
#endif
                   params.log_fs << region_object_label << " merge into region " << merge_region_label << endl;
                 }
#endif
               }
               if (nbregion_object_label != merge_region_label)
               {
                 region_objects[nbregion_object_index].active_flag = false;
                 region_objects[nbregion_object_index].merge_region_label = merge_region_label;
#ifdef DEBUG
                 if (params.debug > 2)
                 {
#ifdef THREEDIM
                   params.log_fs << "At (col,row,slice) (" << col << "," << row << "," << slice << "), nbregion_object label ";
#else
                   params.log_fs << "At (col,row) (" << col << "," << row << "), nbregion_object label ";
#endif
                   params.log_fs << nbregion_object_label << " merge into region " << merge_region_label << endl;
                 }
#endif
               }
             }
           } // if (nbregion_class_label == region_class_label)
         } // for (nbdir = 0; nbdir < params.object_maxnbdir; ++nbdir)
       }
     }
#ifdef THREEDIM
   }
#endif
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: connected_component (recursive version)
|
|       Purpose: Updates a connected component labeling
|
|         Input: recur_level      (Current recursive level of this task)
|                section          (Section or window processed by this call to lrhseg)
|                ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                nslevels         (Number of segmentation levels saved)
|                region_objects (Class which holds connected region related information)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                
|        Output:
|
|         Other: temp_data        (buffers used in communications between parallel tasks)

|       Returns:
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: October 10, 2003
| Modifications: (See comments for first instance of connected_component_init)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void connected_component(const short unsigned int& recur_level, const short unsigned int& section,
                          const int& ncols, const int& nrows, const int& nslices, const short unsigned int& nslevels,
                          vector<RegionObject>& region_objects, Spatial& spatial_data,
                          Temp& temp_data)
#else
 void connected_component(const short unsigned int& recur_level, const short unsigned int& section,
                          const int& ncols, const int& nrows, const short unsigned int& nslevels,
                          vector<RegionObject>& region_objects, Spatial& spatial_data,
                          Temp& temp_data)
#endif
 {
   unsigned int region_object_index, region_objects_size = region_objects.size();
   if (recur_level >= (params.onb_levels-1))
   {
    // At these recursive levels, the data is wholly contained in RAM memory and
    // the regular version of connected_component can be called.
#ifdef THREEDIM
     connected_component(ncols, nrows, nslices, nslevels, region_objects, spatial_data);
#else
     connected_component(ncols, nrows, nslevels, region_objects, spatial_data);
#endif
   }
   else
   {
    // At these recursive levels, the data is not wholly contained in RAM memory and
    // connected_component must be called recursively.
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
     short unsigned int next_recur_level = recur_level + 1;
     int stride, nb_sections;
     set_stride_sections(recur_level,stride,nb_sections);
#ifdef PARALLEL
#ifdef THREEDIM
   // Send request to the parallel recur_tasks
     parallel_recur_requests((short unsigned int) 17, recur_level, 0, ncols, nrows, nslices, 
                             region_objects_size, nslevels, 0, temp_data);
   // Process current task's data section
     connected_component(next_recur_level, section, recur_ncols, recur_nrows, recur_nslices, nslevels,
                         region_objects, spatial_data, temp_data);
#else
   // Send request to the parallel recur_tasks
     parallel_recur_requests((short unsigned int) 17, recur_level, 0, ncols, nrows, 
                             region_objects_size, nslevels, 0, temp_data);
   // Process current task's data section
     connected_component(next_recur_level, section, recur_ncols, recur_nrows, nslevels,
                         region_objects, spatial_data, temp_data);
#endif
    // Receive region_objects result from connected component labeling
     int conn_comp_tag = 117;
     unsigned int short_buf_size = 0, int_buf_size = 0, double_buf_size = 0;
     unsigned int int_buf_position;
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
       if (params.debug > 2)
         params.log_fs << "Waiting for connected component labeling result from task " << recur_section << endl;
#ifdef TIME_IT
       end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
       temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
       MPI::COMM_WORLD.Recv(&int_buf_size, 1, MPI::UNSIGNED, recur_section, conn_comp_tag);
#ifdef TIME_IT
       end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
       temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
       check_buf_size(0,short_buf_size,int_buf_size,0,double_buf_size,temp_data);
       if (int_buf_size > 0)
       {
         MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                              recur_section, conn_comp_tag);
#ifdef TIME_IT
         end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
         elapsed_time = end_time - temp_data.start_time;
         if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
         temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
         int_buf_position = 0;
         while (int_buf_position < int_buf_size)
         {
           if (region_objects[int_buf_position].get_merge_region_label() >
               temp_data.int_buffer[int_buf_position])
           {
             region_objects[int_buf_position].set_merge_region_label(temp_data.int_buffer[int_buf_position]);
           }
           int_buf_position++;
         }
       }
       if (params.debug > 2)
       {
         params.log_fs << "Connected component labeling completed on task " << recur_section << endl;
       }
#else // !PARALLEL
     // In the serial case, process the specified data section
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
       {
         spatial_data.restore_region_class_label_map(recur_section);
         spatial_data.restore_region_object_label_map(recur_section);
         spatial_data.restore_boundary_map(recur_section);
       }
#ifdef THREEDIM
       connected_component(next_recur_level, recur_section, recur_ncols, recur_nrows, recur_nslices, nslevels,
                           region_objects, spatial_data, temp_data);
#else
       connected_component(next_recur_level, recur_section, recur_ncols, recur_nrows, nslevels,
                           region_objects, spatial_data, temp_data);
#endif
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
       {
         spatial_data.save_region_object_label_map(recur_section);
         spatial_data.save_boundary_map(recur_section);
       }
#endif // !PARALLEL
     }
     if (params.debug > 2)
     {
       params.log_fs << "Connected component labeling completed on each subsection." << endl;
     }

    // Do connected component labeling along the processing window seams
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

     get_seam_index_data(recur_level,section,params.recur_mask_flags[recur_level],spatial_data,
                         ncols,nrows,nslices,col_seam_index_data,row_seam_index_data,
                         slice_seam_index_data,temp_data);
     connected_component_seams(recur_level,section,nb_sections,params.recur_mask_flags[recur_level],nslevels,ncols,nrows,nslices,
                               col_seam_index_data,row_seam_index_data,slice_seam_index_data,
                               region_objects,spatial_data,temp_data);
#else
     get_seam_index_data(recur_level,section,params.recur_mask_flags[recur_level],spatial_data,
                         ncols,nrows,col_seam_index_data,row_seam_index_data,
                         temp_data);
     connected_component_seams(recur_level,section,nb_sections,params.recur_mask_flags[recur_level],nslevels,ncols,nrows,
                               col_seam_index_data,row_seam_index_data,
                               region_objects,spatial_data,temp_data);
#endif
   }

   map<unsigned int,unsigned int> region_object_relabel_pairs;
   unsigned int region_object_label, merge_region_object_index, merge_region_label;
   for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
   {
     region_object_label = region_object_index + 1;
     merge_region_label = region_objects[region_object_index].get_merge_region_label();
     while (merge_region_label != region_object_label)
     {
       region_object_label = merge_region_label;
       merge_region_object_index = merge_region_label - 1;
       merge_region_label = region_objects[merge_region_object_index].get_merge_region_label();
     }
     region_objects[region_object_index].set_merge_region_label(region_object_label);
   }
   for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
   {
     region_object_label = region_object_index + 1;
     merge_region_label = region_objects[region_object_index].get_merge_region_label();
     if (merge_region_label != region_object_label)
       region_object_relabel_pairs.insert(make_pair(region_object_label,merge_region_label));
   }
   if (!(region_object_relabel_pairs.empty()))
#ifdef THREEDIM
     do_region_object_relabel(recur_level, section, region_object_relabel_pairs,
                            spatial_data, ncols, nrows, nslices, temp_data);
#else
     do_region_object_relabel(recur_level, section, region_object_relabel_pairs,
                            spatial_data, ncols, nrows, temp_data);
#endif

#ifdef DEBUG
   int col, row;
#ifdef THREEDIM
   int slice;
#endif
   unsigned int pixel_index;
   if (params.debug > 3)
   {
     params.log_fs << endl << "After connected component labeling, dump of spatial_data.region_object_label:" << endl << endl;
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
   return;
 }

/*-----------------------------------------------------------
|
|  Routine Name: connected_component_seams
|
|       Purpose: Performs connected component labeling along processing window seams
|
|         Input: recur_level      (Current recursive level of this task)
|                section          (Section or window processed by this call to lrhseg)
|                nb_sections      (Number of data sections (or tasks) covered by this task)
|                seam_flag        (Seam selection flag)
|                nslevels         (Number of segmentation levels saved)
|                ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                nslices          (Number of slices in pixel_data)
|                col_seam_index_data   (Index class information from the processing window column seam)
|                row_seam_index_data   (Index class information from the processing window row seam)
|                slice_seam_index_data (Index class information from the processing window slice seam)
|                region_objects (Class which holds connected region related information)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                
|        Output:
|
|       Returns:
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: October 10, 2003
| Modifications: (See comments for first instance of connected_component_init)
|
------------------------------------------------------------*/
#ifdef THREEDIM
 void connected_component_seams(const short unsigned int& recur_level, const short unsigned int& section,
                                const short unsigned int& nb_sections, const unsigned char& seam_flag,
                                const short unsigned int& nslevels,
                                const int& ncols, const int& nrows, const int& nslices,
                                vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                                vector<Index>& slice_seam_index_data, vector<RegionObject>& region_objects,
                                Spatial& spatial_data, Temp& temp_data)
#else
 void connected_component_seams(const short unsigned int& recur_level, const short unsigned int& section,
                                const short unsigned int& nb_sections, const unsigned char& seam_flag,
                                const short unsigned int& nslevels,
                                const int& ncols, const int& nrows,
                                vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                                vector<RegionObject>& region_objects,
                                Spatial& spatial_data, Temp& temp_data)
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
   int value;
   if (params.debug > 3)
   {
     if (col_seam_flag)
     {
       params.log_fs << endl << "Entering connected_component_seams, col_seam_index_data:" << endl << endl;
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
       params.log_fs << endl << "Entering connected_component_seams, row_seam_index_data:" << endl << endl;
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
       params.log_fs << endl << "Entering connected_component_seams, slice_seam_index_data:" << endl << endl;
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
       params.log_fs << endl << "Entering connected_component_seams, col_seam_index_data.region_object_label:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < params.seam_size; col++)
           {
#ifdef THREEDIM
             value = (int) col_seam_index_data[row + slice*nrows + col*nslices*nrows].get_region_object_label();
#else
             value = (int) col_seam_index_data[row + col*nrows].get_region_object_label();
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
       params.log_fs << endl << "Entering connected_component_seams, row_seam_index_data.region_object_label:" << endl << endl;
#ifdef THREEDIM
       for (slice = 0; slice < nslices; slice++)
       {
#endif
         for (row = 0; row < params.seam_size; row++)
         {
           for (col = 0; col < ncols; col++)
           {
#ifdef THREEDIM
             value = (int) row_seam_index_data[col + slice*ncols + row*nslices*ncols].get_region_object_label();
#else
             value = (int) row_seam_index_data[col + row*ncols].get_region_object_label();
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
       params.log_fs << endl << "Entering connected_component_seams, slice_seam_index_data.region_object_label:" << endl << endl;
       for (slice = 0; slice < params.seam_size; slice++)
       {
         for (row = 0; row < nrows; row++)
         {
           for (col = 0; col < ncols; col++)
           {
             value = (int) slice_seam_index_data[col + row*ncols + slice*nrows*ncols].get_region_object_label();
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
   short unsigned int nbdir, maxnbdir = params.object_maxnbdir;
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
   unsigned int region_class_label, nbregion_class_label, merge_region_label;
   unsigned int region_object_label, nbregion_object_label, region_object_index, nbregion_object_index;
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
         region_class_label = col_seam_index_data[col_seam_index].get_region_class_label();
         if (region_class_label > 0)
         {
           for (nbdir = 0; nbdir < params.object_maxnbdir; nbdir++)
           {
             nbregion_class_label = 0;
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
               nbregion_class_label = col_seam_index_data[col_seam_nbindex].get_region_class_label();
               if ((nbdir < maxnbdir) && (nbregion_class_label > 0) && (nbregion_class_label != region_class_label))
                 col_seam_index_data[col_seam_index].set_boundary_map(nslevels+1);
             }
             if (nbregion_class_label == region_class_label)
             {
               region_object_label = col_seam_index_data[col_seam_index].get_region_object_label();
               region_object_index = region_object_label - 1;
               while (region_objects[region_object_index].merge_region_label != region_object_label)
               {
                 region_object_label = region_objects[region_object_index].merge_region_label;
                 region_object_index = region_object_label - 1;
               }
               nbregion_object_label = col_seam_index_data[col_seam_nbindex].get_region_object_label();
               nbregion_object_index = nbregion_object_label - 1;
               while (region_objects[nbregion_object_index].merge_region_label != nbregion_object_label)
               {
                 nbregion_object_label = region_objects[nbregion_object_index].merge_region_label;
                 nbregion_object_index = nbregion_object_label - 1;
               }
              //  Can't have region_object_label = 0!!
               if (region_object_label != nbregion_object_label)
               {
                 merge_region_label = region_object_label;
                 if (nbregion_object_label < merge_region_label)
                   merge_region_label = nbregion_object_label;
                 if (region_object_label != merge_region_label)
                 {
                   region_objects[region_object_index].active_flag = false;
                   region_objects[region_object_index].merge_region_label = merge_region_label;
#ifdef DEBUG
                   if (params.debug > 3)
                   {
#ifdef THREEDIM
                     params.log_fs << "At (col,row,slice) (" << col << "," << row << "," << slice << "), region_object label ";
#else
                     params.log_fs << "At (col,row) (" << col << "," << row << "), region_object label ";
#endif
                     params.log_fs << region_object_label << " merge into region " << merge_region_label << endl;
                   }
#endif
                 }
                 if (nbregion_object_label != merge_region_label)
                 {
                   region_objects[nbregion_object_index].active_flag = false;
                   region_objects[nbregion_object_index].merge_region_label = merge_region_label;
#ifdef DEBUG
                   if (params.debug > 3)
                   {
#ifdef THREEDIM
                     params.log_fs << "At (col,row,slice) (" << col << "," << row << "," << slice << "), nbregion_object label ";
#else
                     params.log_fs << "At (col,row) (" << col << "," << row << "), nbregion_object label ";
#endif
                     params.log_fs << nbregion_object_label << " merge into region " << merge_region_label << endl;
                   }
#endif
                 }
               }
             } // if (nbregion_class_label == region_class_label)
           } // for (nbdir = 0; nbdir < params.object_maxnbdir; nbdir++)
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
         region_class_label = row_seam_index_data[row_seam_index].get_region_class_label();
         if (region_class_label > 0)
         {
           for (nbdir = 0; nbdir < params.object_maxnbdir; nbdir++)
           {
             nbregion_class_label = 0;
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
               nbregion_class_label = row_seam_index_data[row_seam_nbindex].get_region_class_label();
               if ((nbdir < maxnbdir) && (nbregion_class_label > 0) && (nbregion_class_label != region_class_label))
                 row_seam_index_data[row_seam_index].set_boundary_map(nslevels+1);
             }
             if (nbregion_class_label == region_class_label)
             {
               region_object_label = row_seam_index_data[row_seam_index].get_region_object_label();
               region_object_index = region_object_label - 1;
               while (region_objects[region_object_index].merge_region_label != region_object_label)
               {
                 region_object_label = region_objects[region_object_index].merge_region_label;
                 region_object_index = region_object_label - 1;
               }
               nbregion_object_label = row_seam_index_data[row_seam_nbindex].get_region_object_label();
               nbregion_object_index = nbregion_object_label - 1;
               while (region_objects[nbregion_object_index].merge_region_label != nbregion_object_label)
               {
                 nbregion_object_label = region_objects[nbregion_object_index].merge_region_label;
                 nbregion_object_index = nbregion_object_label - 1;
               }
             //  Can't have region_object_label = 0!!
               if (region_object_label != nbregion_object_label)
               {
                 merge_region_label = region_object_label;
                 if (nbregion_object_label < merge_region_label)
                   merge_region_label = nbregion_object_label;
                 if (region_object_label != merge_region_label)
                 {
                   region_objects[region_object_index].active_flag = false;
                   region_objects[region_object_index].merge_region_label = merge_region_label;
#ifdef DEBUG
                   if (params.debug > 3)
                   {
#ifdef THREEDIM
                     params.log_fs << "At (col,row,slice) (" << col << "," << row << "," << slice << "), region_object label ";
#else
                     params.log_fs << "At (col,row) (" << col << "," << row << "), region_object label ";
#endif
                     params.log_fs << region_object_label << " merge into region " << merge_region_label << endl;
                   }
#endif
                 }
                 if (nbregion_object_label != merge_region_label)
                 {
                   region_objects[nbregion_object_index].active_flag = false;
                   region_objects[nbregion_object_index].merge_region_label = merge_region_label;
#ifdef DEBUG
                   if (params.debug > 3)
                   {
#ifdef THREEDIM
                     params.log_fs << "At (col,row,slice) (" << col << "," << row << "," << slice << "), nbregion_object label ";
#else
                     params.log_fs << "At (col,row) (" << col << "," << row << "), nbregion_object label ";
#endif
                     params.log_fs << nbregion_object_label << " merge into region " << merge_region_label << endl;
                   }
#endif
                 }
               }
             } // if (nbregion_class_label == region_class_label)
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
         region_class_label = slice_seam_index_data[slice_seam_index].get_region_class_label();
         if (region_class_label > 0)
         {
           for (nbdir = 0; nbdir < params.object_maxnbdir; nbdir++)
           {
             nbregion_class_label = 0;
             find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
             if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
                 (nbslice<params.seam_size) && (nbrow<nrows) && (nbcol<ncols))
             {
               slice_seam_nbindex = nbcol + nbrow*ncols + nbslice*nrows*ncols;
               nbregion_class_label = slice_seam_index_data[slice_seam_nbindex].get_region_class_label();
               if ((nbdir < maxnbdir) && (nbregion_class_label > 0) && (nbregion_class_label != region_class_label))
                 slice_seam_index_data[slice_seam_index].set_boundary_map(nslevels+1);
             }
             if (nbregion_class_label == region_class_label)
             {
               region_object_label = slice_seam_index_data[slice_seam_index].get_region_object_label();
               region_object_index = region_object_label - 1;
               while (region_objects[region_object_index].merge_region_label != region_object_label)
               {
                 region_object_label = region_objects[region_object_index].merge_region_label;
                 region_object_index = region_object_label - 1;
               }
               nbregion_object_label = slice_seam_index_data[slice_seam_nbindex].get_region_object_label();
               nbregion_object_index = nbregion_object_label - 1;
               while (region_objects[nbregion_object_index].merge_region_label != nbregion_object_label)
               {
                 nbregion_object_label = region_objects[nbregion_object_index].merge_region_label;
                 nbregion_object_index = nbregion_object_label - 1;
               }
             //  Can't have region_object_label = 0!!
               if (region_object_label != nbregion_object_label)
               {
                 merge_region_label = region_object_label;
                 if (nbregion_object_label < merge_region_label)
                   merge_region_label = nbregion_object_label;
                 if (region_object_label != merge_region_label)
                 {
                   region_objects[region_object_index].active_flag = false;
                   region_objects[region_object_index].merge_region_label = merge_region_label;
#ifdef DEBUG
                   if (params.debug > 3)
                   {
                     params.log_fs << "At (col,row,slice) (" << col << "," << row << "," << slice << "), region_object label ";
                     params.log_fs << region_object_label << " merge into region " << merge_region_label << endl;
                   }
#endif
                 }
                 if (nbregion_object_label != merge_region_label)
                 {
                   region_objects[nbregion_object_index].active_flag = false;
                   region_objects[nbregion_object_index].merge_region_label = merge_region_label;
#ifdef DEBUG
                   if (params.debug > 3)
                   {
                     params.log_fs << "At (col,row,slice) (" << col << "," << row << "," << slice << "), nbregion_object label ";
                     params.log_fs << nbregion_object_label << " merge into region " << merge_region_label << endl;
                   }
#endif
                 }
               }
             } // if (nbregion_class_label == region_class_label)
           }
         }
       }
   } // if (slice_seam_flag)
#endif
   unsigned int short_buf_size, int_buf_size;
#ifdef THREEDIM
#ifdef PARALLEL
   bool section_flags[nb_sections];
#endif
   short_buf_size = int_buf_size = params.seam_size*(nslices*nrows + nslices*ncols + nrows*ncols);
#else // THREEDIM
#ifdef PARALLEL
   bool section_flags[nb_sections];
#endif
   short_buf_size = int_buf_size = params.seam_size*(ncols + nrows);
#endif // THREEDIM
   check_buf_size(0,short_buf_size,int_buf_size,0,0,temp_data);

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
