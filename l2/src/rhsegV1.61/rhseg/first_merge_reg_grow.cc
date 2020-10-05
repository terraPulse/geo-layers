/*-----------------------------------------------------------
|
|  Routine Name: first_merge_reg_grow
|
|       Purpose: Performs randomized region oriented first merge region growing
|
|         Input: pixel_data       (Class which holds information pertaining to the data pixels)
|                ncols            (Number of columns in pixel_data)
|                nrows            (Number of rows in pixel_data)
|                
|        Output:
|
|         Other: region_classes   (Class which holds region related information)
|                nregions         (Current number of regions)
|
|       Returns: void
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: February 28, 2007.
| Modifications: March 24, 2007 - Modified to account for changes in pixel_data object.
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                January 20, 2010 - Revised initialization and eliminated params.max_nregions
|                September 30, 2010 - Corrected a bug for the case when initialized with region_map_in
|                May 22, 2013 - Added optional initialization from an edge image
|
------------------------------------------------------------*/

#include "hseg.h"
#include <params/params.h>
#include <pixel/pixel.h>
#include <region/region_class.h>
#include <algorithm>
#include <time.h>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef THREEDIM
 void first_merge_reg_grow(const int &ncols, const int& nrows, const int& nslices,
                           vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, unsigned int& nregions)
#else
 void first_merge_reg_grow(const int &ncols, const int& nrows,
                           vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, unsigned int& nregions)
#endif
 {
   if ((params.edge_image_flag) && (params.edge_threshold > 0.0))
     params.init_threshold = params.edge_threshold;

   if (params.debug > 1)
   {
     if (params.init_threshold == 0.0)
       params.log_fs << "Region initialization started with nregions = " << nregions;
     else
       params.log_fs << "First merge region growing started with nregions = " << nregions;
     params.log_fs << endl;
   }

   unsigned int region_index, region_label, region_classes_size = region_classes.size();
   unsigned int region_classes_capacity = region_classes.capacity();
   if (region_classes_capacity <= nregions)
   {
     region_classes_capacity = nregions + 1;
     if (region_classes_capacity > 10000)
       region_classes_capacity += 10000;
     else if (region_classes_capacity > 1000)
        region_classes_capacity += 1000;
     else if (region_classes_capacity < 10)
       region_classes_capacity += 10;
     else
       region_classes_capacity *= 2;
     region_classes.reserve(region_classes_capacity);
   }
   if (region_classes_size < nregions)
   {
     region_classes_size = nregions;
     region_classes.resize(region_classes_size);
   }
   for (region_index = 0; region_index < nregions; region_index++)
   {
     region_label = region_index + 1;
     region_classes[region_index].clear();
     region_classes[region_index].set_label(region_label);
   }

   unsigned int pixel_index, pixel_data_size = pixel_data.size();
   unsigned int tot_npixels = 0;
   vector <unsigned int> pixel_index_vector;
   for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
   {
     if (pixel_data[pixel_index].get_mask())
     {
       pixel_index_vector.push_back(pixel_index);
       tot_npixels++;
     }
   }
   if (params.init_threshold > 0.0)
   {
     if (params.random_init_seed_flag)
       srand(time(NULL)); // Use time seed to a randomized set of "random" numbers
     else
       srand(1234);  // Fixed seed for a consistent set of "random" numbers
     random_shuffle(pixel_index_vector.begin(),pixel_index_vector.end());
   }

   bool init_flag;
   int col, row;
#ifdef THREEDIM
   int slice;
#endif
   vector<unsigned int>::const_iterator pixel_index_iter;
   pixel_index_iter = pixel_index_vector.begin();
   while (pixel_index_iter != pixel_index_vector.end())
   {
     pixel_index = *pixel_index_iter;
#ifdef THREEDIM
     slice = pixel_index/(nrows*ncols);
     row = ((int) pixel_index - slice*((int) nrows*ncols))/ncols;
     col = ((int) pixel_index) - row*((int) ncols) - slice*((int) (nrows*ncols));
#else
     row = pixel_index/ncols;
     col = ((int) pixel_index) - row*((int) ncols);
#endif
     init_flag = pixel_data[pixel_index].get_init_flag();
     if (!init_flag)
     {
       pixel_data[pixel_index].set_init_flag(true);
       region_label = pixel_data[pixel_index].get_region_label();
       if (region_label == 0)
       {
         nregions++;
         region_label = nregions;
         pixel_data[pixel_index].set_region_label(region_label);
         if (region_classes_capacity <= nregions)
         {
           region_classes_capacity = nregions + 1;
           if (region_classes_capacity > 10000)
             region_classes_capacity += 10000;
           else if (region_classes_capacity > 1000)
             region_classes_capacity += 1000;
           else if (region_classes_capacity < 10)
             region_classes_capacity += 10;
           else
             region_classes_capacity *= 2;
           region_classes.reserve(region_classes_capacity);
         }
         if (region_classes_size < region_label)
         {
           region_classes_size = region_label;
           region_classes.resize(region_classes_size);
         }
         region_index = region_label - 1;
         region_classes[region_index].clear();
         region_classes[region_index].set_label(region_label);
#ifdef THREEDIM
         region_classes[region_index].fm_init(pixel_data, region_classes, col, row, slice, ncols, nrows, nslices);
#else
         region_classes[region_index].fm_init(pixel_data, region_classes, col, row, ncols, nrows);
#endif
         while (region_classes[region_index].find_merge(pixel_data))
         {
#ifdef THREEDIM
           region_classes[region_index].do_merge(ncols,nrows,nslices,nregions,pixel_data,region_classes,region_classes_size);
#else
           region_classes[region_index].do_merge(ncols,nrows,nregions,pixel_data,region_classes,region_classes_size);
#endif
         }
       }
       else
       {
         if (region_classes_capacity < region_label)
         {
           region_classes_capacity = region_label;
           if (region_classes_capacity > 10000)
             region_classes_capacity += 10000;
           else if (region_classes_capacity > 1000)
             region_classes_capacity += 1000;
           else if (region_classes_capacity < 10)
             region_classes_capacity += 10;
           else
             region_classes_capacity *= 2;
           region_classes.reserve(region_classes_capacity);
         }
         if (region_classes_size < region_label)
         {
           region_classes_size = region_label;
           region_classes.resize(region_classes_size);
         }
         region_index = region_label - 1;
#ifdef THREEDIM
         region_classes[region_index].fm_init(pixel_data, region_classes, col, row, slice, ncols, nrows, nslices);
#else
         region_classes[region_index].fm_init(pixel_data, region_classes, col, row, ncols, nrows);
#endif
       }
     } // if (!init_flag)
     pixel_index_iter++;
   }
   pixel_index_iter = pixel_index_vector.begin();
   while (pixel_index_iter != pixel_index_vector.end())
   {
     pixel_index = *pixel_index_iter;
     pixel_data[pixel_index].set_init_flag(false);
     pixel_index_iter++;
   }

// Compact the region_classes vector, sort by size, and erase unneeded elements
   unsigned int compact_region_index = 0;
   vector<RegionClass> compact_region_classes(nregions,RegionClass());
   for (region_index = 0; region_index < region_classes_size; ++region_index)
   {
     if (region_classes[region_index].get_active_flag())
     {
       compact_region_classes[compact_region_index++] = region_classes[region_index];
     }
     if (compact_region_index == nregions)
       break;
   }
   region_classes_size = nregions;
   if (region_classes.size() != region_classes_size)
     region_classes.resize(region_classes_size);

   if (params.sort_flag)
   {
  // Sort region_classes by number of pixels.
     sort(compact_region_classes.begin(), compact_region_classes.end(), NpixMoreThan());
   }

 // Renumber region_classes, set up region_class_relabel_pairs.
   map<unsigned int,unsigned int> region_class_relabel_pairs;
   for (region_index = 0; region_index < nregions; ++region_index)
   {
     region_classes[region_index] = compact_region_classes[region_index];
     region_label = region_classes[region_index].get_label();
     region_classes[region_index].set_label(region_index+1);
     region_classes[region_index].clear_best_merge();
     if (region_label != region_classes[region_index].get_label())
       region_class_relabel_pairs.insert(make_pair(region_label,region_classes[region_index].get_label()));
   }

   if (!(region_class_relabel_pairs.empty()))
   {
   // Use region_class_relabel_pairs to renumber region_label in pixel_data.
     do_region_class_relabel(region_class_relabel_pairs,pixel_data);
   }

 // Initialize region_classes.nghbrs_label_set from pixel_data
 // Also initialize intial_merge_flag (if used)
   for (region_index = 0; region_index < nregions; ++region_index)
   {
     region_classes[region_index].nghbrs_label_set_clear();
     if ((params.program_mode != 1) && (params.initial_merge_flag))
     {
       if (region_classes[region_index].get_npix() > params.initial_merge_npix)
         region_classes[region_index].set_initial_merge_flag(true);
       else
         region_classes[region_index].set_initial_merge_flag(false);
     }
   }
#ifdef THREEDIM
   nghbrs_label_set_init(ncols,nrows,nslices,pixel_data,region_classes);
#else
   nghbrs_label_set_init(ncols,nrows,pixel_data,region_classes);
#endif

#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After initialization in first_merge_reg_grow, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < nregions; ++region_index)
       region_classes[region_index].print(region_classes);
     params.log_fs << endl << "Dump of the pixel data region labels:" << endl << endl;
     for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
       if (pixel_data[pixel_index].get_region_label() != 0)
       {
         params.log_fs << "Element " << pixel_index << " is associated with region label ";
         pixel_data[pixel_index].print_region_label();
         params.log_fs << endl;
       }
   }
#endif

   if (params.debug > 1)
   {
     if (params.init_threshold > 0.0)
     {
       params.log_fs << "Region initialization completed utilizing first merge region growing";
       params.log_fs << ", with the number of regions = " << nregions << endl;
     }
     else
       params.log_fs << "Region initialization completed, with the number of regions = " << nregions << endl;
   }

   return;
 }
} // namespace HSEGTilton
