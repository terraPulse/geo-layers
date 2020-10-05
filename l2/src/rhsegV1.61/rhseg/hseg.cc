/*-----------------------------------------------------------
|
|  Routine Name: hseg - Hierarchical Segmentation
|
|       Purpose: Main function for the Hierarchical Segmentation (HSeg) program
|
|         Input: 
|
|        Output: 
|
|       Returns: TRUE (1) on success, FALSE (0) on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: December 9, 2002.
| Modifications: February 25, 2003 - Modified output to output parameter file (-oparam)
|                August 20, 2003 - Reorganized program and added user controlled byteswapping
|                September 24, 2003 - Eliminated use of the index_class class (index_data)
|                October 18, 2003 - Added boundary_npix_file option
|                October 31, 2003 - Added subrlblmap_file and subregmerges_file options
|                October 31, 2003 - Eliminated use of final_region_list
|                November 5, 2003 - Modified Params structure and made class member variables all private
|                November 26, 2003 - Added output paramater file for connected regions (-conn_oparam)
|                March 2, 2004 - Added boundary_map option
|                May 17, 2004 - Modified calculation of critval and added global_dissim
|                January 4, 2004 - Changed from sorting lists to utilizing a heap structure
|                May 9, 2005 - Added code for integration with VisiQuest
|                May 29, 2005 - Added temporary file I/O for faster processing of large data sets
|                August 9, 2005 - Added "conv_criterion" and "gstd_dev_crit_flag" parameters
|                October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                December 23, 2005 - Changed sorting for region labeling to be by distance from minimum vector
|                November 1, 2007 - Revised method for selecting iterations at which segmentations are output,
|                                   eliminating the "conv_criterion" and "conv_factor" parameters.
|                                   This revision also eliminated the "gdissim_crit" and "gstd_dev_crit"
|                                   parameters and added the "gdissim_flag" parameter
|                November 1, 2007 - Removed VisiQuest related code
|                November 9, 2007 - Combined region feature output into region_object list file
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                August 8, 2008 - Added hseg_out_thresholds and hseg_out_nregions parameters.
|                July 30, 2009 - Added inclusion of C++ algorithm library <algorithm>
|                January 10, 2010 - Revised nghbr_heap and region_heap initialization
|		 January 11, 2010 - Implemented new definition of min_npixels parameter.
|                October 29, 2010 - Revised utilization of min_npixels parameter at recur_level = 0 to ensure that
|                                   min_npixels reaches 1 before the hierarchical segmentation results are output.
|                December 28, 2010 - Added spclust_min parameter.
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|
------------------------------------------------------------*/

#include "hseg.h"
#include <params/params.h>
#include <spatial/spatial.h>
#include <pixel/pixel.h>
#include <region/region_class.h>
#include <region/region_object.h>
#include <results/results.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#ifdef DEBUG
#include <ctime>
#endif
#ifdef GTKMM
#include <pthread.h>
#endif

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{
#ifdef GTKMM
 void *hseg_thread(void *threadid)
 {
   params.gtkmm_flag = hseg();
   oparams.percent_complete = 100;
   pthread_exit(NULL);
   return threadid;
 }
#endif
 bool hseg()
 {
// Declaration of temp_data structure and elements - used for both data I/O
// to and from temporary data files (serial) and for data transfer between
// processing tasks (parallel).
   Temp temp_data;
   temp_data.byte_buf_size = 0;
   temp_data.byte_buffer = NULL;
   temp_data.short_buf_size = 0;
   temp_data.short_buffer = NULL;
   temp_data.int_buf_size = 0;
   temp_data.int_buffer = NULL;
   temp_data.float_buf_size = 0;
   temp_data.float_buffer = NULL;
   temp_data.double_buf_size = 0;
   temp_data.double_buffer = NULL;
#ifdef PARALLEL
// Allocate data temp buffer (for Packed communication between parallel tasks)
   temp_data.buf_size = MAX_TEMP_SIZE;
   temp_data.buffer = new char[temp_data.buf_size];
#ifdef TIME_IT
   float end_time, elapsed_time;
   temp_data.setup = 0.0;
   temp_data.wait = 0.0;
   temp_data.transfer = 0.0;
   temp_data.compute = 0.0;
   temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
#endif

// Declare spatial (image) data
   Spatial spatial_data;

// Declare pixel_data.
   Pixel::set_static_vals();
#ifdef THREEDIM
   unsigned int npixels = params.pixel_ncols*params.pixel_nrows*params.pixel_nslices;
#else
   unsigned int npixels = params.pixel_ncols*params.pixel_nrows;
#endif
   vector<Pixel> pixel_data(npixels,Pixel());

// Obtain input data
   spatial_data.read_data(pixel_data,temp_data);

// Compute the scale and offset parameters
   oparams.tot_npixels = scale_offset(pixel_data,temp_data);
   unsigned int nregions = oparams.tot_npixels;

// Declare region_classes.
   RegionClass::set_static_vals();
   vector<RegionClass> region_classes;

#ifdef DEBUG
   time_t now;
   if (params.debug > 2)
   {
     now = time(NULL);
     params.log_fs << "Calling RHSeg at " << ctime(&now) << endl;
   }
#endif

#ifdef PARALLEL
#ifdef TIME_IT
   end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
   elapsed_time = end_time - temp_data.start_time;
   if (elapsed_time > 0.0) temp_data.setup += elapsed_time;
   temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
#endif
   double max_threshold = 0.0;
   vector<RegionClass *> nghbr_heap;
   vector<RegionClass *> region_heap;
   params.min_npixels = 1;

   nregions = rhseg(nregions, max_threshold, spatial_data, pixel_data, region_classes,
                    nghbr_heap, region_heap, temp_data);
#ifdef PARALLEL
   if (params.recur_level > 0)
   {
     delete [ ] temp_data.byte_buffer;
     temp_data.byte_buffer = NULL;
     temp_data.byte_buf_size = 0;
     delete [ ] temp_data.short_buffer;
     temp_data.short_buffer = NULL;
     temp_data.short_buf_size = 0;
     delete [ ] temp_data.int_buffer;
     temp_data.int_buffer = NULL;
     temp_data.int_buf_size = 0;
     delete [ ] temp_data.float_buffer;
     temp_data.float_buffer = NULL;
     temp_data.float_buf_size = 0;
     delete [ ] temp_data.double_buffer;
     temp_data.double_buffer = NULL;
     temp_data.double_buf_size = 0;
     return true;
   }
 // The only task that continues to this point and beyond is the task that initiated
 // overall chain of recursive calls, i.e., the task at params.recur_level = 0.
#endif

// NOTE: Region labeling is guaranteed compact exiting rhseg.
   if (params.debug > 1)
   {
     params.log_fs << endl << "Returned to main HSeg function with the number of regions = " << nregions << endl;
     params.log_fs << "and maximum merging threshold = " << max_threshold;
#ifdef DEBUG
     now = time(NULL);
     params.log_fs << " at " << ctime(&now) << endl;
#else
     params.log_fs << endl;
#endif
   }
   unsigned int region_index, region_classes_size;
   region_classes_size = nregions;
   if (region_classes.size() != region_classes_size)
     region_classes.resize(region_classes_size);

#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < nregions; ++region_index)
       region_classes[region_index].print(region_classes);
   }
#endif

   short unsigned int section = 0;
   int stride, nb_sections;
   set_stride_sections(params.recur_level,stride,nb_sections);

   unsigned int region_label;
   double threshold;
   map<unsigned int,unsigned int> region_relabel_pairs;
   unsigned int converge_nregions;
   bool process_flag;
   if (params.chk_nregions_flag)
   {
     process_flag = (nregions > params.chk_nregions);
     converge_nregions = params.chk_nregions;
   }
   else if (params.hseg_out_nregions_flag)
   {
     process_flag = (nregions > params.hseg_out_nregions[0]);
     converge_nregions = params.hseg_out_nregions[0];
   }
   else if (params.hseg_out_thresholds_flag)
   {
     process_flag = (max_threshold < params.hseg_out_thresholds[0]);
     converge_nregions = params.conv_nregions;
   }
   else
   {
     process_flag = (nregions > 0);
     converge_nregions = 0;
     params.chk_nregions_flag = true;
   }

   process_flag = process_flag || ((nregions > params.conv_nregions) && (max_threshold == 0.0));
   if (process_flag)
   {
     if (params.debug > 1)
     {
       params.log_fs << endl << "Calling HSeg with the number of regions = " << nregions << "," << endl;
       params.log_fs << "maximum merging threshold = " << max_threshold;
       params.log_fs << ", and converge_nregions = " << converge_nregions;
#ifdef DEBUG
       now = time(NULL);
       params.log_fs << " at " << ctime(&now) << endl;
#else
       params.log_fs << endl;
#endif
     }

     params.min_npixels = 1;
     threshold = max_threshold;
     lhseg(params.recur_level, true, section, converge_nregions, nregions, threshold, 
           pixel_data, region_classes, nghbr_heap, region_heap, temp_data);
     if (params.debug > 0)
     {
       params.log_fs << endl << "Exited call to HSeg in main HSeg function with the number of regions = " << nregions;
#ifdef DEBUG
       now = time(NULL);
       params.log_fs << " at " << ctime(&now) << endl;
#else
       params.log_fs << endl;
#endif
     }

     region_classes_size = region_classes.size();
     if (threshold > max_threshold)
       max_threshold = threshold;
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "Exiting lhseg, dump of the region data:" << endl << endl;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         if (region_classes[region_index].get_active_flag())
           region_classes[region_index].print(region_classes);
     }
#endif
   } // if (process_flag)

  // Sort region_classes by distance from minimum vector and erase unneeded elements
   region_classes_size = region_classes.size();
   unsigned int compact_region_index = 0;
   vector<RegionClass> compact_region_classes(nregions,RegionClass());
   for (region_index = 0; region_index < region_classes_size; ++region_index)
   {
     if (region_classes[region_index].get_active_flag())
     {
       region_classes[region_index].set_min_region_dissim();
       compact_region_classes[compact_region_index++] = region_classes[region_index];
     }
     if (compact_region_index == nregions)
       break;
   }
   region_classes_size = nregions;
   region_classes.resize(region_classes_size);

   if (params.sort_flag)
   {
     sort(compact_region_classes.begin(), compact_region_classes.end(), DistToMin());
  //   sort(compact_region_classes.begin(), compact_region_classes.end(), NpixMoreThan());
   }

 // Renumber region_classes from darkest region to brightest, set up region_relabel_pairs.
   for (region_index = 0; region_index < nregions; ++region_index)
   {
     region_classes[region_index] = compact_region_classes[region_index];
     region_label = region_classes[region_index].get_label();
     region_classes[region_index].set_label(region_index+1);
     if (region_label != region_classes[region_index].get_label())
       region_relabel_pairs.insert(make_pair(region_label,region_classes[region_index].get_label()));
   }

   if (!(region_relabel_pairs.empty()))
   {
   // Use region_relabel_pairs to renumber the nghbrs_label_set for each region.
     for (region_index = 0; region_index < nregions; ++region_index)
       region_classes[region_index].nghbrs_label_set_renumber(region_relabel_pairs);

   // Use region_relabel_pairs to renumber region_label in pixel_data.
     do_region_class_relabel(params.recur_level,section,region_relabel_pairs,pixel_data,temp_data);
   }

  // NOTE:  region labeling is compact at this point
   unsigned int heap_index, nghbr_heap_size = 0;
   nghbr_heap.clear();
   for (region_index = 0; region_index < nregions; ++region_index)
   {
     region_classes[region_index].clear_best_merge();
     nghbr_heap.push_back(&region_classes[region_index]);
     nghbr_heap_size++;
   }
   nghbr_heap.push_back(NULL);

   for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
     nghbr_heap[heap_index]->best_nghbr_init(region_classes);
   make_nghbr_heap(nghbr_heap, nghbr_heap_size);

   unsigned int prev_min_npixels, max_min_npixels;
   unsigned int save_region_heap_size, prev_region_heap_size, region_heap_size;
   unsigned int spclust_min, spclust_min_factor = 2;
   unsigned int spclust_max, spclust_max_factor = 6;
   set<RegionClass *> update_nghbrs_set;
   if (params.spclust_wght_flag)
   {
    // Find proper value for min_npixels
     prev_min_npixels = params.min_npixels;
     params.min_npixels = 0;
     region_heap_size = params.spclust_max + 1;
     while (region_heap_size > params.spclust_max)
     {
       params.min_npixels++;
       region_heap_size = 0;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
       {
         if ((region_classes[region_index].get_active_flag()) && 
             (region_classes[region_index].get_npix() >= params.min_npixels))
           region_heap_size++;
         if (region_heap_size > params.spclust_max)
           break;
       }
     } // while (region_heap_size > params.spclust_max)

     if ((params.min_npixels > 1) && (region_heap_size < params.spclust_min))
     {
       params.min_npixels--;
       save_region_heap_size = region_heap_size;
       region_heap_size = 0;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
       {
         if ((region_classes[region_index].get_active_flag()) && 
             (region_classes[region_index].get_npix() >= params.min_npixels))
            region_heap_size++;
       }
       if ((region_heap_size > (spclust_max_factor*params.spclust_max)) && (save_region_heap_size > 1))
       {
         region_heap_size = save_region_heap_size;
         params.min_npixels++;
       }
     } // if ((params.min_npixels > 1) && (region_heap_size < params.spclust_min))

     while (region_heap_size < 2)
     {
       params.min_npixels--;
       region_heap_size = 0;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
       {
         if ((region_classes[region_index].get_active_flag()) && 
             (region_classes[region_index].get_npix() >= params.min_npixels))
           region_heap_size++;
       }
     } // while (region_heap_size < 2)

   // Initialize region_heap
     region_heap.clear();
     region_heap_size = 0;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
     {
       if (region_classes[region_index].get_active_flag())
       {
         region_classes[region_index].set_region_heap_index(UINT_MAX);
         if (region_classes[region_index].get_npix() >= params.min_npixels)
         {
           region_classes[region_index].clear_best_region();
           region_heap.push_back(&region_classes[region_index]);
           region_heap_size++;
         }
       }
     }

     region_heap.push_back(NULL);

     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->best_region_init(heap_index,region_heap,region_heap_size);
     make_region_heap(region_heap, region_heap_size);

    // Update nghbr_heap (if necessary)
     if ((params.merge_accel_flag) && (params.min_npixels != prev_min_npixels))
     {
       max_min_npixels = params.min_npixels;
       if (max_min_npixels < prev_min_npixels)
         max_min_npixels = prev_min_npixels;
       update_nghbrs_set.clear();
       for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
       {
         if (nghbr_heap[heap_index]->get_npix() < max_min_npixels)
           update_nghbrs_set.insert(nghbr_heap[heap_index]);
       }
       update_nghbrs(update_nghbrs_set, nghbr_heap, nghbr_heap_size, region_classes);
     }

     if (params.debug > 0)
     {
       params.log_fs << "After creation of final region classes, min_npixels set to " << params.min_npixels;
       params.log_fs << " (region_heap_size = " << region_heap_size << ")." << endl;
     }
   } // if (params.spclust_wght_flag)
   else
   {
     region_heap_size = 0;
     region_heap.push_back(NULL);
   }

#ifdef DEBUG
   if (params.debug > 2)
   {
     params.log_fs << endl << "After creation of final region_classes, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < nregions; ++region_index)
       region_classes[region_index].print(region_classes);
   }
   unsigned int pixel_index, pixel_data_size = pixel_data.size();
   if (params.debug > 3)
   {
     params.log_fs << endl << "After creation of final region_classes, dump of the pixel data:" << endl << endl;
     for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
     {
       pixel_data[pixel_index].print(pixel_index);
     }
   }
   if (params.debug > 3)
   {
     params.log_fs << endl << "After creation of final region_classes, dump of the pixel data.region_label:" << endl << endl;
#ifdef THREEDIM
     for (int slice = 0; slice < params.padded_nslices; slice++)
     {
#endif
      for (int row = 0; row < params.padded_nrows; row++)
      {
       for (int col = 0; col < params.padded_ncols; col++)
       {
#ifdef THREEDIM
         pixel_index = col + row*params.padded_ncols + slice*params.padded_nrows*params.padded_ncols;
#else
         pixel_index = col + row*params.padded_ncols;
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

   float global_dissim = 0.0, numpix = 0.0;
   float region_numpix;
   if (params.gdissim_flag)
   {
     if ((params.dissim_crit == 1) || (params.dissim_crit == 2) || (params.dissim_crit == 3) ||
         (params.dissim_crit == 4) || (params.dissim_crit == 5) || (params.dissim_crit == 8) ||
         (params.dissim_crit == 10))
     {
       for (region_index = 0; region_index < nregions; ++region_index)
         region_classes[region_index].set_sum_pixel_gdissim(0.0);
       update_sum_pixel_gdissim(params.recur_level,section,pixel_data,region_classes,temp_data);
     }

     for (region_index = 0; region_index < nregions; ++region_index)
     {
       if ((params.dissim_crit == 6) || (params.dissim_crit == 7) || (params.dissim_crit == 9))
         region_classes[region_index].calc_sum_pixel_gdissim();
       region_numpix = region_classes[region_index].get_npix();
       numpix += region_numpix;
       global_dissim += (float) region_classes[region_index].get_sum_pixel_gdissim();
#ifdef DEBUG
       if (params.debug > 3)
       {
         params.log_fs << "For region label " << region_classes[region_index].get_label();
         params.log_fs << ", sum_pixel_gdissim = ";
         params.log_fs << region_classes[region_index].get_sum_pixel_gdissim();
         params.log_fs << " and npix = " << region_classes[region_index].get_npix() << endl;
       }
#endif
     }
     if ((params.dissim_crit == 6) || (params.dissim_crit == 7))
     {
       global_dissim /= (numpix-1.0);
#ifdef MSE_SQRT
       global_dissim = sqrt(global_dissim); // Added to make dimensionality consistent
#endif
     }
     else
       global_dissim /= numpix;
   }

   short unsigned int hlevel=0;
   Results results_data;
   RegionObject::set_static_vals();
   unsigned int region_objects_size=0, region_object_index;
   unsigned int region_object_label;
   unsigned int nb_objects=0;
   vector<RegionObject> region_objects;
   map<unsigned int,unsigned int> region_object_relabel_pairs;
   spatial_data.update_region_label_map(params.recur_level,section,pixel_data,temp_data);
   if (params.region_nb_objects_flag)
   {
     params.set_object_maxnbdir();
#ifdef THREEDIM
     connected_component_init(params.recur_level,section,params.padded_ncols,params.padded_nrows,params.padded_nslices,
                              nb_objects,region_objects,spatial_data,temp_data);
     nb_objects = do_region_objects_init(params.recur_level,section,params.padded_ncols,
                                         params.padded_nrows,params.padded_nslices,
                                         pixel_data,spatial_data,region_objects,temp_data);
#else
     connected_component_init(params.recur_level,section,params.padded_ncols,params.padded_nrows,
                              nb_objects,region_objects,spatial_data,temp_data);
     nb_objects = do_region_objects_init(params.recur_level,section,
                                         params.padded_ncols,params.padded_nrows,
                                         pixel_data,spatial_data,region_objects,temp_data);
#endif
     region_objects_size = region_objects.size();
     if (params.sort_flag)
     {
     // Set min_region_dissim
       for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
         if (region_objects[region_object_index].get_active_flag())
           region_objects[region_object_index].set_min_region_dissim();
     // Sort region_objects by distance from minimum vector
       sort(region_objects.begin(), region_objects.end(), ObjectDistToMin());
     // Renumber region_objects from darkest region to brightest, set up region_object_relabel_pairs.
       region_object_relabel_pairs.clear();
       for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
       {
         region_object_label = region_objects[region_object_index].get_label();
         region_object_relabel_pairs.insert(make_pair(region_object_label,(region_object_index+1)));
         region_object_label = region_object_index + 1;
         region_objects[region_object_index].set_label(region_object_label);
         region_objects[region_object_index].set_merge_region_label(region_object_label);
         region_objects[region_object_index].clear_region_object_info();
       }
     // Use do_region_object_relabel_pairs to renumber region_object_label_map in spatial_data.
       if (!(region_object_relabel_pairs.empty()))
#ifdef THREEDIM
         do_region_object_relabel(params.recur_level,section,region_object_relabel_pairs,spatial_data,
                                  params.padded_ncols,params.padded_nrows,params.padded_nslices,
                                  temp_data);
#else
         do_region_object_relabel(params.recur_level,section,region_object_relabel_pairs,spatial_data,
                                  params.padded_ncols,params.padded_nrows,
                                  temp_data);
#endif
     } // if (params.sort_flag)

     for (region_index = 0; region_index < nregions; ++region_index)
       region_classes[region_index].clear_region_info();
     update_region_object_info(params.recur_level,section,hlevel,spatial_data,region_objects,temp_data);
     update_region_class_info(params.recur_level,section,hlevel,spatial_data,region_classes,temp_data);
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "After call to update_region_class_info, dump of the pixel data:" << endl << endl;
       for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
       {
         pixel_data[pixel_index].print(pixel_index);
       }
     }
#endif
   }
   else if ((params.region_boundary_npix_flag) || (params.boundary_map_flag))
   {
     for (region_index = 0; region_index < nregions; ++region_index)
       region_classes[region_index].clear_region_info();
#ifdef THREEDIM
     boundary_map(params.recur_level,section,params.padded_ncols,params.padded_nrows,params.padded_nslices,
                  0,spatial_data,temp_data);
#else
     boundary_map(params.recur_level,section,params.padded_ncols,params.padded_nrows,
                  0,spatial_data,temp_data);
#endif
     update_region_class_info(params.recur_level,section,hlevel,spatial_data,region_classes,temp_data);
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "After call to update_region_class_info, dump of the pixel data:" << endl << endl;
       for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
       {
         pixel_data[pixel_index].print(pixel_index);
       }
     }
#endif
   }
   if (params.debug > 2)
   {
     params.log_fs << endl << "Initiating hierarchical segmentation output at nregions = " << nregions;
     params.log_fs << ", with maximum merging threshold = " << max_threshold << endl;
     if (params.gdissim_flag)
       params.log_fs << " with global dissimilarity value = " << global_dissim << endl;
   }

#ifdef PARALLEL
#ifdef TIME_IT
   if (params.myid == 0)
   {
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     if (params.debug > 0)
     {
       params.log_fs << "setup time: " << temp_data.setup << endl;
       params.log_fs << "compute time: " << temp_data.compute << endl;
       params.log_fs << "transfer time: " << temp_data.transfer << endl;
       params.log_fs << "wait time: " << temp_data.wait << endl;
     }
     else
     {
       cout << "setup time: " << temp_data.setup << endl;
       cout << "compute time: " << temp_data.compute << endl;
       cout << "transfer time: " << temp_data.transfer << endl;
       cout << "wait time: " << temp_data.wait << endl;
     }
   }
#endif
#endif

  // Open output files
   results_data.set_buffer_sizes(params.nbands,nregions,nb_objects);
   results_data.open_output(params.region_classes_file,params.region_objects_file);
#ifndef GDAL
   spatial_data.open_region_label_map_output();
#endif
   if (params.debug > 0)
   {
     params.log_fs << endl << "Data written at hlevel = " << hlevel << " for " << nregions << " region classes,";
     params.log_fs << " with maximum merging threshold = " << max_threshold;
     if (params.gdissim_flag)
       params.log_fs << endl << "and with global dissimilarity value = " << global_dissim << ".  ";
     else
       params.log_fs << "." << endl;
     if (params.region_nb_objects_flag)
       params.log_fs << "There are " << nb_objects << " region objects.  ";
     if ((params.min_npixels > 1) && (region_heap_size > 0))
       params.log_fs << " (large_nregions = " << region_heap_size << ")" << endl;
     else
       params.log_fs << endl;
     params.log_fs << endl;
   }
   else
   {
     cout << endl << "Data written at hlevel = " << hlevel << " for " << nregions << " region classes,";
     cout << " with maximum merging threshold = " << max_threshold;
     if (params.gdissim_flag)
       cout << endl << "and with global dissimilarity value = " << global_dissim << ".  ";
     else
       cout << "." << endl;
     if (params.region_nb_objects_flag)
       cout << "There are " << nb_objects << " region objects.  " << endl;
     if ((params.min_npixels > 1) && (region_heap_size > 0))
       cout << " (large_nregions = " << region_heap_size << ")" << endl;
     else
       cout << endl;
     cout << endl;
   }

  // Write data.
   results_data.write(hlevel,nregions,nb_objects,region_classes,region_objects);
   oparams.int_buffer_size.push_back(results_data.get_int_buffer_index());
   oparams.max_threshold.push_back(max_threshold);
   if (params.gdissim_flag)
     oparams.gdissim.push_back(global_dissim);
   spatial_data.write_region_label_map(temp_data);
#ifndef GDAL
   spatial_data.close_region_label_map_output();
#endif
#ifdef DEBUG
   if (params.debug > 2)
   {
     params.log_fs << endl << "After writing out results, dump of the region class data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       if (region_classes[region_index].get_active_flag())
         region_classes[region_index].print(region_classes);
     if (params.region_objects_list_flag)
     {
       params.log_fs << endl << "After writing out results, dump of the region object data:" << endl << endl;
       for (region_index = 0; region_index < region_objects_size; ++region_index)
         if (region_objects[region_index].get_active_flag())
           region_objects[region_index].print(region_objects);
     }
     params.log_fs << endl << "After writing out results, dump of region classes map:" << endl << endl;
     spatial_data.print_class_label_map(hlevel,region_classes);
     if (params.object_labels_map_flag)
     {
       params.log_fs << endl << "After writing out results, dump of region objects map:" << endl << endl;
       spatial_data.print_object_label_map(hlevel,region_objects);
     }
   }
   if (params.debug > 3)
   {
     params.log_fs << endl << "After call to write_region_label_map, dump of the pixel data:" << endl << endl;
     for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
     {
       pixel_data[pixel_index].print(pixel_index);
     }
   }
#endif
   hlevel++;

// Perform region growing and spectral clustering with selected hierarchical segmentation output.
   unsigned int max_spclust_min = (unsigned int) (params.spclust_max - 0.05*(params.spclust_max - params.spclust_min));
   int temp_int;

   spclust_min = region_heap_size;
   if (region_heap_size <= params.spclust_max)
   {
     temp_int = spclust_min_factor*(params.spclust_max - region_heap_size);
     temp_int = (int) params.spclust_max - temp_int;
     if (temp_int > (int) params.spclust_min)
       spclust_min = (unsigned int) temp_int;
   }
   if (spclust_min > nregions)
     spclust_min = nregions;
   if (spclust_min > max_spclust_min)
     spclust_min = max_spclust_min;

   spclust_max = region_heap_size;
   if (spclust_max < params.spclust_max)
     spclust_max = params.spclust_max;

   unsigned int prev_nregions = nregions;
   region_classes_size = region_classes.size();

   bool hseg_out_flag;
   for (region_index = 0; region_index < region_classes_size; ++region_index)
   {
     region_classes[region_index].reset_merged_flag();
     region_classes[region_index].set_large_nghbr_merged_flag(false);
   }

   converge_nregions = params.conv_nregions;
   while (nregions > converge_nregions)
   {
     if ((region_heap_size > spclust_max) || ((region_heap_size < spclust_min) && (params.min_npixels > 1)))
     {
      // Readjust min_npixels if indicated
       prev_region_heap_size = region_heap_size;
       prev_min_npixels = params.min_npixels;
      // Find proper value for min_npixels
       params.min_npixels = 0;
       region_heap_size = params.spclust_max + 1;
       while (region_heap_size > params.spclust_max)
       {
         params.min_npixels++;
         region_heap_size = 0;
         for (region_index = 0; region_index < region_classes_size; ++region_index)
         { 
           if ((region_classes[region_index].get_active_flag()) && 
               (region_classes[region_index].get_npix() >= params.min_npixels))
             region_heap_size++;
           if (region_heap_size > params.spclust_max)
             break;
         }
       }

       if ((params.min_npixels > 1) && (region_heap_size < params.spclust_min))
       {
         params.min_npixels--;
         save_region_heap_size = region_heap_size;
         region_heap_size = 0;
         for (region_index = 0; region_index < region_classes_size; ++region_index)
         {
           if ((region_classes[region_index].get_active_flag()) && 
               (region_classes[region_index].get_npix() >= params.min_npixels))
              region_heap_size++;
         }
         if ((region_heap_size > (spclust_max_factor*params.spclust_max)) && (save_region_heap_size > 1))
         {
           region_heap_size = save_region_heap_size;
           params.min_npixels++;
         }
       } // if ((params.min_npixels > 1) && (region_heap_size < params.spclust_min))

       while (region_heap_size < 2)
       {
         params.min_npixels--;
         region_heap_size = 0;
         for (region_index = 0; region_index < region_classes_size; ++region_index)
         {
           if ((region_classes[region_index].get_active_flag()) && 
               (region_classes[region_index].get_npix() >= params.min_npixels))
             region_heap_size++;
         }
       } // while (region_heap_size < 2)
     
       if (params.min_npixels != prev_min_npixels)
       {
         if (params.debug > 1)
         {
           params.log_fs << "Readjusting min_npixels. Currently min_npixels = " << prev_min_npixels;
           params.log_fs << " and region_heap_size = " << prev_region_heap_size << "." << endl;
         }

         if (params.min_npixels < prev_min_npixels)
           max_threshold = 0.0;

      // Reinitialize region_heap
         region_heap.clear();
         region_heap_size = 0;
         for (region_index = 0; region_index < region_classes_size; ++region_index)
         {
           if (region_classes[region_index].get_active_flag())
           {
             region_classes[region_index].set_region_heap_index(UINT_MAX);
             if (region_classes[region_index].get_npix() >= params.min_npixels)
             {
               region_classes[region_index].clear_best_region();
               region_heap.push_back(&region_classes[region_index]);
               region_heap_size++;
             }
           }
         }
         region_heap.push_back(NULL);

         if (params.debug > 1)
         {
           params.log_fs << "At nregions = " << nregions << ", min_npixels readjusted to " << params.min_npixels;
           params.log_fs << " giving region_heap_size = " << region_heap_size << "." << endl;
         }

         for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
           region_heap[heap_index]->best_region_init(heap_index,region_heap,region_heap_size);
         make_region_heap(region_heap, region_heap_size);

         if (params.merge_accel_flag)
         {
          // Update nghbr_heap
           max_min_npixels = params.min_npixels;
           if (max_min_npixels < prev_min_npixels)
             max_min_npixels = prev_min_npixels;
           update_nghbrs_set.clear();
           for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
           {
             if (nghbr_heap[heap_index]->get_npix() < max_min_npixels)
               update_nghbrs_set.insert(nghbr_heap[heap_index]);
           }
           update_nghbrs(update_nghbrs_set, nghbr_heap, nghbr_heap_size, region_classes);
         }

       } // if (params.min_npixels != prev_min_npixels)

       spclust_min = region_heap_size;
       if (region_heap_size <= params.spclust_max)
       {
         temp_int = spclust_min_factor*(params.spclust_max - region_heap_size);
         temp_int = (int) params.spclust_max - temp_int;
         if (temp_int > (int) params.spclust_min)
           spclust_min = (unsigned int) temp_int;
       }
       if (spclust_min > nregions)
         spclust_min = nregions;
       if (spclust_min > max_spclust_min)
         spclust_min = max_spclust_min;
       spclust_max = region_heap_size;
       if (spclust_max < params.spclust_max)
         spclust_max = params.spclust_max;
       if (params.debug > 2)
         params.log_fs << "spclust_min readjusted to " << spclust_min << ", and spclust_max readjusted to " << spclust_max << endl; 

     } // if ((region_heap_size > params.spclust_max) || ((region_heap_size < spclust_min) && (params.min_npixels > 1)))

     if (params.chk_nregions_flag)
     {
       hseg_out_flag = merge_regions(true, converge_nregions,
                                     nghbr_heap, nghbr_heap_size, region_heap, region_heap_size, 
                                     region_classes, nregions, max_threshold);
     }
     else
     {
       merge_regions(false, converge_nregions,
                     nghbr_heap, nghbr_heap_size, region_heap, region_heap_size, 
                     region_classes, nregions, max_threshold);
       hseg_out_flag = false;
       if (params.hseg_out_nregions_flag)
       {
         if (hlevel < params.nb_hseg_out_nregions)
           hseg_out_flag = (nregions <= params.hseg_out_nregions[hlevel]);
         else
           hseg_out_flag = (nregions <= converge_nregions);
       }
       else if (params.hseg_out_thresholds_flag)
       {
         if (hlevel < params.nb_hseg_out_thresholds)
           hseg_out_flag = (max_threshold >= params.hseg_out_thresholds[hlevel]);
         else
           hseg_out_flag = (nregions <= converge_nregions);
       }
     }

#ifdef DEBUG
     if (params.debug > 2)
     {
       params.log_fs << endl << "After call to merge_regions, max_threshold = " << max_threshold << endl;
     }
     if (params.debug > 3)
     {
       params.log_fs << endl << "After call to merge_regions, dump of the region data:" << endl << endl;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         if (region_classes[region_index].get_active_flag())
           region_classes[region_index].print(region_classes);
     }
     if (params.debug > 3)
     {
       params.log_fs << endl << "After call to merge_regions, dump of the pixel data:" << endl << endl;
       for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
       {
         pixel_data[pixel_index].print(pixel_index);
       }
     }
#endif

     if ((hseg_out_flag) || (nregions <= converge_nregions))
     {
// Update pixel_data.region_label
       unsigned int merge_region_index, next_merge_region_label;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         if (!(region_classes[region_index].get_active_flag()))
         {
           merge_region_index = region_index;
           next_merge_region_label = region_classes[merge_region_index].get_merge_region_label();
           while (next_merge_region_label != 0)
           {
             merge_region_index = next_merge_region_label - 1;
             next_merge_region_label = region_classes[merge_region_index].get_merge_region_label();
           }
           region_classes[region_index].set_merge_region_label(region_classes[merge_region_index].get_label());
         }
       region_relabel_pairs.clear();
       for (region_index = 0; region_index < region_classes_size; ++region_index)
       {
         region_label = region_classes[region_index].get_label();
         if ((!region_classes[region_index].get_active_flag()) && (region_classes[region_index].get_merged_flag()))
         {
           region_relabel_pairs.insert(make_pair(region_label,
                                       region_classes[region_classes[region_index].get_merge_region_label() - 1].get_label()));
         }
       } 
       if (!(region_relabel_pairs.empty()))
         do_region_class_relabel(params.recur_level,section,region_relabel_pairs,pixel_data,temp_data);

#ifdef DEBUG
       if (params.debug > 3)
       {
         params.log_fs << endl << "After call to do_region_class_relabel, dump of the pixel data:" << endl << endl;
         for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
         {
           pixel_data[pixel_index].print(pixel_index);
         }
       }
       if (params.debug > 3)
       {
         params.log_fs << endl << "List of pixel data labels:" << endl;
         for (pixel_index = 0; pixel_index < pixel_data_size; pixel_index++)
         {
           params.log_fs << "Element " << pixel_index;
           params.log_fs << " has region label ";
           pixel_data[pixel_index].print_region_label();
         }
       }
#endif

       if (params.gdissim_flag)
       {
         if ((params.dissim_crit == 1) || (params.dissim_crit == 2) || (params.dissim_crit == 3) ||
             (params.dissim_crit == 4) || (params.dissim_crit == 5) || (params.dissim_crit == 8) ||
             (params.dissim_crit == 10))
         {
           for (region_index = 0; region_index < region_classes_size; ++region_index)
             region_classes[region_index].set_sum_pixel_gdissim(0.0);
           update_sum_pixel_gdissim(params.recur_level,section,pixel_data,region_classes,temp_data);
         }

         global_dissim = 0.0;
         numpix = 0.0;
         for (region_index = 0; region_index < region_classes_size; ++region_index)
         {
           if (region_classes[region_index].get_active_flag())
           {
             if ((params.dissim_crit == 6) || (params.dissim_crit == 7) || (params.dissim_crit == 9))
               region_classes[region_index].calc_sum_pixel_gdissim();
             region_numpix = region_classes[region_index].get_npix();
             numpix += region_numpix;
             global_dissim += region_classes[region_index].get_sum_pixel_gdissim();
           }
         }  
         if ((params.dissim_crit == 6) || (params.dissim_crit == 7))
         {
           global_dissim /= (numpix-1.0);
#ifdef MSE_SQRT
           global_dissim = sqrt(global_dissim); // Added to make dimensionality consistent
#endif
         }
         else
           global_dissim /= numpix;
       }

       spatial_data.update_region_label_map(params.recur_level,section,pixel_data,temp_data);
       if (params.region_nb_objects_flag)
       {
#ifdef THREEDIM
         connected_component(params.recur_level, section, params.padded_ncols, params.padded_nrows, params.padded_nslices, hlevel,
                             region_objects, spatial_data, temp_data);
#else
         connected_component(params.recur_level, section, params.padded_ncols, params.padded_nrows, hlevel,
                             region_objects, spatial_data, temp_data);
#endif
         for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
           if (region_objects[region_object_index].get_active_flag())
             region_objects[region_object_index].clear();
#ifdef THREEDIM
         nb_objects = do_region_objects_init(params.recur_level,section, 
                                             params.padded_ncols,params.padded_nrows,params.padded_nslices,
                                             pixel_data,spatial_data,region_objects,temp_data);
#else
         nb_objects = do_region_objects_init(params.recur_level,section,
                                             params.padded_ncols, params.padded_nrows,
                                             pixel_data,spatial_data,region_objects,temp_data);
#endif
         update_region_object_info(params.recur_level,section,hlevel,spatial_data,region_objects,temp_data);
         for (region_index = 0; region_index < region_classes_size; ++region_index)
           region_classes[region_index].clear_region_info();
         update_region_class_info(params.recur_level,section,hlevel,spatial_data,region_classes,temp_data);
#ifdef DEBUG
         if (params.debug > 3)
         {
           params.log_fs << "After updating region info, dump of region_classes:" << endl;
           for (region_index = 0; region_index < region_classes_size; ++region_index)
             region_classes[region_index].print(region_classes);
         }
#endif
       }
       else if ((params.region_boundary_npix_flag) || (params.boundary_map_flag))
       {
         for (region_index = 0; region_index < region_classes_size; ++region_index)
           region_classes[region_index].clear_region_info();
#ifdef THREEDIM
         boundary_map(params.recur_level,section,params.padded_ncols,params.padded_nrows,params.padded_nslices,
                      hlevel,spatial_data,temp_data);
#else
         boundary_map(params.recur_level,section,params.padded_ncols,params.padded_nrows,
                      hlevel,spatial_data,temp_data);
#endif
         update_region_class_info(params.recur_level,section,hlevel,spatial_data,region_classes,temp_data);
#ifdef DEBUG
         if (params.debug > 3)
         {
           params.log_fs << "After updating region info, dump of region_classes:" << endl;
           for (region_index = 0; region_index < region_classes_size; ++region_index)
             region_classes[region_index].print(region_classes);
         }
#endif
       }

       if (params.debug > 0)
       {
         params.log_fs << endl << "Data written at hlevel = " << hlevel << " for " << nregions << " region classes,";
         params.log_fs << " with maximum merging threshold = " << max_threshold;
         if (params.gdissim_flag)
           params.log_fs << endl << "and with global dissimilarity value = " << global_dissim << ".  ";
         else
           params.log_fs << "." << endl;
         if (params.region_nb_objects_flag)
           params.log_fs << "There are " << nb_objects << " region objects.  ";
         if ((params.min_npixels > 1) && (region_heap_size > 0))
           params.log_fs << " (large_nregions = " << region_heap_size << ")" << endl;
         else
           params.log_fs << endl;
         params.log_fs << endl;
       }
       else
       {
         cout << endl << "Data written at hlevel = " << hlevel << " for " << nregions << " region classes,";
         cout << " with maximum merging threshold = " << max_threshold;
         if (params.gdissim_flag)
           cout << endl << "and with global dissimilarity value = " << global_dissim << ".  ";
         else
           cout << "." << endl;
         if (params.region_nb_objects_flag)
           cout << "There are " << nb_objects << " region objects.  ";
         if ((params.min_npixels > 1) && (region_heap_size > 0))
           cout << " (large_nregions = " << region_heap_size << ")" << endl;
         else
           cout << endl;
         cout << endl;
       }
     // Write data.
       results_data.write(hlevel,nregions,nb_objects,region_classes,region_objects);
       oparams.int_buffer_size.push_back(results_data.get_int_buffer_index());
       oparams.max_threshold.push_back(max_threshold);
       if (params.gdissim_flag)
         oparams.gdissim.push_back(global_dissim);
#ifdef DEBUG
       if (params.debug > 3)
       {
         params.log_fs << endl << "After writing out results, dump of the region class data:" << endl << endl;
         for (region_index = 0; region_index < region_classes_size; ++region_index)
           if (region_classes[region_index].get_active_flag())
             region_classes[region_index].print(region_classes);
         if (params.region_objects_list_flag)
         {
           params.log_fs << endl << "After writing out results, dump of the region object data:" << endl << endl;
           for (region_index = 0; region_index < region_objects_size; ++region_index)
             if (region_objects[region_index].get_active_flag())
               region_objects[region_index].print(region_objects);
         }
         params.log_fs << endl << "After writing out results, dump of region classes map:" << endl << endl;
         spatial_data.print_class_label_map(hlevel,region_classes);
         if (params.object_labels_map_flag)
         {
           params.log_fs << endl << "After writing out results, dump of region objects map:" << endl << endl;
           spatial_data.print_object_label_map(hlevel,region_objects);
         }
       }
#endif
       hlevel++;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
       {
         region_classes[region_index].reset_merged_flag();
         region_classes[region_index].set_large_nghbr_merged_flag(false);
       }
     } // if (hseg_out_flag)
     else if (prev_nregions == nregions)
       converge_nregions = nregions;
     prev_nregions = nregions;
   } // while (nregions > converge_nregions)

   region_classes_size = region_classes.size();

   results_data.close_output();
   if (params.boundary_map_flag)
   {
#ifndef GDAL
     spatial_data.open_boundary_map_output();
#endif
     spatial_data.write_boundary_map(temp_data);
#ifndef GDAL
     spatial_data.close_boundary_map();
#endif
   }

   oparams.level0_nb_classes = region_classes_size;
   if (params.region_nb_objects_flag)
     oparams.level0_nb_objects =  region_objects_size;
   oparams.nb_levels = hlevel;

#ifdef PARALLEL
   do_termination(params.recur_level,temp_data);
#ifdef TIME_IT
   if (params.myid == 0)
   {
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
     if (params.debug > 0)
     {
       params.log_fs << "setup time: " << temp_data.setup << endl;
       params.log_fs << "compute time: " << temp_data.compute << endl;
       params.log_fs << "transfer time: " << temp_data.transfer << endl;
       params.log_fs << "wait time: " << temp_data.wait << endl;
     }
     else
     {
       cout << "setup time: " << temp_data.setup << endl;
       cout << "compute time: " << temp_data.compute << endl;
       cout << "transfer time: " << temp_data.transfer << endl;
       cout << "wait time: " << temp_data.wait << endl;
     }
   }
#endif
#else
   if (params.nb_sections > 1)
   {
     for (section = 0; section < params.nb_sections; section++)
     {
       remove_temp_file("input_image",section);
       remove_temp_file("mask",section);
       remove_temp_file("pixel",section);
       remove_temp_file("class_labels_map",section);
       remove_temp_file("object_labels_map",section);
       remove_temp_file("boundary_map",section);
       remove_temp_file("int",section);
       remove_temp_file("short",section);
       remove_temp_file("byte",section);
     }
   }
#endif

// Write output parameters
#ifdef PARALLEL
   if (params.myid == 0)
   {
#endif
     params.write_oparam(oparams);
#ifdef PARALLEL
   }
#endif

#ifdef PARALLEL
   if ((params.myid == 0) && (params.debug > 0))
     params.log_fs << "Processing is 100% complete" << endl;
#else
   if (!params.gtkmm_flag)
     cout << "Processing is 100% complete" << endl;
#endif

   return true;
 }
} // namespace HSEGTilton
