/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>          File:  region_class.cc
   >>>>
   >>>>          See region_class.h for documentation
   >>>>
   >>>>          Date:  December 10, 2002
   >>>>
   >>>> Modifications:  February 5, 2003:  Corrected precision problem in calc_std_dev
   >>>>                 February 10, 2003: Changed region index to region label
   >>>>                 February 14, 2003: Corrected = and + operator overload functions
   >>>>                 June 4, 2003: Changed list<short unsigned int>nghbrs_label_list to
   >>>>                               set<short unsigned int>nghbrs_label_set for efficiency
   >>>>                 June 9, 2003:  Changed other lists to sets for efficiency
   >>>>                 June 16, 2003 - Made std_dev_flag and nbands static
   >>>>                 July 8, 2003 - Added dissim_crit = 10:  1-Norm with cyclotron frequency masking
   >>>>                 August 21, 2003 - Reorganized program and added user controlled byteswapping
   >>>>                 September 29, 2003 - Added subregion_set and boundary_npix member variables
   >>>>                 October 1, 2003 - Added renumber function to renumber nghbrs_label_set
   >>>>                 November 7, 2003 - Added load_data function.
   >>>>                 December 23, 2004 - Changed region label from short unsigned int to unsigned int
   >>>>                 January 14, 2005 - Replaced dissim_crit = 10 with new SAR Speckle Noise Criterion
   >>>>                 August 9, 2005 - Added static sumsq_flag
   >>>>                 August 9, 2005 - Changed max_merge_threshold member variable to conv_crit_value
   >>>>                 August 15, 2005 - Added the conv_ratio member variable
   >>>>                 August 18, 2005 - Added the merged_flag and one_pixel_flag member variables.
   >>>>                 October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
   >>>>                 November 22, 2005 - Added the col_init, row_init and slice_init functions (for three-d)
   >>>>                 December 23, 2005 - Added the min_region_dissim member variable.
   >>>>                 February 27, 2006 - Added nghbr_heap_npix and region_heap_npix member variables.
   >>>>                 August 28, 2006 - Added additional Dissimilarity Criterion (Nos. 4, 5 and 8)
   >>>>                 February 9, 2007 - Modified method for including region standard deviation in dissimilarity criteria
   >>>>                 February 28, 2007 - Added fm_init function for region initialization embedded in first merge region growing
   >>>>                 February 28, 2007 - Also added find_merge, and do_merge functions.
   >>>>                 March 25, 2007 - Modified set_static_vals to also initialize scale and offset static members
   >>>>                 June 27, 2007 - Modified factor for small region merge acceleration
   >>>>                 November 1, 2007 - Changed the conv_crit_value member variable to merge_threshold
   >>>>                 November 1, 2007 - Eliminated conv_ratio, one_pixel_flag and conv_criterion member variables
   >>>>                 March 31, 2008 - Replaced std_dev feature with mean normalized std_dev feature.
   >>>>                 May 12, 2008 - Modified to work with new Params object class
   >>>>                 July 30, 2009 - Added inclusion of C++ algorithm library <algorithm>
   >>>>                 October 6, 2010 - Modified code to mask sure pixel_data masking is accounted for properly.
   >>>>                 October 7, 2010 - Set result of dissimilarity calculations to 0.0 if computed result is less than SMALL_EPSILON.
   >>>>                 January 29, 2011 - Added the large_nghbr_merged_flag.
   >>>>			March 1, 2013 - Replaced the std_dev member variable with sum_pixel_std_dev as part of the expansion
   >>>>				        of the use of the standard deviation spatial feature.
   >>>>                 April 16, 2013 - Added the related RegionEdge class and replaced nghbrs_label_set with nghbr_info_map
   >>>>                 May 22, 2013 - Added optional initialization (in fm_init, find_best_merge, merge_best2_nghbrs) from an edge image
   >>>>                 August 9, 2013 - Added the best_edge_nghbr_init function.
   >>>>                 August 9, 2013 - Added the capability perform processing window artifact elimination based on edge information.
   >>>>                 September 6, 2013 - Replaced the best_edge_nghbr_init function with the seam_nghbr_init function
   >>>>                 November 7, 2103 - Modified definition of edge_dissim_option == 2.
   >>>>                 November 19, 2013 - Added the compare_region_classes function.
   >>>>                 January 8, 2014 - Removed the RegionEdge and Location classes and replaced nghbr_info_map with nghbrs_label_set
   >>>>                 February 3, 2014 - Added seam_nghbr_npix_map, and sum_seam_edge_value for used in artifact elimination
   >>>>                 February 3, 2014 - Removed seam_nghbr_init function
   >>>>                 February 28, 2014 - Added the initial_merge_flag member variable and associated logic.
   >>>>                 March 5, 2014 - Added seam_flag member variable and associated logic.
   >>>>                 March 27, 2014 - Added the RegionSeam class.
   >>>>                 March 27, 2014 - Changed seam_nghbr_label_npix_map to nghbrs_label_seam_map and dropped sum_seam_edge_value.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "region_class.h"
#include "region_seam.h"
#include "region_object.h"
#include <params/params.h>
#include <spatial/spatial.h>
#include <pixel/pixel.h>
#include <index/index.h>
#include <iostream>
#include <cmath>
#include <algorithm>

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{

 bool RegionClass::sumsq_flag;
 bool RegionClass::sumxlogx_flag;
 bool RegionClass::std_dev_flag;
 int  RegionClass::nbands;
 double *RegionClass::scale;
 double *RegionClass::offset;
#ifdef RHSEG_RUN
 double *RegionClass::minval;
 double *RegionClass::meanval;
#endif

 RegionClass::RegionClass()
 {
  int band;

// Initialize member variables
  active_flag = false;
#ifdef RHSEG_RUN
  initial_merge_flag = false;
  seam_flag = false;
  merged_flag = true;
  large_nghbr_merged_flag = false;
#endif
  label = 0;
  npix = 0;
  sum = new double[nbands];
  for (band = 0; band < nbands; band++)
    sum[band] = 0.0;
  sumsq = NULL;
  if (sumsq_flag)
  {
    sumsq = new double[nbands];
    for (band = 0; band < nbands; band++)
      sumsq[band] = 0.0;
  }
  if (sumxlogx_flag)
  {
    sumxlogx = new double[nbands];
    for (band = 0; band < nbands; band++)
      sumxlogx[band] = 0.0;
  }
  if (std_dev_flag)
  {
    sum_pixel_std_dev = new double[nbands];
    for (band = 0; band < nbands; band++)
      sum_pixel_std_dev[band] = 0.0;
  }
  max_edge_value = -FLT_MAX;
  band_max_std_dev = -1.0;  // Value less than zero signifies this variable is uninitialized.
  nghbrs_label_set.clear();
#ifdef RHSEG_RUN
  best_nghbr_label.clear();
  best_nghbr_dissim = FLT_MAX;
  nghbr_heap_index = UINT_MAX;
  best_region_label.clear();
  best_region_dissim = FLT_MAX;
  region_heap_index = UINT_MAX;
  nghbrs_label_seam_map.clear();
#endif
  merge_region_label = 0;
  sum_pixel_gdissim = 0.0;
  min_region_dissim = FLT_MAX;
  nb_region_objects = 0;
  region_objects_set.clear();
  boundary_npix = 0;
  merge_threshold = 0.0;

  return;
 }

 RegionClass::RegionClass(const RegionClass& source)
 {
  int band;

// Copy member variables
  active_flag = source.active_flag;
#ifdef RHSEG_RUN
  initial_merge_flag = source.initial_merge_flag;
  seam_flag = source.seam_flag;
  merged_flag = source.merged_flag;
  large_nghbr_merged_flag = source.large_nghbr_merged_flag;
#endif
  label = source.label;
  npix = source.npix;
#ifdef RHSEG_RUN
  nghbr_heap_npix = source.npix; // This makes sure nghbr_heap_npix is initialized properly
  region_heap_npix = source.npix; // This makes sure region_heap_npix is initialized properly
#endif
  sum = new double[nbands];
  for (band = 0; band < nbands; band++)
    sum[band] = source.sum[band];
  if (sumsq_flag)
  {
    sumsq = new double[nbands];
    for (band = 0; band < nbands; band++)
      sumsq[band] = source.sumsq[band];
  }
  if (sumxlogx_flag)
  {
    sumxlogx = new double[nbands];
    for (band = 0; band < nbands; band++)
      sumxlogx[band] = source.sumxlogx[band];
  }
  if (std_dev_flag)
  {
    sum_pixel_std_dev = new double[nbands];
    for (band = 0; band < nbands; band++)
      sum_pixel_std_dev[band] = source.sum_pixel_std_dev[band];
    band_max_std_dev = source.band_max_std_dev;
  }
  max_edge_value = source.max_edge_value;

#ifdef RHSEG_RUN
  nghbr_heap_index = source.nghbr_heap_index;
  region_heap_index = source.region_heap_index;
#endif
  merge_region_label = source.merge_region_label;
  sum_pixel_gdissim = source.sum_pixel_gdissim;
  min_region_dissim = source.min_region_dissim;
  nb_region_objects = source.nb_region_objects;
  boundary_npix = source.boundary_npix;
  merge_threshold = source.merge_threshold;

  nghbrs_label_set_copy(source);
  region_objects_set_copy(source);
#ifdef RHSEG_RUN
  best_nghbr_copy(source);
  best_region_copy(source);
  nghbrs_label_seam_map_copy(source);
#endif // RHSEG_RUN

  return;
 }

 RegionClass::~RegionClass()
 {
  delete [ ] sum;
  if (sumsq_flag)
    delete [ ] sumsq;
  if (sumxlogx_flag)
    delete [ ] sumxlogx;
  if (std_dev_flag)
    delete [ ] sum_pixel_std_dev;
  nghbrs_label_set.clear();
  region_objects_set.clear();
#ifdef RHSEG_RUN
  best_nghbr_label.clear();
  best_region_label.clear();
  nghbrs_label_seam_map.clear();
#endif

  return;
 }

 void RegionClass::set_static_vals()
 {
  int band;

  sumsq_flag = params.region_sumsq_flag;
  sumxlogx_flag = params.region_sumxlogx_flag;
  std_dev_flag = params.std_dev_image_flag;
  nbands = params.nbands;
  scale = new double[nbands];
  offset = new double[nbands];
#ifdef RHSEG_RUN
  minval = new double[nbands];
  meanval = new double[nbands];
#endif
 // Coarsen to float to maintain consistency of results between different computing platforms and operating systems.
  for (band = 0; band < nbands; band++)
  {
    scale[band] = (float) (oparams.scale[band]*params.scale[band]);
    offset[band] = (float) (params.offset[band] + (oparams.offset[band]/params.scale[band]));
#ifdef RHSEG_RUN
    minval[band] = (float) oparams.minval[band];
    meanval[band] = (float) oparams.meanval[band];
#endif
  }
  return;
 }

 void RegionClass::operator =(const RegionClass& source)
 {
  int band;

  if (this == &source)
    return;

// Copy member variables
  active_flag = source.active_flag;
#ifdef RHSEG_RUN
  initial_merge_flag = source.initial_merge_flag;
  seam_flag = source.seam_flag;
  merged_flag = source.merged_flag;
  large_nghbr_merged_flag = source.large_nghbr_merged_flag;
#endif
  label = source.label;
  npix = source.npix;
#ifdef RHSEG_RUN
  nghbr_heap_npix = source.npix; // This makes sure nghbr_heap_npix is initialized properly
  region_heap_npix = source.npix; // This makes sure region_heap_npix is initialized properly
#endif
  for (band = 0; band < nbands; band++)
    sum[band] = source.sum[band];
  if (sumsq_flag)
  {
    for (band = 0; band < nbands; band++)
      sumsq[band] = source.sumsq[band];
  }
  if (sumxlogx_flag)
  {
    for (band = 0; band < nbands; band++)
      sumxlogx[band] = source.sumxlogx[band];
  }
  if (std_dev_flag)
  {
    for (band = 0; band < nbands; band++)
      sum_pixel_std_dev[band] = source.sum_pixel_std_dev[band];
    band_max_std_dev = source.band_max_std_dev;
  }
  max_edge_value = source.max_edge_value;

#ifdef RHSEG_RUN
  nghbr_heap_index = source.nghbr_heap_index;
  region_heap_index = source.region_heap_index;
#endif
  merge_region_label = source.merge_region_label;
  sum_pixel_gdissim = source.sum_pixel_gdissim;
  min_region_dissim = source.min_region_dissim;
  nb_region_objects = source.nb_region_objects;
  boundary_npix = source.boundary_npix;
  merge_threshold = source.merge_threshold;

  nghbrs_label_set_copy(source);
  region_objects_set_copy(source);
#ifdef RHSEG_RUN
  best_nghbr_copy(source);
  best_region_copy(source);
  nghbrs_label_seam_map_copy(source);
#endif // RHSEG_RUN

  return;
 }

 void RegionClass::operator +=(const RegionClass& source)
 {
  int band;

// Update private data - label remains unchanged
  npix += source.npix;
#ifdef RHSEG_RUN
  nghbr_heap_npix = npix;
  region_heap_npix = npix;
#endif
  for (band = 0; band < nbands; band++)
    sum[band] += source.sum[band];
  if (sumsq_flag)
  {
    for (band = 0; band < nbands; band++)
      sumsq[band] += source.sumsq[band];
  }
  if (sumxlogx_flag)
  {
    for (band = 0; band < nbands; band++)
      sumxlogx[band] += source.sumxlogx[band];
  }
  if (std_dev_flag)
  {
    for (band = 0; band < nbands; band++)
      sum_pixel_std_dev[band] += source.sum_pixel_std_dev[band];
  }

// These values returned to original default values - must be updated by other means
  max_edge_value = -FLT_MAX;
  band_max_std_dev = -1.0;
#ifdef RHSEG_RUN
  best_nghbr_dissim = FLT_MAX;
  nghbr_heap_index = UINT_MAX;
  best_region_dissim = FLT_MAX;
  region_heap_index = UINT_MAX;
#endif
  merge_region_label = 0;
  sum_pixel_gdissim = 0.0;
  min_region_dissim = FLT_MAX;
  boundary_npix = 0;
  merge_threshold = 0.0;
  nb_region_objects = 0;
  nghbrs_label_set.clear();
  region_objects_set.clear();
#ifdef RHSEG_RUN
  best_nghbr_label.clear();
  best_region_label.clear();
  nghbrs_label_seam_map.clear();
#endif

// Combine nghbrs_label_set
  if (!source.nghbrs_label_set.empty())
  {
    set<unsigned int>::const_iterator nghbrs_label_set_iter = source.nghbrs_label_set.begin();
    while (nghbrs_label_set_iter != source.nghbrs_label_set.end())
    {
      nghbrs_label_set.insert(*nghbrs_label_set_iter);
      ++nghbrs_label_set_iter;
    }
    nghbrs_label_set.erase(label);
    nghbrs_label_set.erase(source.label);
  }

  return;
 }

 void RegionClass::operator -=(const RegionClass& source)
 {
  int band;

// Update private data - label remains unchanged
  npix -= source.npix;
#ifdef RHSEG_RUN
  nghbr_heap_npix = npix;
  region_heap_npix = npix;
#endif
  for (band = 0; band < nbands; band++)
    sum[band] -= source.sum[band];
  if (sumsq_flag)
  {
    for (band = 0; band < nbands; band++)
      sumsq[band] -= source.sumsq[band];
  }
  if (sumxlogx_flag)
  {
    for (band = 0; band < nbands; band++)
      sumxlogx[band] -= source.sumxlogx[band];
  }
  if (std_dev_flag)
  {
    for (band = 0; band < nbands; band++)
      sum_pixel_std_dev[band] -= source.sum_pixel_std_dev[band];
  }

// These values returned to original default values - must be updated by other means
  max_edge_value = -FLT_MAX;
  band_max_std_dev = -1.0;
#ifdef RHSEG_RUN
  best_nghbr_dissim = FLT_MAX;
  nghbr_heap_index = UINT_MAX;
  best_region_dissim = FLT_MAX;
  region_heap_index = UINT_MAX;
#endif
  merge_region_label = 0;
  sum_pixel_gdissim = 0.0;
  min_region_dissim = FLT_MAX;
  boundary_npix = 0;
  merge_threshold = 0.0;
  nb_region_objects = 0;
  nghbrs_label_set.clear();
  region_objects_set.clear();
#ifdef RHSEG_RUN
  best_nghbr_label.clear();
  best_region_label.clear();
  nghbrs_label_seam_map.clear();
#endif // RHSEG_RUN

  return;
 }

 void RegionClass::clear()
 {
      int band;

      active_flag = false;
#ifdef RHSEG_RUN
      initial_merge_flag = false;
      seam_flag = false;
      merged_flag = true;
      large_nghbr_merged_flag = false;
#endif
// Don't clear label!!!
      npix = 0;
#ifdef RHSEG_RUN
      nghbr_heap_npix = 0;
      region_heap_npix = 0;
#endif
      for (band = 0; band < nbands; band++)
        sum[band] = 0.0;
      if (sumsq_flag)
      {
        for (band = 0; band < nbands; band++)
          sumsq[band] = 0.0;
      }
      if (sumxlogx_flag)
      {
        for (band = 0; band < nbands; band++)
          sumxlogx[band] = 0.0;
      }
      if (std_dev_flag)
      {
        for (band = 0; band < nbands; band++)
          sum_pixel_std_dev[band] = 0.0;
      }
      max_edge_value = -FLT_MAX;
      band_max_std_dev = -1.0;
      nghbrs_label_set.clear();
#ifdef RHSEG_RUN
      best_nghbr_label.clear();
      best_nghbr_dissim = FLT_MAX;
      nghbr_heap_index = UINT_MAX;
      best_region_label.clear();
      best_region_dissim = FLT_MAX;
      region_heap_index = UINT_MAX;
      nghbrs_label_seam_map.clear();
#endif
      merge_region_label = 0;
      sum_pixel_gdissim = 0.0;
      min_region_dissim = FLT_MAX;
      nb_region_objects = 0;
      region_objects_set.clear();
      boundary_npix = 0;
      merge_threshold = 0.0;

      return;
 }

 void RegionClass::partial_clear()
 {
   max_edge_value = -FLT_MAX;
   band_max_std_dev = -1.0;
#ifdef RHSEG_RUN
   best_nghbr_dissim = FLT_MAX;
   nghbr_heap_index = UINT_MAX;
   best_region_dissim = FLT_MAX;
   region_heap_index = UINT_MAX;
#endif
   merge_region_label = 0;
   sum_pixel_gdissim = 0.0;
   min_region_dissim = FLT_MAX;
   boundary_npix = 0;
   merge_threshold = 0.0;
   nb_region_objects = 0;
   nghbrs_label_set.clear();
   region_objects_set.clear();
#ifdef RHSEG_RUN
   best_nghbr_label.clear();
   best_region_label.clear();
   nghbrs_label_seam_map.clear();
#endif // RHSEG_RUN

   return;
 }

#ifdef RHSEG_RUN
 void RegionClass::clear_best_merge()
 {
      best_nghbr_label.clear();
      best_nghbr_dissim = FLT_MAX;
      best_region_label.clear();
      best_region_dissim = FLT_MAX;
      nghbrs_label_seam_map.clear();
      merge_region_label = 0;
    // Don't clear initial_merge_flag or seam_flag!
      merged_flag = true;
      large_nghbr_merged_flag = false;

      return;
 }
#endif // RHSEG_RUN

 void RegionClass::clear_region_info()
 {
      boundary_npix = 0;
      region_objects_set.clear();

      return;
 }

 void RegionClass::nghbrs_label_set_copy(const RegionClass& source)
 {
    nghbrs_label_set.clear();
    if (!source.nghbrs_label_set.empty())
    {
      set<unsigned int>::const_iterator nghbrs_label_set_iter = source.nghbrs_label_set.begin();
      while (nghbrs_label_set_iter != source.nghbrs_label_set.end())
      {
        nghbrs_label_set.insert(*nghbrs_label_set_iter);
        ++nghbrs_label_set_iter;
      }
    }

    return;
 }

#ifdef RHSEG_RUN
 void RegionClass::best_nghbr_copy(const RegionClass& source)
 {
    best_nghbr_label.clear();
    if (!source.best_nghbr_label.empty())
    {
      set<unsigned int>::const_iterator best_nghbr_iter = source.best_nghbr_label.begin();
      while (best_nghbr_iter != source.best_nghbr_label.end())
      {
        best_nghbr_label.insert(*best_nghbr_iter);
        ++best_nghbr_iter;
      }
      best_nghbr_dissim = source.best_nghbr_dissim;
    }
    else
      best_nghbr_dissim = FLT_MAX;

    return;
 }

 void RegionClass::best_region_copy(const RegionClass& source)
 {
    best_region_label.clear();
    if (!source.best_region_label.empty())
   {
      set<unsigned int>::const_iterator best_region_iter = source.best_region_label.begin();
      while (best_region_iter != source.best_region_label.end())
      {
        best_region_label.insert(*best_region_iter);
        ++best_region_iter;
      }
      best_region_dissim = source.best_region_dissim;
    }
    else
      best_region_dissim = FLT_MAX;

    return;
 }

 void RegionClass::nghbrs_label_seam_map_copy(const RegionClass& source)
 {
    nghbrs_label_seam_map.clear();
    if (!source.nghbrs_label_seam_map.empty())
    {
      map<unsigned int, RegionSeam>::const_iterator seam_nghbr_iter = source.nghbrs_label_seam_map.begin();
      while (seam_nghbr_iter != source.nghbrs_label_seam_map.end())
      {
        nghbrs_label_seam_map.insert(make_pair((*seam_nghbr_iter).first,(*seam_nghbr_iter).second));
        ++seam_nghbr_iter;
      }
    }

    return;
 }
#endif // RHSEG_RUN

 void RegionClass::region_objects_set_copy(const RegionClass& source)
 {
    region_objects_set.clear();
    if (!source.region_objects_set.empty())
    {
      set<unsigned int>::const_iterator region_object_iter = source.region_objects_set.begin();
      while (region_object_iter != source.region_objects_set.end())
      {
        region_objects_set.insert(*region_object_iter);
        ++region_object_iter;
      }
    }

    return;
 }

#ifdef RHSEG_RUN
 void RegionClass::set_min_region_dissim()
 {
 // Current region is compared to a minimum vector region of indeterminant size.
   int band;
   double reg1_mean, reg2_mean;
   double sqdiff, sumsqdiff;

   sumsqdiff = 0.0;
   for (band=0; band < nbands; band++)
   {
     reg1_mean = sum[band]/((double) npix);
     reg2_mean = minval[band];
     sqdiff = reg1_mean - reg2_mean;
     if ((params.dissim_crit == 2) || (params.dissim_crit == 4) || (params.dissim_crit == 5) ||
         (params.dissim_crit == 6) || (params.dissim_crit == 7) || (params.dissim_crit == 8) ||
         (params.dissim_crit == 9))
       sqdiff = sqdiff*sqdiff;
     else if (sqdiff < 0.0)
       sqdiff = -sqdiff;
     if ((params.dissim_crit == 3) || (params.dissim_crit == 7))
     {
       if (sqdiff > sumsqdiff)
         sumsqdiff = sqdiff;
     }
     else
       sumsqdiff += sqdiff;
   }
   if ((params.dissim_crit == 2) || (params.dissim_crit == 4) || (params.dissim_crit == 5) ||
       (params.dissim_crit == 8) || (params.dissim_crit == 9))
     sumsqdiff = sqrt(sumsqdiff);
#ifdef MSE_SQRT
   if ((params.dissim_crit == 6) || (params.dissim_crit == 7))
   {
     sumsqdiff = sqrt(sumsqdiff);  // Added to make dimensionality consistent.
   }
#endif

   min_region_dissim = sumsqdiff;

   return;
 }

 void RegionClass::reset_merged_flag()
 {
   merged_flag = false;

   return;
 }

// Used in first_merge_reg_grow
#ifdef THREEDIM
 void RegionClass::fm_init(vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                           const int& col, const int& row, const int& slice,
                           const int& ncols, const int& nrows, const int& nslices)
#else
 void RegionClass::fm_init(vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                           const int& col, const int& row,
                           const int& ncols, const int& nrows)
#endif
 {
   int band, nbdir;
   unsigned int pixel_index, nbpixel_index;
   int nbcol, nbrow;
#ifdef THREEDIM
   int nbslice;
#endif
   double temp_value;

#ifdef THREEDIM
   pixel_index = col + row*ncols + slice*nrows*ncols;
#else
   pixel_index = col + row*ncols;
#endif

   if (pixel_data[pixel_index].mask)
   {
     active_flag = true;

     merged_flag = true;  // To prevent unnecessary caculations until convergence checking is initiated.
     large_nghbr_merged_flag = false;

     npix++;
     nghbr_heap_npix = npix;
     region_heap_npix = npix;
     for (band = 0; band < nbands; band++)
     {
       temp_value = (double) pixel_data[pixel_index].get_input_data(band);
       temp_value = scale[band]*(temp_value - offset[band]);
       sum[band] += temp_value;
       if (sumsq_flag)
         sumsq[band] += temp_value*temp_value;
       if (sumxlogx_flag)
         sumxlogx[band] += temp_value*log(temp_value);
       if (std_dev_flag)
       {
         temp_value = pixel_data[pixel_index].get_local_std_dev(band);
         temp_value = scale[band]*temp_value;
         sum_pixel_std_dev[band] += temp_value;
       }
     }

     if (params.edge_image_flag)
     {
       if (pixel_data[pixel_index].get_edge_mask())
       {
         if (pixel_data[pixel_index].get_edge_value() > max_edge_value)
           max_edge_value = pixel_data[pixel_index].get_edge_value();
       }
     }

     for (nbdir = 0; nbdir < params.maxnbdir; ++nbdir)
     {
#ifdef THREEDIM
       find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
       if ((nbslice>=0)&&(nbrow>=0)&&(nbcol>=0)&&(nbslice<nslices)&&(nbrow<nrows)&&(nbcol<ncols))
       {
         nbpixel_index = nbcol + nbrow*ncols + nbslice*nrows*ncols;
#else
       find_nghbr(col,row,nbdir,nbcol,nbrow);
       if ((nbrow>=0)&&(nbcol>=0)&&(nbrow<nrows)&&(nbcol<ncols))
       {
         nbpixel_index = nbcol + nbrow*ncols;
#endif
         if (pixel_data[nbpixel_index].mask)
         {
           if ((pixel_data[nbpixel_index].region_label == 0) &&
               (!pixel_data[nbpixel_index].init_flag))
           {
             nghbrs_label_set.insert(nbpixel_index);
           }
         }
       }
     } // for (nbdir = 0; nbdir < params.maxnbdir; ++nbdir)
   }

   return;
 }

// Used in first_merge_reg_grow
 bool RegionClass::find_merge(vector<Pixel>& pixel_data)
 {
   unsigned int nbpixel_index;
   float region_dissim;
   set<unsigned int>::const_iterator nghbrs_label_set_iter;
   vector<unsigned int> nghbrs_label_vector;

   best_nghbr_dissim = FLT_MAX;
   best_nghbr_label.clear();
   nghbrs_label_vector.clear();
   nghbrs_label_set_iter = nghbrs_label_set.begin();
   while (nghbrs_label_set_iter != nghbrs_label_set.end())
   {
     nbpixel_index = *nghbrs_label_set_iter;
     nghbrs_label_vector.push_back(nbpixel_index);
     nghbrs_label_set_iter++;
   }
   if (params.init_threshold > 0.0)
     random_shuffle(nghbrs_label_vector.begin(),nghbrs_label_vector.end());

   vector<unsigned int>::const_iterator nghbrs_label_viter;
   nghbrs_label_viter = nghbrs_label_vector.begin();
   while ((nghbrs_label_viter != nghbrs_label_vector.end()) &&
          (best_nghbr_dissim > params.init_threshold))
   {
     nbpixel_index = *nghbrs_label_viter;
     if (pixel_data[nbpixel_index].get_mask())
     {
       if ((params.edge_image_flag) && (params.edge_threshold > 0.0))
         region_dissim = calc_edge_pixel_dissim(this,&pixel_data[nbpixel_index]);
       else
         region_dissim = (float) calc_region_pixel_dissim(this,&pixel_data[nbpixel_index]);
       if (region_dissim <= params.init_threshold)
       {
         best_nghbr_label.insert(nbpixel_index);
         best_nghbr_dissim = region_dissim;
       }
       else
       {
         nghbrs_label_set.erase(nbpixel_index);  // Eliminate this pixel from further consideration!
       }
     }
     ++nghbrs_label_viter;
   } // while (nghbrs_label_viter != nghbrs_label_vector.end())

   if (best_nghbr_dissim > params.init_threshold)
     return false;
   else
     return true;
 }

// Used in first_merge_reg_grow
#ifdef THREEDIM
 void RegionClass::do_merge(const int &ncols, const int& nrows, const int& nslices, const unsigned int& nregions,
                            vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, unsigned int& region_classes_size)
#else
 void RegionClass::do_merge(const int &ncols, const int& nrows, const unsigned int& nregions,
                            vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, unsigned int& region_classes_size)
#endif
 {
    unsigned int nbpixel_index, region_label;
    int col, row;
#ifdef THREEDIM
    int slice;
#endif

    nbpixel_index = *best_nghbr_label.begin();

    nghbrs_label_set.erase(nbpixel_index);

    if (pixel_data[nbpixel_index].get_mask())
    {
      region_label = nregions+1;
      if (region_classes_size < region_label)
      {
        region_classes_size++;
        region_classes.resize(region_classes_size);
      }

      region_classes[nregions].clear();
      region_classes[nregions].set_label(region_label);
#ifdef THREEDIM
      slice = nbpixel_index/(nrows*ncols);
      row = ((int) nbpixel_index - slice*((int) nrows*ncols))/ncols;
      col = ((int) nbpixel_index) - row*((int) ncols) - slice*((int) (nrows*ncols));;
      region_classes[nregions].fm_init(pixel_data, region_classes, col, row, slice, ncols, nrows, nslices);
#else
      row = nbpixel_index/ncols;
      col = ((int) nbpixel_index) - row*((int) ncols);
      region_classes[nregions].fm_init(pixel_data, region_classes, col, row, ncols, nrows);
#endif
      this->merge_regions(&region_classes[nregions],region_classes);
      pixel_data[nbpixel_index].set_region_label(label);
      pixel_data[nbpixel_index].set_init_flag(true);
    }

    return;
 }

// Used in first_merge_reg_grow (via do_merge)
 void RegionClass::merge_regions(RegionClass *merge_region, vector<RegionClass>& region_classes)
 {
    int band;

    if (params.debug > 2)
    {
      params.log_fs << endl << "Merging region " << merge_region->label << " into region " << this->label;
      params.log_fs << " with merge threshold = " << params.init_threshold << endl;
    }
#ifdef DEBUG
    if (params.debug > 3)
    {
      merge_region->print(region_classes);
      this->print(region_classes);
    }
#endif
    if (!active_flag)
    {
      if (params.debug > 0)
      {
        params.log_fs << "Region label " << label << " is inactive:  can't participate in a merge!!" << endl;
        this->print(region_classes);
      }
      else
        cout << "Region label " << label << " is inactive:  can't participate in a merge!!" << endl;
    }
    if (!(merge_region->active_flag))
    {
      if (params.debug > 0)
      {
        params.log_fs << "Region label " << merge_region->label << " is inactive:  can't participate in a merge!!" << endl;
        merge_region->print(region_classes);
      }
      else
        cout << "Region label " << merge_region->label << " is inactive:  can't participate in a merge!!" << endl;
    }

  // Update merge_threshold
    merge_threshold = params.init_threshold;

// Update merged region npix, sum values, and, if necessary, update
// the sumsq and sum_pixel_std_dev values.
    npix += merge_region->npix;
    for (band = 0; band < nbands; band++)
      sum[band] += merge_region->sum[band];
    if (sumsq_flag)
    {
      for (band = 0; band < nbands; band++)
        sumsq[band] += merge_region->sumsq[band];
    }
    if (sumxlogx_flag)
    {
      for (band = 0; band < nbands; band++)
        sumxlogx[band] += merge_region->sumxlogx[band];
    }
    if (std_dev_flag)
    {
      for (band = 0; band < nbands; band++)
        sum_pixel_std_dev[band] += merge_region->sum_pixel_std_dev[band];
    }
    if ((params.edge_image_flag) && (max_edge_value < merge_region->max_edge_value))
      max_edge_value = merge_region->max_edge_value;

// Merge neighbor label lists
    if (!merge_region->nghbrs_label_set.empty())
    {
      set<unsigned int>::const_iterator nghbrs_label_set_iter = merge_region->nghbrs_label_set.begin();
      while (nghbrs_label_set_iter != merge_region->nghbrs_label_set.end())
      {
        nghbrs_label_set.insert(*nghbrs_label_set_iter);
        ++nghbrs_label_set_iter;
      }
    }

// set merge_region.
    merge_region->active_flag = false;
    merge_region->merged_flag = true;
    merge_region->merge_region_label = label;
#ifdef DEBUG
    if (params.debug > 2)
    {
      params.log_fs << "After merging: " << endl;
      merge_region->print(region_classes);
      this->print(region_classes);
    }
#endif
    return;
 }

#ifdef THREEDIM
 void RegionClass::init(vector<Pixel>& pixel_data,
                        const int& col, const int& row, const int& slice,
                        const int& ncols, const int& nrows, const int& nslices)
#else
 void RegionClass::init(vector<Pixel>& pixel_data,
                        const int& col, const int& row,
                        const int& ncols, const int& nrows)
#endif
 {
   int band;
   unsigned int pixel_index;
   double temp_value;

#ifdef THREEDIM
   pixel_index = col + row*ncols + slice*nrows*ncols;
#else
   pixel_index = col + row*ncols;
#endif

   if ((pixel_data[pixel_index].mask) && (pixel_data[pixel_index].region_label > 0))
   {
     active_flag = true;

     merged_flag = true;   // To prevent unnecessary calculations until convergence checking is initiated.
     large_nghbr_merged_flag = false;

     label = pixel_data[pixel_index].region_label;
     npix++;
     nghbr_heap_npix = npix;
     region_heap_npix = npix;
     for (band = 0; band < nbands; band++)
     {
       temp_value = (double) pixel_data[pixel_index].get_input_data(band);
       temp_value = scale[band]*(temp_value - offset[band]);
       sum[band] += temp_value;
       if (sumsq_flag)
         sumsq[band] += temp_value*temp_value;
       if (sumxlogx_flag)
         sumxlogx[band] += temp_value*log(temp_value);
       if (std_dev_flag)
       {
         temp_value = pixel_data[pixel_index].get_local_std_dev(band);
         temp_value = scale[band]*temp_value;
         sum_pixel_std_dev[band] += temp_value;
       }
     }
     if ((pixel_data[pixel_index].edge_mask) && (max_edge_value < pixel_data[pixel_index].edge_value))
       max_edge_value = pixel_data[pixel_index].edge_value;

#ifdef THREEDIM
     this->nghbrs_label_set_init(pixel_data, col, row, slice, ncols, nrows, nslices);
#else
     this->nghbrs_label_set_init(pixel_data, col, row, ncols, nrows);
#endif
   } // if ((pixel_data[pixel_index].mask) && (pixel_data[pixel_index].region_label > 0))

   return;
 }

#ifdef THREEDIM
 void RegionClass::col_init(vector<Index>& col_seam_index_data,
                            const int& col, const int& row, const int& slice,
                            const int& nrows, const int& nslices)
#else
 void RegionClass::col_init(vector<Index>& col_seam_index_data,
                            const int& col, const int& row, const int& nrows)
#endif
 {
   short unsigned int nbdir;
   unsigned int nbindex, nblabel;
   int nbcol, nbrow;
#ifdef THREEDIM
   int nbslice;
#endif

   if (params.nb_dimensions == 1)
   {
     if ((col == 0) && (params.maxnbdir > 6))
     {
       for (nbdir = 7; nbdir < params.maxnbdir; nbdir += 2)
       {
#ifdef THREEDIM
         find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
         find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
         if ((nbcol>=0) && (nbcol<params.seam_size))
         {
           nbindex = nbcol;
           nblabel = col_seam_index_data[nbindex].region_class_label;
           if ((nblabel != 0) && (label != nblabel))
             nghbrs_label_set.insert(nblabel);
         }
       }
     }
     else if ((col == 1) && (params.maxnbdir > 4))
     {
       for (nbdir = 5; nbdir < params.maxnbdir; nbdir += 2)
       {
#ifdef THREEDIM
         find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
         find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
         if ((nbcol>=0) && (nbcol<params.seam_size))
         {
           nbindex = nbcol;
           nblabel = col_seam_index_data[nbindex].region_class_label;
           if ((nblabel != 0) && (label != nblabel))
             nghbrs_label_set.insert(nblabel);
         }
       }
     }
     else if ((col == 2) && (params.maxnbdir > 2))
     {
       for (nbdir = 3; nbdir < params.maxnbdir; nbdir += 2)
       {
#ifdef THREEDIM
         find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
         find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
         if ((nbcol>=0) && (nbcol<params.seam_size))
         {
           nbindex = nbcol;
           nblabel = col_seam_index_data[nbindex].region_class_label;
           if ((nblabel != 0) && (label != nblabel))
             nghbrs_label_set.insert(nblabel);
         }
       }
     }
     else if (col == 3)
     {
       for (nbdir = 1; nbdir < params.maxnbdir; nbdir += 2)
       {
#ifdef THREEDIM
         find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
         find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
         if ((nbcol>=0) && (nbcol<params.seam_size))
         {
           nbindex = nbcol;
           nblabel = col_seam_index_data[nbindex].region_class_label;
           if ((nblabel != 0) && (label != nblabel))
             nghbrs_label_set.insert(nblabel);
         }
       }
     }
     else if (col == 4)
     {
       for (nbdir = 0; nbdir < params.maxnbdir; nbdir += 2)
       {
#ifdef THREEDIM
         find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
         find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
         if ((nbcol>=0) && (nbcol<params.seam_size))
         {
           nbindex = nbcol;
           nblabel = col_seam_index_data[nbindex].region_class_label;
           if ((nblabel != 0) && (label != nblabel))
             nghbrs_label_set.insert(nblabel);
         }
       }
     }
     else if ((col == 5) && (params.maxnbdir > 1))
     {
       for (nbdir = 2; nbdir < params.maxnbdir; nbdir += 2)
       {
#ifdef THREEDIM
         find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
         find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
         if ((nbcol>=0) && (nbcol<params.seam_size))
         {
           nbindex = nbcol;
           nblabel = col_seam_index_data[nbindex].region_class_label;
           if ((nblabel != 0) && (label != nblabel))
             nghbrs_label_set.insert(nblabel);
         }
       }
     }
     else if ((col == 6) && (params.maxnbdir > 3))
     {
       for (nbdir = 2; nbdir < params.maxnbdir; nbdir += 2)
       {
#ifdef THREEDIM
         find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
         find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
         if ((nbcol>=0) && (nbcol<params.seam_size))
         {
           nbindex = nbcol;
           nblabel = col_seam_index_data[nbindex].region_class_label;
           if ((nblabel != 0) && (label != nblabel))
             nghbrs_label_set.insert(nblabel);
         }
       }
     }
     else if ((col == 7) && (params.maxnbdir > 5))
     {
       for (nbdir = 2; nbdir < params.maxnbdir; nbdir += 2)
       {
#ifdef THREEDIM
         find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
         find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
         if ((nbcol>=0) && (nbcol<params.seam_size))
         {
           nbindex = nbcol;
           nblabel = col_seam_index_data[nbindex].region_class_label;
           if ((nblabel != 0) && (label != nblabel))
             nghbrs_label_set.insert(nblabel);
         }
       }
     }
   }
   else if (params.nb_dimensions == 2)
   {
     if ((col == 0) && (params.maxnbdir > 10))
     {
       for (nbdir = 11; nbdir < params.maxnbdir; nbdir += 2)
       {
         if ((nbdir != 13) && (nbdir != 19))
         {
#ifdef THREEDIM
           find_nghbr(row,col,slice,nbdir,nbrow,nbcol,nbslice);
#else
           find_nghbr(row,col,nbdir,nbrow,nbcol);
#endif
           if ((nbrow>=0) && (nbcol>=0) &&
               (nbrow<nrows) && (nbcol<params.seam_size))
           {
             nbindex = nbrow + + nbcol*nrows;
             nblabel = col_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
     else if (col == 1)
     {
       for (nbdir = 3; nbdir < params.maxnbdir; nbdir += 2)
       {
         if (nbdir != 9)
         {
#ifdef THREEDIM
           find_nghbr(row,col,slice,nbdir,nbrow,nbcol,nbslice);
#else
           find_nghbr(row,col,nbdir,nbrow,nbcol);
#endif
           if ((nbrow>=0) && (nbcol>=0) &&
               (nbrow<nrows) && (nbcol<params.seam_size))
           {
             nbindex = nbrow + nbcol*nrows;
             nblabel = col_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
     else if (col == 2)
     {
       for (nbdir = 2; nbdir < params.maxnbdir; nbdir += 2)
       {
         if (nbdir != 8)
         {
#ifdef THREEDIM
           find_nghbr(row,col,slice,nbdir,nbrow,nbcol,nbslice);
#else
           find_nghbr(row,col,nbdir,nbrow,nbcol);
#endif
           if ((nbrow>=0) && (nbcol>=0) &&
               (nbrow<nrows) && (nbcol<params.seam_size))
           {
             nbindex = nbrow + nbcol*nrows;
             nblabel = col_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
     else if ((col == 3) && (params.maxnbdir > 9))
     {
       for (nbdir = 10; nbdir < params.maxnbdir; nbdir += 2)
       {
         if ((nbdir != 12) && (nbdir != 18))
         {
#ifdef THREEDIM
           find_nghbr(row,col,slice,nbdir,nbrow,nbcol,nbslice);
#else
           find_nghbr(row,col,nbdir,nbrow,nbcol);
#endif
           if ((nbrow>=0) && (nbcol>=0) &&
               (nbrow<nrows) && (nbcol<params.seam_size))
           {
             nbindex = nbrow + nbcol*nrows;
             nblabel = col_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
   }
#ifdef THREEDIM
   else if (params.nb_dimensions == 3)
   {
     if (col == 0)
     {
       for (nbdir = 5; nbdir < params.maxnbdir; nbdir += 2)
       {
         if ((nbdir != 7) && (nbdir != 9))
         {
           find_nghbr(row,slice,col,nbdir,nbrow,nbslice,nbcol);
           if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
               (nbslice<nslices) && (nbrow<nrows) && (nbcol<params.seam_size))
           {
             nbindex = nbrow + nbslice*nrows + nbcol*nslices*nrows;
             nblabel = col_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
     else if (col == 1)
     {
       for (nbdir = 4; nbdir < params.maxnbdir; nbdir += 2)
       {
         if ((nbdir != 6) && (nbdir != 8))
         {
           find_nghbr(row,slice,col,nbdir,nbrow,nbslice,nbcol);
           if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
               (nbslice<nslices) && (nbrow<nrows) && (nbcol<params.seam_size))
           {
             nbindex = nbrow + nbslice*nrows + nbcol*nslices*nrows;
             nblabel = col_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
   }
#endif // THREEDIM
   return;
 }
#ifdef THREEDIM
 void RegionClass::row_init(vector<Index>& row_seam_index_data,
                            const int& col, const int& row, const int& slice,
                            const int& ncols, const int& nslices)
#else
 void RegionClass::row_init(vector<Index>& row_seam_index_data,
                            const int& col, const int& row, const int& ncols)
#endif
 {
   short unsigned int nbdir;
   unsigned int nbindex, nblabel;
   int nbcol, nbrow;
#ifdef THREEDIM
   int nbslice;
#endif

   if (params.nb_dimensions == 2)
   {
     if ((row == 0) && (params.maxnbdir > 10))
     {
       for (nbdir = 11; nbdir < params.maxnbdir; nbdir += 2)
       {
         if ((nbdir != 13) && (nbdir != 19))
         {
#ifdef THREEDIM
           find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
           find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
           if ((nbrow>=0) && (nbcol>=0) &&
               (nbrow<params.seam_size) && (nbcol<ncols))
           {
             nbindex = nbcol + nbrow*ncols;
             nblabel = row_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
     else if (row == 1)
     {
       for (nbdir = 3; nbdir < params.maxnbdir; nbdir += 2)
       {
         if (nbdir != 9)
         {
#ifdef THREEDIM
           find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
           find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
           if ((nbrow>=0) && (nbcol>=0) &&
               (nbrow<params.seam_size) && (nbcol<ncols))
           {
             nbindex = nbcol + nbrow*ncols;
             nblabel = row_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
     else if (row == 2)
     {
       for (nbdir = 2; nbdir < params.maxnbdir; nbdir += 2)
       {
         if (nbdir != 8)
         {
#ifdef THREEDIM
           find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
           find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
           if ((nbrow>=0) && (nbcol>=0) &&
               (nbrow<params.seam_size) && (nbcol<ncols))
           {
             nbindex = nbcol + nbrow*ncols;
             nblabel = row_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
     else if ((row == 3) && (params.maxnbdir > 9))
     {
       for (nbdir = 10; nbdir < params.maxnbdir; nbdir += 2)
       {
         if ((nbdir != 12) && (nbdir != 18))
         {
#ifdef THREEDIM
           find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
#else
           find_nghbr(col,row,nbdir,nbcol,nbrow);
#endif
           if ((nbrow>=0) && (nbcol>=0) &&
               (nbrow<params.seam_size) && (nbcol<ncols))
           {
             nbindex = nbcol + nbrow*ncols;
             nblabel = row_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
   }
#ifdef THREEDIM
   else if (params.nb_dimensions == 3)
   {
     if (row == 0)
     {
       for (nbdir = 5; nbdir < params.maxnbdir; nbdir += 2)
       {
         if ((nbdir != 7) && (nbdir != 9))
         {
           find_nghbr(slice,col,row,nbdir,nbslice,nbcol,nbrow);
           if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
               (nbslice<nslices) && (nbrow<params.seam_size) && (nbcol<ncols))
           {
             nbindex = nbcol + nbslice*ncols + nbrow*nslices*ncols;
             nblabel = row_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
     else if (row == 1)
     {
       for (nbdir = 4; nbdir < params.maxnbdir; nbdir += 2)
       {
         if ((nbdir != 6) && (nbdir != 8))
         {
           find_nghbr(slice,col,row,nbdir,nbslice,nbcol,nbrow);
           if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
               (nbslice<nslices) && (nbrow<params.seam_size) && (nbcol<ncols))
           {
             nbindex = nbcol + nbslice*ncols + nbrow*nslices*ncols;
             nblabel = row_seam_index_data[nbindex].region_class_label;
             if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
           }
         }
       }
     }
   }
#endif
   return;
 }
#ifdef THREEDIM
 void RegionClass::slice_init(vector<Index>& slice_seam_index_data,
                              const int& col, const int& row, const int& slice,
                              const int& ncols, const int& nrows)
 {
   short unsigned int nbdir;
   unsigned int nbindex, nblabel;
   int nbcol, nbrow, nbslice;

   if (slice == 0)
   {
     for (nbdir = 5; nbdir < params.maxnbdir; nbdir += 2)
     {
       if ((nbdir != 7) && (nbdir != 9))
       {
         find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
         if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
             (nbslice<params.seam_size) && (nbrow<nrows) && (nbcol<ncols))
         {
           nbindex = nbcol + nbrow*ncols + nbslice*nrows*ncols;
           nblabel = slice_seam_index_data[nbindex].region_class_label;
           if ((nblabel != 0) && (label != nblabel))
               nghbrs_label_set.insert(nblabel);
         }
       }
     }
   }
   else if (slice == 1)
   {
     for (nbdir = 4; nbdir < params.maxnbdir; nbdir += 2)
     {
       if ((nbdir != 6) && (nbdir != 8))
       {
         find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
         if ((nbslice>=0) && (nbrow>=0) && (nbcol>=0) &&
             (nbslice<params.seam_size) && (nbrow<nrows) && (nbcol<ncols))
         {
           nbindex = nbcol + nbrow*ncols + nbslice*nrows*ncols;
           nblabel = slice_seam_index_data[nbindex].region_class_label;
           if ((nblabel != 0) && (label != nblabel))
             nghbrs_label_set.insert(nblabel);
         }
       }
     }
   }

   return;
 }
#endif

#ifdef THREEDIM
 void RegionClass::nghbrs_label_set_init(vector<Pixel>& pixel_data,
                                         const int& col, const int& row, const int& slice,
                                         const int& ncols, const int& nrows, const int& nslices)
#else
 void RegionClass::nghbrs_label_set_init(vector<Pixel>& pixel_data,
                                         const int& col, const int& row, const int& ncols, const int& nrows)
#endif
 {
 // Initializes this region's nghbrs_label_set from the pixel_data information
   short unsigned int nbdir;
   unsigned int nghbr_pixel_index;
   int nbcol, nbrow;
#ifdef THREEDIM
   int nbslice;
#endif

   for (nbdir = 0; nbdir < params.maxnbdir; ++nbdir)
   {
#ifdef THREEDIM
     find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
     if ((nbcol>=0)&&(nbrow>=0)&&(nbslice>=0)&&
         (nbcol<ncols)&&(nbrow<nrows)&&(nbslice<nslices))
     {
       nghbr_pixel_index = nbcol + nbrow*ncols + nbslice*nrows*ncols;
#else
     find_nghbr(col,row,nbdir,nbcol,nbrow);
     if ((nbcol>=0)&&(nbrow>=0)&&(nbcol<ncols)&&(nbrow<nrows))
     {
       nghbr_pixel_index = nbcol + nbrow*ncols;
#endif
       if ((pixel_data[nghbr_pixel_index].mask) &&
           (label != pixel_data[nghbr_pixel_index].region_label))
       {
         nghbrs_label_set.insert(pixel_data[nghbr_pixel_index].region_label);
       }
     }
   }

   return;
 }

#endif // RHSEG_RUN
/*
 void RegionClass::calc_std_dev()
 {
 // Postcondition: If npix <= 1, std_dev is set to 0.0.  If npix > 1,
 // std_dev is the region standard deviation.

  std_dev = 0.0;
  if (npix <= 1)
    return;

  int band;
  float  sumf, sumsqf, tempf;
  double tempd;
  double numpix = (double) npix;

  for (band = 0; band < nbands; band++)
  {
*//*  The original code for at this point was:
    tempd = (sumsq[band] - ((sum[band]*sum[band])/numpix))/(numpix-1.0);
    However, minor differences in the value of sumsq due to different
    order of summation performed by instances of the program run with
    different number of processors was magnified by this form of the
    equation.  The following four lines of code reduce or eliminate
    this magnification of differences of value of sumsq - even though
    the calculations are performed in single rather than double precision!!
*//*
    sumf = (float) ((sum[band]*sum[band])/numpix);
    sumsqf = (float) sumsq[band];
    tempf = sumsqf - sumf;
    tempd = tempf/(numpix-1.0);
    if (tempd > 0.0)
      tempd = sqrt(tempd);
    else
      tempd = 0.0;
  // Use mean normalized std_dev in place of plain std_dev
    tempd = tempd*(numpix/sum[band]);
    if (tempd > std_dev)
      std_dev = tempd;
  }

  return;
 }
*/

 void RegionClass::set_band_max_std_dev()
 {
   if (std_dev_flag)
     band_max_std_dev = this->get_band_max_std_dev();
   else
   {
     band_max_std_dev = 0.0;

     if (npix <= 1)
       return;

     int band;
     double stdDev, numpix = (double) npix;

     for (band = 0; band < nbands; band++)
     {
       stdDev = (sumsq[band] - ((sum[band]*sum[band])/numpix))/(numpix-1.0);
       if (stdDev > 0.0)
         stdDev = sqrt(stdDev);
       else
         stdDev = 0.0;
       if (stdDev > band_max_std_dev)
         band_max_std_dev = stdDev;
     }

     return;
   }
 }

#ifdef RHSEG_RUN
 void RegionClass::best_nghbr_init(vector<RegionClass>& region_classes)
 {
   bool edge_process_flag;
   nghbr_heap_npix = npix;
   if (!nghbrs_label_set.empty())
   {
     double result, edge_result, edge_factor;
     RegionClass *nghbr_region_ptr;
     set<unsigned int>::iterator nghbrs_label_set_iter = nghbrs_label_set.begin();
     while (nghbrs_label_set_iter != nghbrs_label_set.end())
     {
       nghbr_region_ptr = &region_classes[(*nghbrs_label_set_iter)-1];

       result = calc_region_dissim(this,nghbr_region_ptr,params.merge_accel_flag);

       edge_process_flag = (params.edge_wght > 0.0);
       if ((params.initial_merge_flag) && (!seam_flag) && (!nghbr_region_ptr->seam_flag))
       {
         if ((initial_merge_flag) && (nghbr_region_ptr->initial_merge_flag))
         {
           result /= params.spclust_wght;
           edge_process_flag = false;
         }
       }
       if (edge_process_flag)
       {
         edge_result = max_edge_value;
         if (nghbr_region_ptr->max_edge_value > edge_result)
           edge_result = nghbr_region_ptr->max_edge_value;
         if (edge_result < 0.0)
         {
           if (params.edge_dissim_option == 1)
             edge_result = params.max_edge_value;
           else
             edge_result = params.min_edge_value;
         }
         edge_factor = (edge_result - params.min_edge_value)/(params.max_edge_value - params.min_edge_value);
         edge_factor = pow((double) edge_factor, (double) params.edge_power);
         edge_factor = (1.0 - params.edge_wght) + edge_factor*params.edge_wght;
         if (params.edge_dissim_option == 2)
           edge_factor = (params.spclust_wght + (1.0 - params.spclust_wght)*edge_factor)/params.spclust_wght;
         result *= edge_factor;
       }
       float float_result = (float) result;

       if (float_result < best_nghbr_dissim)
       {
         best_nghbr_label.clear();
         best_nghbr_label.insert(nghbr_region_ptr->label);
         best_nghbr_dissim = float_result;
       }
       else if (float_result == best_nghbr_dissim)
       {
         best_nghbr_label.insert(nghbr_region_ptr->label);
       }
       ++nghbrs_label_set_iter;
     }
   }

   return;
 }

 void RegionClass::best_region_init(const unsigned int& heap_index,
                                    vector<RegionClass *>& region_heap, const unsigned int& region_heap_size)
 {
   unsigned int other_heap_index;
   float result;

   region_heap_npix = npix;
   for (other_heap_index = (heap_index+1); other_heap_index < region_heap_size; ++other_heap_index)
   {
     result = (float) calc_region_dissim(this,region_heap[other_heap_index],false);
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << "Region dissimilarity between regions with labels " << this->label;
       params.log_fs << " and " << region_heap[other_heap_index]->label << " is " << result << endl;
     }
#endif
     if (result < best_region_dissim)
     {
       best_region_label.clear();
       best_region_label.insert(region_heap[other_heap_index]->label);
       best_region_dissim = result;
     }
     else if (result == best_region_dissim)
     {
       best_region_label.insert(region_heap[other_heap_index]->label);
     }

     if (result < region_heap[other_heap_index]->best_region_dissim)
     {
       region_heap[other_heap_index]->best_region_label.clear();
       region_heap[other_heap_index]->best_region_label.insert(label);
       region_heap[other_heap_index]->best_region_dissim = result;
     }
     else if (result == region_heap[other_heap_index]->best_region_dissim)
     {
       region_heap[other_heap_index]->best_region_label.insert(label);
     }
   } // for (other_heap_index = (heap_index+1); other_heap_index < region_heap_size; ++other_heap_index)
 }

 void RegionClass::merge_regions(const bool& last_stage_flag, const double& threshold,
                                 RegionClass *merge_region, vector<RegionClass>& region_classes)
 {
    int band;
    if (params.debug > 2)
    {
      params.log_fs << endl << "Merging region " << merge_region->label << " into region " << this->label;
      params.log_fs << " with merge threshold = " << threshold << endl;
    }
#ifdef DEBUG
    if (params.debug > 3)
    {
      merge_region->print(region_classes);
      this->print(region_classes);
    }
#endif
    if (!active_flag)
    {
      if (params.debug > 0)
        params.log_fs << "Region label " << label << " is inactive:  can't participate in a merge!!" << endl;
      else
      {
        cout << "Region label " << label << " is inactive:  can't participate in a merge!!" << endl;
#ifdef PARALLEL
        cout << "Message from task ID " << params.myid << endl;
#endif
      }
    }
    if (!(merge_region->active_flag))
    {
      if (params.debug > 0)
        params.log_fs << "Region label " << merge_region->label << " is inactive:  can't participate in a merge!!" << endl;
      else
      {
        cout << "Region label " << merge_region->label << " is inactive:  can't participate in a merge!!" << endl;
#ifdef PARALLEL
        cout << "Message from task ID " << params.myid << endl;
#endif
      }
    }

   // Update merge_threshold, initial_merge_flag and merged_flag
    merge_threshold = threshold;
    if (params.initial_merge_flag)
    {
      initial_merge_flag = (initial_merge_flag) || (merge_region->initial_merge_flag);
      seam_flag = (seam_flag) || (merge_region->seam_flag);
    }
    merged_flag = true;

// Update merged region npix, sum values, and, if necessary, update
// the sumsq values and sum_pixel_std_dev values.
    npix += merge_region->npix;
    for (band = 0; band < nbands; band++)
      sum[band] += merge_region->sum[band];
    if (sumsq_flag)
    {
      for (band = 0; band < nbands; band++)
        sumsq[band] += merge_region->sumsq[band];
    }
    if (sumxlogx_flag)
    {
      for (band = 0; band < nbands; band++)
        sumxlogx[band] += merge_region->sumxlogx[band];
    }
    if (std_dev_flag)
    {
      for (band = 0; band < nbands; band++)
        sum_pixel_std_dev[band] += merge_region->sum_pixel_std_dev[band];
    }
    if ((params.edge_image_flag) && (max_edge_value < merge_region->max_edge_value))
      max_edge_value = merge_region->max_edge_value;

// Merge neighbor label sets
    if (!merge_region->nghbrs_label_set.empty())
    {
      set<unsigned int>::const_iterator nghbrs_label_set_iter = merge_region->nghbrs_label_set.begin();
      while (nghbrs_label_set_iter != merge_region->nghbrs_label_set.end())
      {
        nghbrs_label_set.insert(*nghbrs_label_set_iter);
        ++nghbrs_label_set_iter;
      }
    }

// Delete this region and merge_region from nghbrs_label_set
    nghbrs_label_set.erase(label);
    nghbrs_label_set.erase(merge_region->label);

// Flag merged out region (merge_region) as inactive and set merge_region_label and merged_flag.
    merge_region->active_flag = false;
    merge_region->merge_region_label = label;
    merge_region->merged_flag = true;

// Replace merge_region->label in the nghbrs_label_set and best_nghbr_label
// of this region's neighbors with this region's label.
    bool found_flag;
    RegionClass *nghbr_region_ptr;
    set<unsigned int>::iterator other_nghbrs_label_set_iter;
    set<unsigned int>::iterator nghbrs_label_set_iter = nghbrs_label_set.begin();
    while (nghbrs_label_set_iter != nghbrs_label_set.end())
    {
      nghbr_region_ptr = &region_classes[(*nghbrs_label_set_iter)-1];
      found_flag = false;
      other_nghbrs_label_set_iter = nghbr_region_ptr->nghbrs_label_set.begin();
      while ((!found_flag) && (other_nghbrs_label_set_iter != nghbr_region_ptr->nghbrs_label_set.end()))
      {
        if ((*other_nghbrs_label_set_iter) == merge_region->label)
          found_flag = true;
        ++other_nghbrs_label_set_iter;
      }
      if (found_flag)
      {
        nghbr_region_ptr->nghbrs_label_set.erase(merge_region->label);
        nghbr_region_ptr->nghbrs_label_set.insert(label);
#ifdef DEBUG
        if (params.debug > 3)
        {
          params.log_fs << "Region label " << merge_region->label;
          params.log_fs << " replaced in nghbrs_label_set of region label " << nghbr_region_ptr->label;
          params.log_fs << " with region label " << label << endl;
        }
#endif
        found_flag = false;
        other_nghbrs_label_set_iter = nghbr_region_ptr->best_nghbr_label.begin();
        while ((!found_flag) && (other_nghbrs_label_set_iter != nghbr_region_ptr->best_nghbr_label.end()))
        {
          if ((*other_nghbrs_label_set_iter) == merge_region->label)
            found_flag = true;
          ++other_nghbrs_label_set_iter;
        }
        if (found_flag)
        {
          nghbr_region_ptr->best_nghbr_label.erase(merge_region->label);
          nghbr_region_ptr->best_nghbr_label.insert(label);
#ifdef DEBUG
          if (params.debug > 3)
          {
            params.log_fs << "Region label " << merge_region->label;
            params.log_fs << " replaced in best_nghbr_label of region label " << nghbr_region_ptr->label;
            params.log_fs << " with region label " << label << endl;
          }
#endif
        }
      }
      ++nghbrs_label_set_iter;
    }
#ifdef DEBUG
    if (params.debug > 3)
    {
      params.log_fs << "After merging: " << endl;
      merge_region->print(region_classes);
      this->print(region_classes);
    }
#endif
    return;
 }

 void RegionClass::merge_seam_regions(RegionClass *seam_merge_region, vector<RegionClass>& seam_region_classes)
 {
    if (params.debug > 2)
    {
      params.log_fs << endl << "Merging seam region " << seam_merge_region->label << " into seam region " << this->label << endl;
    }
#ifdef DEBUG
    if (params.debug > 3)
    {
      params.log_fs << "Before merging: " << endl;
      seam_merge_region->seam_print(seam_region_classes);
      this->seam_print(seam_region_classes);
    }
#endif
    if (!active_flag)
    {
      if (params.debug > 0)
        params.log_fs << "Region label " << label << " is inactive:  can't participate in a merge!!" << endl;
      else
      {
        cout << "Region label " << label << " is inactive:  can't participate in a merge!!" << endl;
#ifdef PARALLEL
        cout << "Message from task ID " << params.myid << endl;
#endif
      }
    }
    if (!(seam_merge_region->active_flag))
    {
      if (params.debug > 0)
        params.log_fs << "Region label " << seam_merge_region->label << " is inactive:  can't participate in a merge!!" << endl;
      else
      {
        cout << "Region label " << seam_merge_region->label << " is inactive:  can't participate in a merge!!" << endl;
#ifdef PARALLEL
        cout << "Message from task ID " << params.myid << endl;
#endif
      }
    }
    seam_merge_region->active_flag = false;

// Merge nghbrs_label_seam_map's.
    unsigned int seam_label;
    RegionSeam seam_region_seam;
    map<unsigned int, RegionSeam>::iterator nghbrs_label_seam_map_iter;
    map<unsigned int, RegionSeam>::const_iterator merge_nghbrs_label_seam_map_iter;
    merge_nghbrs_label_seam_map_iter = seam_merge_region->nghbrs_label_seam_map.begin();
    while (merge_nghbrs_label_seam_map_iter != seam_merge_region->nghbrs_label_seam_map.end())
    {
      seam_label = (*merge_nghbrs_label_seam_map_iter).first;
      seam_region_seam = (*merge_nghbrs_label_seam_map_iter).second;
      nghbrs_label_seam_map_iter = nghbrs_label_seam_map.find(seam_label);
      if (nghbrs_label_seam_map_iter == nghbrs_label_seam_map.end())
        nghbrs_label_seam_map.insert(make_pair(seam_label,seam_region_seam));
      else
        (*nghbrs_label_seam_map_iter).second += seam_region_seam;
      ++merge_nghbrs_label_seam_map_iter;
    }

// Delete this region and seam_merge_region from nghbrs_label_seam_map's.
    nghbrs_label_seam_map.erase(label);
    nghbrs_label_seam_map.erase(seam_merge_region->label);

// Replace seam_merge_region->label in the nghbrs_label_seam_map 
// of this region's neighbors with this region's label.
    unsigned int nghbr_label, nghbr_index;
    map<unsigned int, RegionSeam>::iterator nghbr_nghbrs_label_seam_map_iter, other_nghbrs_label_seam_map_iter;
    nghbrs_label_seam_map_iter = nghbrs_label_seam_map.begin();
    while (nghbrs_label_seam_map_iter != nghbrs_label_seam_map.end())
    {
      nghbr_label = (*nghbrs_label_seam_map_iter).first;
      nghbr_index = nghbr_label - 1;
      nghbr_nghbrs_label_seam_map_iter = seam_region_classes[nghbr_index].nghbrs_label_seam_map.find(seam_merge_region->label);
      if (nghbr_nghbrs_label_seam_map_iter != seam_region_classes[nghbr_index].nghbrs_label_seam_map.end())
      {
        seam_region_seam = (*nghbr_nghbrs_label_seam_map_iter).second;
        other_nghbrs_label_seam_map_iter = seam_region_classes[nghbr_index].nghbrs_label_seam_map.find(label);
        if (other_nghbrs_label_seam_map_iter == seam_region_classes[nghbr_index].nghbrs_label_seam_map.end())
          seam_region_classes[nghbr_index].nghbrs_label_seam_map.insert(make_pair(label,seam_region_seam));
        else
          (*other_nghbrs_label_seam_map_iter).second += seam_region_seam;
        seam_region_classes[nghbr_index].nghbrs_label_seam_map.erase(seam_merge_region->label);
      }
      ++nghbrs_label_seam_map_iter;
    }
#ifdef DEBUG
    if (params.debug > 3)
    {
      params.log_fs << "After merging: " << endl;
      this->seam_print(seam_region_classes);
    }
#endif
    return;
 }

 void RegionClass::update_best_nghbr(RegionClass *nghbr_region_ptr)
 {
   bool edge_process_flag;
   double result, edge_result, edge_factor;

   result = calc_region_dissim(this,nghbr_region_ptr,params.merge_accel_flag);

   edge_process_flag = (params.edge_wght > 0.0);
   if ((params.initial_merge_flag) && (!seam_flag) && (!nghbr_region_ptr->seam_flag))
   {
     if ((initial_merge_flag) && (nghbr_region_ptr->initial_merge_flag))
     {
       result /= params.spclust_wght;
       edge_process_flag = false;
     }
   }
   if (edge_process_flag)
   {
     edge_result = max_edge_value;
     if (nghbr_region_ptr->max_edge_value > edge_result)
       edge_result = nghbr_region_ptr->max_edge_value;
     if (edge_result < 0.0)
     {
       if (params.edge_dissim_option == 1)
         edge_result = params.max_edge_value;
       else
         edge_result = params.min_edge_value;
     }
     edge_factor = (edge_result - params.min_edge_value)/(params.max_edge_value - params.min_edge_value);
     edge_factor = pow((double) edge_factor, (double) params.edge_power);
     edge_factor = (1.0 - params.edge_wght) + edge_factor*params.edge_wght;
     if (params.edge_dissim_option == 2)
       edge_factor = (params.spclust_wght + (1.0 - params.spclust_wght)*edge_factor)/params.spclust_wght;
     result *= edge_factor;
   }
   float float_result = (float) result;

   if (float_result < best_nghbr_dissim)
   {
     best_nghbr_label.clear();
     best_nghbr_label.insert(nghbr_region_ptr->label);
     best_nghbr_dissim = float_result;
   }
   else if (float_result == best_nghbr_dissim)
   {
     best_nghbr_label.insert(nghbr_region_ptr->label);
   }

   return;
 }

 void RegionClass::update_best_region(RegionClass *other_region_ptr)
 {
   float result;

   result = (float) calc_region_dissim(this,other_region_ptr,false);

#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << "Region dissimilarity between regions with labels " << this->label;
     params.log_fs << " and " << other_region_ptr->label << " is " << result << endl;
   }
#endif
   if (result < best_region_dissim)
   {
     best_region_label.clear();
     best_region_label.insert(other_region_ptr->label);
     best_region_dissim = result;
   }
   else if (result == best_region_dissim)
   {
     best_region_label.insert(other_region_ptr->label);
   }

   return;
 }

 void RegionClass::update_nghbr_heap(vector<RegionClass *>& nghbr_heap,
                                     unsigned int& nghbr_heap_size)
 {
#ifdef DEBUG
   unsigned int heap_index;
   if (params.debug > 3)
     params.log_fs << "Calling update_nghbr_heap for region " << label << endl;
   if (params.debug > 3)
   {
     params.log_fs << endl << "Before updating heap, nghbr_heap:  " << endl;
     for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
       nghbr_heap[heap_index]->print();
   }
#endif
   bool change_flag = true;
   nghbr_heap_npix = npix;
   while (change_flag)
   {
     change_flag = false;
     if (nghbr_downheap(nghbr_heap_index,nghbr_heap,nghbr_heap_size))
       change_flag = true;
     if (nghbr_upheap(nghbr_heap_index,nghbr_heap,nghbr_heap_size))
       change_flag = true;
   }
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After updating heap, nghbr_heap:  " << endl;
     for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
       nghbr_heap[heap_index]->print();
   }
#endif
   return;
 }

 void RegionClass::nghbr_bring_to_top_heap(vector<RegionClass *>& nghbr_heap,
                                           const unsigned int& nghbr_heap_size)
 {
    unsigned int heap_index = nghbr_heap_index;
    RegionClass *region_ptr;

    region_ptr = nghbr_heap[heap_index];
    while (heap_index != 0)
    {
      nghbr_heap[heap_index] = nghbr_heap[heap_div2(heap_index)];
      nghbr_heap[heap_index]->nghbr_heap_index = heap_index;
      heap_index = heap_div2(heap_index);
    }
    nghbr_heap[heap_index] = region_ptr;
    nghbr_heap_index = heap_index;

    return;
 }

 void RegionClass::update_region_heap(vector<RegionClass *>& region_heap,
                                      unsigned int& region_heap_size)
 {
#ifdef DEBUG
   unsigned int heap_index;
   if (params.debug > 3)
     params.log_fs << "Calling update_region_heap for region " << label << endl;
   if (params.debug > 3)
   {
     params.log_fs << endl << "Before updating heap, region_heap:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }
#endif
   bool change_flag = true;
   region_heap_npix = npix;
   while (change_flag)
   {
     change_flag = false;
     if (region_downheap(region_heap_index,region_heap,region_heap_size))
       change_flag = true;
     if (region_upheap(region_heap_index,region_heap,region_heap_size))
       change_flag = true;
   }
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After updating heap, region_heap:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }
#endif
 }

 void RegionClass::region_bring_to_top_heap(vector<RegionClass *>& region_heap,
                                            const unsigned int& region_heap_size)
 {
    unsigned int heap_index = region_heap_index;
    RegionClass *region_ptr;

    region_ptr = region_heap[heap_index];
    while (heap_index != 0)
    {
      region_heap[heap_index] = region_heap[heap_div2(heap_index)];
      region_heap[heap_index]->region_heap_index = heap_index;
      heap_index = heap_div2(heap_index);
    }
    region_heap[heap_index] = region_ptr;
    region_heap_index = heap_index;

    return;
 }

 void RegionClass::nghbrs_label_set_renumber(map<unsigned int,unsigned int>& region_relabel_pairs)
 {
   unsigned int region_label;
   set<unsigned int> copy_of_label_set;

   if (!nghbrs_label_set.empty())
   {
     set<unsigned int>::const_iterator nghbrs_label_set_iter = nghbrs_label_set.begin();
     while (nghbrs_label_set_iter != nghbrs_label_set.end())
     {
       region_label = *nghbrs_label_set_iter;
       copy_of_label_set.insert(region_label);
       ++nghbrs_label_set_iter;
     }
     nghbrs_label_set.clear();
     nghbrs_label_set_iter = copy_of_label_set.begin();
     map<unsigned int,unsigned int>::iterator region_relabel_pair_iter;
     while (nghbrs_label_set_iter != copy_of_label_set.end())
     {
       region_label = *nghbrs_label_set_iter;
       region_relabel_pair_iter = region_relabel_pairs.find(region_label);
       if (region_relabel_pair_iter != region_relabel_pairs.end())
         region_label = (*region_relabel_pair_iter).second;
       if (region_label > 0)
         nghbrs_label_set.insert(region_label);
       ++nghbrs_label_set_iter;
     }
   }

   return;
 }

 void RegionClass::update_region_class_info(const bool& boundary_flag,
                                            const unsigned int& region_object_label)
 {
  if (boundary_flag)
    boundary_npix += 1;
  if (params.region_nb_objects_flag)
    region_objects_set.insert(region_object_label);

  return;
 }

 void RegionClass::calc_sum_pixel_gdissim()  // Shouldn't need to add code for the new std_dev_wght definition...
 {
   int band;
   double reg_mean, accum;
   double reg_npix = (double) npix;

   if (!active_flag)
   {
     if (params.debug > 0)
       params.log_fs << "Region label" << label << " is inactive:  can't use to update pixel_gdissim!!" << endl;
     else
     {
       cout << "Region label" << label << " is inactive:  can't use to update pixel_gdissim!!" << endl;
#ifdef PARALLEL
       cout << "Message from taskid = " << params.myid << endl;
#endif
     }
   }
   accum = 0.0;
   sum_pixel_gdissim = 0.0;
   for (band=0; band < nbands; band++)
   {
     reg_mean = sum[band]/reg_npix;
     if (params.dissim_crit == 6)
     {
       accum += sumsq[band] - sum[band]*reg_mean;
     }
     else if (params.dissim_crit == 7)
     {
       accum = sumsq[band] - sum[band]*reg_mean;
       if (accum > sum_pixel_gdissim)
         sum_pixel_gdissim = accum;
     }
     else if (params.dissim_crit == 9)
     {
       accum += (sumxlogx[band] - sum[band]*log(reg_mean))/meanval[band];
     }
   }
   if (params.dissim_crit != 7)
     sum_pixel_gdissim = accum;
   if (sum_pixel_gdissim < 0.0)
     sum_pixel_gdissim = 0.0;

   return;
 }

 bool RegionClass::is_nghbr(unsigned int nghbr_label)
 {
   if (nghbrs_label_set.find(nghbr_label) == nghbrs_label_set.end())
     return false;
   else
     return true;
 }

 bool RegionClass::is_best_nghbr(unsigned int label)
 {
   if (best_nghbr_label.empty())
     return true;   

   if (best_nghbr_label.find(label) == best_nghbr_label.end())
     return false;
   else
     return true;
 }

 bool RegionClass::is_best_region(unsigned int label)
 {
   if (best_region_label.empty())
     return true;   

   if (best_region_label.find(label) == best_region_label.end())
     return false;
   else
     return true;
 }

 unsigned int RegionClass::get_best_nghbr_label(vector<RegionClass>& region_classes)
 {
   unsigned int nghbr_index, nghbr_label = UINT_MAX, nghbr_npix = 0;

   set<unsigned int>::iterator nghbr_label_iter = best_nghbr_label.begin();
   while (nghbr_label_iter != best_nghbr_label.end())
   {
     nghbr_index = (*nghbr_label_iter) - 1;
     if ((region_classes[nghbr_index].npix > nghbr_npix) ||
         ((region_classes[nghbr_index].label < nghbr_label) &&
          (region_classes[nghbr_index].npix == nghbr_npix)))
     {
       nghbr_label = region_classes[nghbr_index].label;
       nghbr_npix = region_classes[nghbr_index].npix;
     }
     ++nghbr_label_iter;
   }
   if (nghbr_label < UINT_MAX)
     return nghbr_label;
   else
     return 0;

 }

 unsigned int RegionClass::get_best_region_label(vector<RegionClass>& region_classes)
 {
   unsigned int region_index, region_label = UINT_MAX, region_npix = 0;

   set<unsigned int>::iterator region_label_iter = best_region_label.begin();
   while (region_label_iter != best_region_label.end())
   {
     region_index = (*region_label_iter) - 1;
     if ((region_classes[region_index].npix > region_npix) ||
         ((region_classes[region_index].label < region_label) &&
          (region_classes[region_index].npix == region_npix)))
     {
       region_label = region_classes[region_index].label;
       region_npix = region_classes[region_index].npix;
     }
     ++region_label_iter;
   }
   if (region_label < UINT_MAX)
     return region_label;
   else
     return 0;

 }
#endif // RHSEG_RUN

// Returns "true" is the nghbrs_label_set is OK, "false" if not OK.
 bool RegionClass::check_nghbrs_label_set(vector<RegionClass>& region_classes)
 {
  unsigned int nghbr_region_label, nghbr_region_index, merge_region_index;
  set<unsigned int>::iterator region_label_set_iter;
  bool set_OK_flag = true;

  if (active_flag)
  {
    if (!nghbrs_label_set.empty())
    {
      region_label_set_iter = nghbrs_label_set.begin();
      while (region_label_set_iter != nghbrs_label_set.end())
      {
        nghbr_region_label = *region_label_set_iter;
        nghbr_region_index = nghbr_region_label -1;
        if (region_classes[nghbr_region_index].active_flag)
        {
          if (region_classes[nghbr_region_index].nghbrs_label_set.find(label) ==
              region_classes[nghbr_region_index].nghbrs_label_set.end())
          {
            set_OK_flag = false;
//            region_classes[nghbr_region_index].nghbrs_label_set.insert(label);
            if (params.debug > 0)
            {
              params.log_fs << endl << "WARNING: In checking nghbrs_label_set for region " << label << "," << endl;
              params.log_fs << "found nonsymmetric nghbrs_label_set!" << endl;
              params.log_fs << "(Region label " << label << " not found in nghbrs_label_set for region ";
              params.log_fs << nghbr_region_label << ")" << endl;
            }
            else
            {
              cout << endl << "WARNING: In checking nghbrs_label_set for region " << label << "," << endl;
              cout << "found nonsymmetric nghbrs_label_set!" << endl;
              cout << "(Region label " << label << " not found in nghbrs_label_set for region ";
              cout << nghbr_region_label << ")" << endl;
#ifdef PARALLEL
              cout << "Message from taskid = " << params.myid << endl;
#endif
            }
          }
        }
        else
        {
          set_OK_flag = false;
          if (params.debug > 0)
          {
            params.log_fs << endl << "WARNING: In checking nghbrs_label_set for region " << label << "," << endl;
            params.log_fs << "found inactive neighboring region with label " << nghbr_region_label << endl;
          }
          else
          {
            cout << endl << "WARNING: In checking nghbrs_label_set for region " << label << "," << endl;
            cout << "found inactive neighboring region with label " << nghbr_region_label << endl;
#ifdef PARALLEL
            cout << "Message from taskid = " << params.myid << endl;
#endif
          }
          if (region_classes[nghbr_region_index].merge_region_label != 0)
          {
            merge_region_index = region_classes[nghbr_region_index].merge_region_label - 1;
            while ((!(region_classes[merge_region_index].get_active_flag())) &&
                   (region_classes[merge_region_index].merge_region_label != 0))
              merge_region_index = region_classes[merge_region_index].merge_region_label - 1;
            params.log_fs << "(Neighboring region with label " << nghbr_region_label << " was merged into the region labeled ";
            params.log_fs << region_classes[merge_region_index].label << ")" << endl;
          }
        }
        ++region_label_set_iter;
      }
    }
    else
      params.log_fs << "nghbrs_label_set is empty" << endl;
  }
  else
  {
    if (merge_region_label != 0)
    {
      merge_region_index = merge_region_label-1;
      while ((!(region_classes[merge_region_index].get_active_flag())) &&
             (region_classes[merge_region_index].merge_region_label != 0))
        merge_region_index = region_classes[merge_region_index].merge_region_label - 1;
      params.log_fs << endl << "Region label " << label << " was merged into region label ";
      params.log_fs << region_classes[merge_region_index].label << endl << endl;
    }
    else
      params.log_fs << endl << "Region label " << label << " is inactive " << endl;
  }
  return set_OK_flag;
 }

#ifdef RHSEG_RUN
 void RegionClass::update_sum_pixel_gdissim(Pixel *this_pixel)  // Shouldn't need to add code for new std_dev_wght definition
 {
   int band;
   double temp_value, reg1_mean, reg2_mean;
   double pixel_gdissim, sqdiff, sumsqdiff, norm1, norm2, scalar_prod, entropy;
   double reg1_npix = (double) npix;

   if (!active_flag)
   {
     if (params.debug > 0)
       params.log_fs << "Region label" << label << " is inactive:  can't use to update pixel_gdissim!!" << endl;
     else
     {
       cout << "Region label" << label << " is inactive:  can't use to update pixel_gdissim!!" << endl;
#ifdef PARALLEL
       cout << "Message from taskid = " << params.myid << endl;
#endif
     }
   }
   if (npix == 1)
   {
     pixel_gdissim = 0.0;
   }
   else
   {
     sumsqdiff = 0.0; norm1 = 0.0; norm2 = 0.0; scalar_prod = 0.0, entropy = 0.0;
     for (band=0; band < nbands; band++)
     {
       reg1_mean = sum[band]/reg1_npix;
       temp_value = (double) this_pixel->get_input_data(band);
       temp_value = scale[band]*(temp_value - offset[band]);
       reg2_mean = temp_value;
       if ((params.dissim_crit == 4) || (params.dissim_crit == 8))
       {
         norm1 += reg1_mean*reg1_mean;
         norm2 += reg2_mean*reg2_mean;
         scalar_prod += reg1_mean*reg2_mean;
       }
       else if (params.dissim_crit == 5)
       {
         norm1 += reg1_mean;
         norm2 += reg2_mean;
       }
       else if (params.dissim_crit == 9)
       {
         entropy += (sum[band]*log(reg1_mean) - temp_value*log(temp_value));
         entropy /= meanval[band];
       }
       else
       {
         sqdiff = reg1_mean - reg2_mean;
         if ((params.dissim_crit == 2) || (params.dissim_crit == 6) || (params.dissim_crit == 7))
           sqdiff = sqdiff*sqdiff;
         else if (sqdiff < 0.0)
           sqdiff = -sqdiff;
         if (params.dissim_crit == 10)
         {
           sqdiff /= (reg1_npix*reg1_mean + reg2_mean);
         }
         if ((params.dissim_crit == 3) || (params.dissim_crit == 7))
         {
           if (sqdiff > sumsqdiff)
             sumsqdiff = sqdiff;
         }
         else
           sumsqdiff += sqdiff;
       }
     }
     if ((params.dissim_crit == 4) || (params.dissim_crit == 8))
     {
       pixel_gdissim = scalar_prod/sqrt(norm1*norm2);
       if (params.dissim_crit == 4)
         pixel_gdissim = acos(pixel_gdissim);
       else
       {
         pixel_gdissim = (acos(0.0)-acos(pixel_gdissim))/acos(0.0);
         if (norm2 != 0.0)
           reg1_mean = norm1/norm2;
         else
           reg1_mean = FLT_MAX;
         if (norm1 != 0.0)
           reg2_mean = norm2/norm1;
         else
           reg2_mean = FLT_MAX;
         if (reg2_mean < reg1_mean)
           reg1_mean = reg2_mean;
         if ((norm1 == 0.0) && (norm2 == 0.0))
           reg1_mean = 1.0;
         pixel_gdissim = 1.0 - reg1_mean*pixel_gdissim;
       }
     }
     else if (params.dissim_crit == 5)
     {
       pixel_gdissim = 0.0;
       for (band=0; band < nbands; band++)
       {
         reg1_mean = sum[band]/reg1_npix;
         temp_value = (double) this_pixel->get_input_data(band);
         temp_value = scale[band]*(temp_value - offset[band]);
         reg2_mean = temp_value;
         reg1_mean /= norm1;
         reg2_mean /= norm2;
         if ((reg1_mean/reg2_mean) < 1.0)
           pixel_gdissim += reg2_mean*log(reg2_mean/reg1_mean) - reg1_mean*log(reg1_mean/reg2_mean);
         else
           pixel_gdissim += reg1_mean*log(reg1_mean/reg2_mean) - reg2_mean*log(reg2_mean/reg1_mean);
       }
     }
     else if (params.dissim_crit == 9)
     {
       pixel_gdissim = entropy;
       if (pixel_gdissim < 0.0)
         pixel_gdissim = 0.0;
     }
     else
     {
       if (params.dissim_crit == 2)
         sumsqdiff = sqrt(sumsqdiff);
       if (params.dissim_crit == 10)
         sumsqdiff *= sqrt((reg1_npix)*(reg1_npix+1.0));
       pixel_gdissim = sumsqdiff;
     }
  }

  sum_pixel_gdissim += pixel_gdissim;

  return;
 }
#ifdef PARALLEL
 void RegionClass::update_nghbrs_label_set(Temp& temp_data, unsigned int& int_buf_position)
 {
 // Updates the nghbrs_label_set based on information received in buffers from another parallel task.
    unsigned int nghbrs_label_size;
    unsigned int pixel_index;

    nghbrs_label_size = temp_data.int_buffer[int_buf_position++];
    for (pixel_index = 0; pixel_index < nghbrs_label_size; pixel_index++)
      nghbrs_label_set.insert(temp_data.int_buffer[int_buf_position++]);

    return;
 }

 void RegionClass::load_nghbrs_label_set(Temp& temp_data, unsigned int& int_buf_position)
 {
 // Loads nghbrs_label_set information into buffers for transmission to another parallel task.
   int nghbrs_label_size;

   nghbrs_label_size = nghbrs_label_set.size();
   temp_data.int_buffer[int_buf_position++] = (unsigned int) nghbrs_label_size;
   if (nghbrs_label_size > 0)
   {
     set<unsigned int>::const_iterator nghbrs_label_set_iter = nghbrs_label_set.begin();
     while (nghbrs_label_set_iter != nghbrs_label_set.end())
     {
       temp_data.int_buffer[int_buf_position++] = *nghbrs_label_set_iter;
       ++nghbrs_label_set_iter;
     }
   }

   return;
 }
#endif // PARALLEL

 void RegionClass::print(vector<RegionClass>& region_classes)
 {
  if (active_flag)
  {
    int band;
    set<unsigned int>::iterator region_label_set_iter;

    params.log_fs << endl << "For region label " << label << ", npix = " << npix << endl;
//    params.log_fs.precision(6);
    if (params.region_sum_flag)
    {
      params.log_fs << "Region sum values:" << endl;
      for (band = 0; band < nbands; band++)
        params.log_fs << sum[band] << "  ";
      params.log_fs << endl;
      if (params.region_sumsq_flag)
      {
        params.log_fs << "Region sum square values:" << endl;
        for (band = 0; band < nbands; band++)
          params.log_fs << sumsq[band] << "  ";
        params.log_fs << endl;
      }
      if (params.region_sumxlogx_flag)
      {
        params.log_fs << "Region sum xlogx values:" << endl;
        for (band = 0; band < nbands; band++)
          params.log_fs << sumxlogx[band] << "  ";
        params.log_fs << endl;
      }
      if (params.std_dev_image_flag)
      {
        params.log_fs << "Local region sum pixel standard deviation values:" << endl;
        for (band = 0; band < nbands; band++)
          params.log_fs << sum_pixel_std_dev[band] << "  ";
        params.log_fs << endl;
      }
    }
    if (std_dev_flag)
      params.log_fs << "Region band maximum standard deviation = " << this->get_band_max_std_dev() << endl;
    if (params.edge_image_flag)
      params.log_fs << "Region maximum edge value = " << this->max_edge_value << endl;
    if (params.region_threshold_flag)
      params.log_fs << "Threshold for last merge involving this region = " << merge_threshold << endl;
    if (!nghbrs_label_set.empty())
    {
      region_label_set_iter = nghbrs_label_set.begin();
      params.log_fs << "Neighbor list:" << endl;
      while (region_label_set_iter != nghbrs_label_set.end())
      {
        params.log_fs << *region_label_set_iter << "  ";
        ++region_label_set_iter;
      }
      params.log_fs << endl;
    }
    else
      params.log_fs << "nghbrs_label_set is empty" << endl;
    if (!best_nghbr_label.empty())
    {
      region_label_set_iter = best_nghbr_label.begin();
      params.log_fs << "Most similar neighboring regions are region labels ";
      while (region_label_set_iter != best_nghbr_label.end())
      {
        params.log_fs << *region_label_set_iter << "  ";
        ++region_label_set_iter;
      }
      params.log_fs << endl << "with dissimilarity criterion value = " << best_nghbr_dissim << endl;
    }
    else
      params.log_fs << "best_nghbr_label_set is empty" << endl;
    if (!best_region_label.empty())
    {
      region_label_set_iter = best_region_label.begin();
      params.log_fs << "Most similar regions (overall) are region labels ";
      while (region_label_set_iter != best_region_label.end())
      {
        params.log_fs << *region_label_set_iter << "  ";
        ++region_label_set_iter;
      }
      params.log_fs << endl << "    with dissimilarity criterion value = " << best_region_dissim << endl;
    }
    else
      params.log_fs << "best_region_label_set is empty" << endl;
/*
    params.log_fs << endl << "nghbr_heap_index = " << nghbr_heap_index << " and region_heap_index = " << region_heap_index << endl;
    params.log_fs << endl << "Sum of the pixel_gdissim values is " << sum_pixel_gdissim;
    params.log_fs << endl << "Distance to minimum vector region is " << min_region_dissim;
    if (params.region_nb_objects_flag)
      params.log_fs << "Region consists of " << region_objects_set.size() << " spatially connected subregions" << endl;
    if (!region_objects_set.empty())
    {
      region_label_set_iter = region_objects_set.begin();
      params.log_fs << "Connect regions label list:" << endl;
      while (region_label_set_iter != region_objects_set.end())
      {
        params.log_fs << *region_label_set_iter << "  ";
        ++region_label_set_iter;
      }
      params.log_fs << endl;
    }
*/
    params.log_fs << endl;
  }
  else
  {
    if (merge_region_label != 0)
    {
      unsigned int region_index = merge_region_label-1;
      while ((!(region_classes[region_index].get_active_flag())) && (region_classes[region_index].merge_region_label != 0))
        region_index = region_classes[region_index].merge_region_label - 1;
      params.log_fs << endl << "Region label " << label << " was merged into region label ";
      params.log_fs << region_classes[region_index].label << endl << endl;
    }
    else
      params.log_fs << endl << "Region label " << label << " is inactive " << endl;
  }
  return;
 }

 void RegionClass::seam_print(vector<RegionClass>& seam_region_classes)
 {
  if (active_flag)
  {
    map<unsigned int, RegionSeam>::iterator nghbrs_label_seam_map_iter;

    if (!nghbrs_label_seam_map.empty())
    {
      params.log_fs << endl << "For seam region label " << label << ", nghbrs_label_seam_map:" << endl;
      nghbrs_label_seam_map_iter = nghbrs_label_seam_map.begin();
      while (nghbrs_label_seam_map_iter != nghbrs_label_seam_map.end())
      {
        params.log_fs << (*nghbrs_label_seam_map_iter).first << ":  ";
        (*nghbrs_label_seam_map_iter).second.print();
        ++nghbrs_label_seam_map_iter;
      }
      params.log_fs << endl;
    }
    else
      params.log_fs << "nghbrs_label_seam_map is empty" << endl;
  }
/*
  else
  {
    params.log_fs << endl << "Seam region label " << label << " is inactive " << endl;
  }
*/
  return;
 }

 void RegionClass::print(vector<RegionClass>& region_classes, const unsigned int& offset)
 {
  if (active_flag)
  {
    int band;
    set<unsigned int>::iterator region_label_set_iter;

    params.log_fs << endl << "For region label " << label << ", npix = " << npix;
    if (std_dev_flag)
      params.log_fs << ", band_max_std_dev = " << this->get_band_max_std_dev() << endl;
    else
      params.log_fs << endl;
    params.log_fs << endl;
    params.log_fs.precision(6);
//    params.log_fs.precision(18);
    params.log_fs << "Region sum values:" << endl;
    for (band = 0; band < nbands; band++)
      params.log_fs << sum[band] << "  ";
    params.log_fs << endl;
    if (sumsq_flag)
    {
      params.log_fs << "Region sum square values:" << endl;
      for (band = 0; band < nbands; band++)
        params.log_fs << sumsq[band] << " ";
      params.log_fs << endl;
    }
    if (sumxlogx_flag)
    {
      params.log_fs << "Region sum xlogx values:" << endl;
      for (band = 0; band < nbands; band++)
        params.log_fs << sumxlogx[band] << " ";
      params.log_fs << endl;
    }
    if (std_dev_flag)
    {
      params.log_fs << "Local region sum pixel standard deviation values:" << endl;
      for (band = 0; band < nbands; band++)
        params.log_fs << sum_pixel_std_dev[band] << " ";
      params.log_fs << endl;
    }
//    params.log_fs.precision(6);
    if (!nghbrs_label_set.empty())
    {
      region_label_set_iter = nghbrs_label_set.begin();
      params.log_fs << "Neighbor list:" << endl;
      while (region_label_set_iter != nghbrs_label_set.end())
      {
        params.log_fs << region_classes[(*region_label_set_iter)-1-offset].label << "  ";
        ++region_label_set_iter;
      }
      params.log_fs << endl;
    }
    if (!best_nghbr_label.empty())
    {
      region_label_set_iter = best_nghbr_label.begin();
      params.log_fs << "Most similar neighboring regions are region labels ";
      while (region_label_set_iter != best_nghbr_label.end())
      {
        params.log_fs << (*region_label_set_iter) << "  ";
        ++region_label_set_iter;
      }
      params.log_fs << endl << "with dissimilarity criterion value = " << best_nghbr_dissim << endl;
    }
    if (!best_region_label.empty())
    {
      region_label_set_iter = best_region_label.begin();
      params.log_fs << "Most similar regions (overall) are region labels ";
      while (region_label_set_iter != best_region_label.end())
      {
        params.log_fs << *region_label_set_iter << "  ";
        ++region_label_set_iter;
      }
      params.log_fs << endl << "    with dissimilarity criterion value = " << best_region_dissim << endl;
    }
    params.log_fs << endl << "Sum of the pixel_gdissim values is " << sum_pixel_gdissim;
    params.log_fs << endl << "Distance to minimum vector region is " << min_region_dissim;
    if (region_objects_set.size() > 1)
      params.log_fs << endl << "Region consists of " << region_objects_set.size() << " spatially connected region_objects";
    if (boundary_npix > 0)
      params.log_fs << endl << "Region has " << boundary_npix << " boundary pixels";
    params.log_fs << endl << "Threshold for latest merge involving this region = " << merge_threshold;
    params.log_fs << endl << endl;
  }
  else
  {
    if (merge_region_label != 0)
    {
      unsigned int region_index = merge_region_label-1;
      while ((!(region_classes[region_index].get_active_flag())) && (region_classes[region_index].merge_region_label != 0))
        region_index = region_classes[region_index].merge_region_label - 1;
      params.log_fs << endl << "Region label " << label << " was merged into region label ";
      params.log_fs << region_classes[region_index].label << endl << endl;
    }
  }
  return;
 }

 void RegionClass::print()
 {
  if (active_flag)
  {
//    int band;
    params.log_fs << endl << "For region label " << label << ", npix = " << npix << " and nghbrs_label_set.size = ";
    params.log_fs << nghbrs_label_set.size() << endl;
/*
    if (params.region_sum_flag)
    {
      params.log_fs << "Region sum values:" << endl;
      for (band = 0; band < nbands; band++)
        params.log_fs << sum[band] << "  ";
      params.log_fs << endl;
      if (params.region_sumsq_flag)
      {
        params.log_fs << "Region sum square values:" << endl;
        for (band = 0; band < nbands; band++)
          params.log_fs << sumsq[band] << "  ";
        params.log_fs << endl;
      }
      if (params.region_sumxlogx_flag)
      {
        params.log_fs << "Region sum xlogx values:" << endl;
        for (band = 0; band < nbands; band++)
          params.log_fs << sumxlogx[band] << "  ";
        params.log_fs << endl;
      }
    }
    if (std_dev_flag)
      params.log_fs << ", band_max_std_dev = " << this->get_band_max_std_dev() << endl;
    else
      params.log_fs << endl;
    if (params.region_threshold_flag)
      params.log_fs << "Threshold for last merge involving this region = " << merge_threshold << endl;
    if (!best_nghbr_label.empty())
    {
      region_label_set_iter = best_nghbr_label.begin();
      params.log_fs << "Most similar neighboring regions are region labels ";
      while (region_label_set_iter != best_nghbr_label.end())
      {
        params.log_fs << (*region_label_set_iter) << "  ";
        ++region_label_set_iter;
      }
      params.log_fs << endl << "with dissimilarity criterion value = " << best_nghbr_dissim << endl;
    }
    else
      params.log_fs << "best_nghbr_label_set is empty" << endl;
    if (!best_region_label.empty())
    {
      region_label_set_iter = best_region_label.begin();
      params.log_fs << "Most similar regions (overall) are region labels ";
      while (region_label_set_iter != best_region_label.end())
      {
        params.log_fs << *region_label_set_iter << "  ";
        ++region_label_set_iter;
      }
      params.log_fs << endl << "    with dissimilarity criterion value = " << best_region_dissim << endl;
    }
    else
      params.log_fs << "best_region_label_set is empty" << endl;
    params.log_fs << "nghbr_heap_index = " << nghbr_heap_index << " and region_heap_index = " << region_heap_index << endl;
    params.log_fs << endl << "Threshold for latest merge involving this region = " << merge_threshold << endl;
    params.log_fs.precision(6);
    params.log_fs << endl << "Sum of the pixel_gdissim values is " << sum_pixel_gdissim;
    params.log_fs << endl << "Distance to minimum vector region is " << min_region_dissim;
    if (region_objects_set.size() > 1)
      params.log_fs << endl << "Region consists of " << region_objects_set.size() << " spatially connected region_objects";
    if (boundary_npix > 0)
      params.log_fs << endl << "Region has " << boundary_npix << " boundary pixels";
    params.log_fs << endl << "Threshold for latest merge involving this region = " << merge_threshold;
    params.log_fs << endl;
*/
  }
  else
  {
    params.log_fs << "Region label " << label << " was merged into region label " << merge_region_label;
/*
    params.log_fs << "(best_nghbr_dissim = " << best_nghbr_dissim;
    if (!best_region_label.empty())
      params.log_fs << " and best_region_dissim = " << best_region_dissim;
    params.log_fs << ")" << endl;
*/
    params.log_fs << endl;
  }
  return;
 }
#endif // RHSEG_RUN

 void RegionClass::print(vector<RegionClass>& region_classes, vector<RegionObject>& region_objects)
 {
  if (active_flag)
  {
    int band;
    set<unsigned int>::iterator region_label_set_iter;

    params.log_fs << endl << "For region label " << label << ", npix = " << npix << endl;
//    params.log_fs.precision(6);
    if (params.region_sum_flag)
    {
      params.log_fs << "Region sum values:" << endl;
      for (band = 0; band < nbands; band++)
        params.log_fs << sum[band] << "  ";
      params.log_fs << endl;
      if (params.region_sumsq_flag)
      {
        params.log_fs << "Region sum square values:" << endl;
        for (band = 0; band < nbands; band++)
          params.log_fs << sumsq[band] << "  ";
        params.log_fs << endl;
      }
      if (params.region_sumxlogx_flag)
      {
        params.log_fs << "Region sum xlogx values:" << endl;
        for (band = 0; band < nbands; band++)
          params.log_fs << sumxlogx[band] << "  ";
        params.log_fs << endl;
      }
      if (params.std_dev_image_flag)
      {
        params.log_fs << "Region sum pixel standard deviation values:" << endl;
        for (band = 0; band < nbands; band++)
          params.log_fs << sum_pixel_std_dev[band] << "  ";
        params.log_fs << endl;
      }
    }
    if (params.region_std_dev_flag)
      params.log_fs << "Region band maximum standard deviation = " << this->get_band_max_std_dev() << endl;
    if (params.region_threshold_flag)
      params.log_fs << "Threshold for last merge involving this region = " << merge_threshold << endl;
    if (params.region_boundary_npix_flag)
      params.log_fs << "Region has " << boundary_npix << " boundary pixels" << endl;
    if (!nghbrs_label_set.empty())
    {
      region_label_set_iter = nghbrs_label_set.begin();
      params.log_fs << "Neighbor list:" << endl;
      while (region_label_set_iter != nghbrs_label_set.end())
      {
        params.log_fs << *region_label_set_iter << "  ";
        ++region_label_set_iter;
      }
      params.log_fs << endl;
    }
    else
      params.log_fs << "nghbrs_label_set is empty" << endl;
    if (params.region_nb_objects_flag)
      params.log_fs << "Region contains " << nb_region_objects << " region objects" << endl;
    if (!region_objects_set.empty())
    {
      region_label_set_iter = region_objects_set.begin();
      params.log_fs << "Region object label list:" << endl;
      while (region_label_set_iter != region_objects_set.end())
      {
        params.log_fs << *region_label_set_iter << "  ";
        ++region_label_set_iter;
      }
      params.log_fs << endl;
      unsigned int max_npix = 0, npix_threshold = 0;
      unsigned int region_object_label;
      region_label_set_iter = region_objects_set.begin();
      while (region_label_set_iter != region_objects_set.end())
      { 
        region_object_label = (*region_label_set_iter);
        if (region_objects[region_object_label-1].get_npix() > max_npix)
        {
          npix_threshold = max_npix;
          max_npix = region_objects[region_object_label-1].get_npix();
        }
        else if (region_objects[region_object_label-1].get_npix() > npix_threshold)
        {
          npix_threshold = region_objects[region_object_label-1].get_npix();
        }
        ++region_label_set_iter;
      }
      if (npix_threshold > (25*(npix/region_objects_set.size())))
        npix_threshold = 25*(npix/region_objects_set.size());
      region_label_set_iter = region_objects_set.begin();
      params.log_fs << endl << "Region objects with at least " << npix_threshold << " pixels:" << endl << endl;
      while (region_label_set_iter != region_objects_set.end())
      { 
        region_object_label = (*region_label_set_iter);
        if (region_objects[region_object_label-1].get_npix() >= npix_threshold)
          region_objects[region_object_label-1].print(region_objects);
        ++region_label_set_iter;
      }
      params.log_fs << endl;
    }
    params.log_fs << endl;
  }
  else
  {
    if (merge_region_label != 0)
    {
      unsigned int region_index = merge_region_label-1;
      while ((!(region_classes[region_index].get_active_flag())) && (region_classes[region_index].merge_region_label != 0))
        region_index = region_classes[region_index].merge_region_label - 1;
      params.log_fs << endl << "Region label " << label << " was merged into region label ";
      params.log_fs << region_classes[region_index].label << endl << endl;
    }
    else
      params.log_fs << endl << "Region label " << label << " is inactive " << endl;
  }
  return;
 }

 void RegionClass::print(string& strMessage)
 {
  if (active_flag)
  {
    int band, max_print_bands = 10;
    float bp_ratio=0.0;

    if (nbands < max_print_bands)
      max_print_bands = nbands;

    if (params.region_boundary_npix_flag)
      bp_ratio = ((float) boundary_npix)/((float) npix);

    strMessage = "Feature Values for Region Class " + stringify_int(label) + ":\n\n";
    strMessage += "npix = " + stringify_int(npix) + "\n";
    params.log_fs << "For Region Class " << label << ": npix = " << npix;
    if (params.region_std_dev_flag)
    {
      strMessage += "band_max_std_dev = " + stringify_float((float) this->get_band_max_std_dev()) + "\n";
      params.log_fs << ", band_max_std_dev = " << this->get_band_max_std_dev();
    }
    if (params.region_boundary_npix_flag)
    {
      strMessage += "boundary pixel ratio = " + stringify_float(bp_ratio) + "\n";
      params.log_fs << ", and boundary pixel ratio = " << bp_ratio;
    }
    params.log_fs << endl;
    if (params.region_sum_flag)
    {
      strMessage += "mean values (unnormalized):\n";
      for (band = 0; band < max_print_bands; band++)
        strMessage += stringify_float(((sum[band]/npix)/scale[band])+offset[band]) + "  ";
      strMessage += "\n";
      strMessage += "sum values:\n";
      for (band = 0; band < max_print_bands; band++)
        strMessage += stringify_float(sum[band]) + "  ";
      strMessage += "\n";
      if (params.region_sumsq_flag)
      {
        strMessage += "sum square values:\n";
        for (band = 0; band < max_print_bands; band++)
          strMessage += stringify_float(sumsq[band]) + "  ";
        strMessage += "\n";
      }
      if (params.region_sumxlogx_flag)
      {
        strMessage += "sum xlogx values:\n";
        for (band = 0; band < max_print_bands; band++)
          strMessage += stringify_float(sumxlogx[band]) + "  ";
        strMessage += "\n";
      }
      if (params.std_dev_image_flag)
      {
        strMessage += "sum pixel standard deviation values:\n";
        for (band = 0; band < max_print_bands; band++)
          strMessage += stringify_float(sum_pixel_std_dev[band]) + "  ";
        strMessage += "\n";
      }
    }
    if (params.region_threshold_flag)
      strMessage += "Threshold for last merge involving this region class = " + stringify_float(merge_threshold) + "\n";
    if (params.region_boundary_npix_flag)
      strMessage += "Region class has " + stringify_int(boundary_npix) + " boundary pixels\n";
    if (!nghbrs_label_set.empty())
      strMessage += "Region class has " + stringify_int(nghbrs_label_set.size()) + " class neighbors\n";
    else
      strMessage += "Region class neighbor list is empty\n";
    if (params.region_nb_objects_flag)
      strMessage += "Region class contains " + stringify_int(nb_region_objects) + " region objects\n";
    strMessage += "\n";
  }
  else
  {
    if (merge_region_label != 0)
    {
      strMessage += "Region class " + stringify_int(label) + " was merged into region class ";
      strMessage += stringify_int(merge_region_label) + "\n";
    }
    else
      strMessage += "Region class " + stringify_int(label) + " is inactive\n";
  }
  return;
 }

 double RegionClass::get_unscaled_mean(int band) const
 {
   return (((sum[band]/npix)/oparams.scale[band])+oparams.offset[band]);
 }

 double RegionClass::get_unscaled_std_dev(int band) const
 {
   return (get_std_dev(band)/oparams.scale[band]);
 }

 double RegionClass::get_std_dev(int band) const
 {
   double stdDev = sum_pixel_std_dev[band];

   if (npix <= 1)
#ifdef MEAN_NORM_STD_DEV
     return (stdDev/sum[band]);
#else
     return stdDev;
#endif

   double numpix = (double) npix;

   stdDev = (sumsq[band] - ((sum[band]*sum[band])/numpix))/(numpix-1.0);
   if (stdDev > 0.0)
     stdDev = sqrt(stdDev);
   else
     stdDev = 0.0;

   if (npix < 9)
   {
     double factor = (9.0 - numpix)/9.0;
     stdDev = factor*sum_pixel_std_dev[band]/numpix + (1.0 - factor)*stdDev;
   }

#ifdef MEAN_NORM_STD_DEV
   stdDev = numpix*stdDev/sum[band];
#endif

   return stdDev;
 }

 double RegionClass::get_band_max_std_dev() const
 {
   int band;
   double maxStdDev = 0.0;

   for (band = 0; band < nbands; band++)
   {
     if (get_std_dev(band) > maxStdDev)
       maxStdDev = get_std_dev(band);
   }

   return maxStdDev;
 }

 RegionClass operator -(const RegionClass& arg1, const RegionClass& arg2)
 {
  int band;
  RegionClass result;

// Update private data - label remains unchanged
  result.npix = arg1.npix - arg2.npix;
#ifdef RHSEG_RUN
  result.nghbr_heap_npix = result.npix;
  result.region_heap_npix = result.npix;
#endif
  for (band = 0; band < result.nbands; band++)
    result.sum[band] = arg1.sum[band] - arg2.sum[band];
  if (result.sumsq_flag)
  {
    for (band = 0; band < result.nbands; band++)
      result.sumsq[band] = arg1.sumsq[band] - arg2.sumsq[band];
  }
  if (result.sumxlogx_flag)
  {
    for (band = 0; band < result.nbands; band++)
      result.sumxlogx[band] = arg1.sumxlogx[band] - arg2.sumxlogx[band];
  }
  if (result.std_dev_flag)
  {
    for (band = 0; band < result.nbands; band++)
      result.sum_pixel_std_dev[band] = arg1.sum_pixel_std_dev[band] - arg2.sum_pixel_std_dev[band];
  }

// These values returned to original default values - must be updated by other means
  result.band_max_std_dev = -1.0;
  result.max_edge_value = -FLT_MAX;
#ifdef RHSEG_RUN
  result.best_nghbr_dissim = FLT_MAX;
  result.nghbr_heap_index = UINT_MAX;
  result.best_region_dissim = FLT_MAX;
  result.region_heap_index = UINT_MAX;
#endif
  result.merge_region_label = 0;
  result.sum_pixel_gdissim = 0.0;
  result.min_region_dissim = FLT_MAX;
  result.boundary_npix = 0;
  result.merge_threshold = 0.0;
  result.nb_region_objects = 0;
  result.region_objects_set.clear();
  result.nghbrs_label_set.clear();
#ifdef RHSEG_RUN
  result.best_nghbr_label.clear();
  result.best_region_label.clear();
  result.nghbrs_label_seam_map.clear();
#endif // RHSEG_RUN

  return result;
 }

 bool compare_region_classes(const RegionClass& region_class, const RegionClass& check_region_class)
 {
   int band;

   if (region_class.label != check_region_class.label)
   {
     if (params.debug > 0)
       params.log_fs << "label check in compare_region_classes returned false" << endl;
     return false;
   }
   if (region_class.npix != check_region_class.npix)
   {
     if (params.debug > 0)
       params.log_fs << "npix check in compare_region_classes returned false" << endl;
     return false;
   }
   for (band = 0; band < region_class.nbands; band++)
     if (((region_class.sum[band]/check_region_class.sum[band]) > 1.01) ||
         ((region_class.sum[band]/check_region_class.sum[band]) < 0.99))
     {
       if (params.debug > 0)
       {
         params.log_fs << "sum check in compare_region_classes returned false" << endl;
         params.log_fs << "region_class.sum = " << region_class.sum[band] << endl;
         params.log_fs << "check_region_class.sum = " << check_region_class.sum[band] << endl;
         params.log_fs << "ratio = " << (region_class.sum[band]/check_region_class.sum[band]) << endl;
       }
       return false;
     }
   if (region_class.sumsq_flag)
   {
     for (band = 0; band < region_class.nbands; band++)
       if (((region_class.sumsq[band]/check_region_class.sumsq[band]) > 1.01) ||
           ((region_class.sumsq[band]/check_region_class.sumsq[band]) < 0.99))
       {
         if (params.debug > 0)
         {
           params.log_fs << "sumsq check in compare_region_classes returned false" << endl;
           params.log_fs << "region_class.sumsq = " << region_class.sumsq[band] << endl;
           params.log_fs << "check_region_class.sumsq = " << check_region_class.sumsq[band] << endl;
           params.log_fs << "ratio = " << (region_class.sumsq[band]/check_region_class.sumsq[band]) << endl;
         }
         return false;
       }
   }
   if (region_class.sumxlogx_flag)
   {
     for (band = 0; band < region_class.nbands; band++)
       if (((region_class.sumxlogx[band]/check_region_class.sumxlogx[band]) > 1.01) ||
           ((region_class.sumxlogx[band]/check_region_class.sumxlogx[band]) < 0.99))
       {
         if (params.debug > 0)
           params.log_fs << "sumxlogx check in compare_region_classes returned false" << endl;
         return false;
       }
   }
   if (region_class.std_dev_flag)
   {
     for (band = 0; band < region_class.nbands; band++)
       if (((region_class.sum_pixel_std_dev[band]/check_region_class.sum_pixel_std_dev[band]) > 1.01) ||
           ((region_class.sum_pixel_std_dev[band]/check_region_class.sum_pixel_std_dev[band]) < 0.99))
       {
         if (params.debug > 0)
           params.log_fs << "sum_pixel_std_dev check in compare_region_classes returned false" << endl;
         return false;
       }
   }

   if (!region_class.nghbrs_label_set.empty())
   {
     set<unsigned int>::const_iterator check_nghbrs_label_set_iter, nghbrs_label_set_iter = region_class.nghbrs_label_set.begin();
     while (nghbrs_label_set_iter != region_class.nghbrs_label_set.end())
     {
       check_nghbrs_label_set_iter = check_region_class.nghbrs_label_set.find((*nghbrs_label_set_iter));
       if (check_nghbrs_label_set_iter == check_region_class.nghbrs_label_set.end())
       {
         if (params.debug > 0)
           params.log_fs << "nghbrs_label_set.find check returned false" << endl;
         return false;
       } 
       ++nghbrs_label_set_iter;
     }
   }

   return true;
 }

#ifdef RHSEG_RUN
 double calc_region_dissim(RegionClass *region1, RegionClass *region2, bool merge_accel_flag)
 {
   int band, nbands = region1->nbands;
   double reg1_mean, reg2_mean, reg_sum, reg_mean, reg_npix;
   double result, sqdiff, entropy1, entropy2;
   double reg1_npix = (double) region1->npix;
   double reg2_npix = (double) region2->npix;

   double sumsqdiff = 0.0, norm1 = 0.0, norm2 = 0.0, scalar_prod = 0.0, entropy = 0.0;
   reg_npix = reg1_npix + reg2_npix;
   for (band=0; band < nbands; band++)
   {
     reg1_mean = region1->sum[band]/reg1_npix;
     reg2_mean = region2->sum[band]/reg2_npix;
     if ((params.dissim_crit == 4) || (params.dissim_crit == 8))
     {
       norm1 += reg1_mean*reg1_mean;
       norm2 += reg2_mean*reg2_mean;
       scalar_prod += reg1_mean*reg2_mean;
     }
     else if (params.dissim_crit == 5)
     {
       norm1 += reg1_mean;
       norm2 += reg2_mean;
     }
     else if (params.dissim_crit == 9)
     {
       reg_sum = region1->sum[band] + region2->sum[band];
       reg_mean = reg_sum/reg_npix;
    // The next four lines ensure that the entropy result is symmetric
       entropy1 = region1->sum[band]*log(reg1_mean) + region2->sum[band]*log(reg2_mean);
       entropy2 = region2->sum[band]*log(reg2_mean) + region1->sum[band]*log(reg1_mean);
       entropy1 = (entropy1 + entropy2)/2.0;
       entropy1 = (entropy1 - reg_sum*log(reg_mean));
       if (params.normind == 1)
         entropy1 /= region1->meanval[band];
       entropy += entropy1;
     }
     else
     {
       sqdiff = reg1_mean - reg2_mean;
       if ((params.dissim_crit == 2) || (params.dissim_crit == 6) || (params.dissim_crit == 7))
         sqdiff = sqdiff*sqdiff;
       else if (sqdiff < 0.0)
         sqdiff = -sqdiff;
       if (params.dissim_crit == 10)
       {
         sqdiff /= (reg1_npix*reg1_mean + reg2_npix*reg2_mean);
       }
       if ((params.dissim_crit == 3) || (params.dissim_crit == 7))
       {
         if (sqdiff > sumsqdiff)
           sumsqdiff = sqdiff;
       }
       else
         sumsqdiff += sqdiff;
     }
   }
   if ((params.dissim_crit == 4) || (params.dissim_crit == 8))
   {
     result = scalar_prod/sqrt(norm1*norm2);
     if (params.dissim_crit == 4)
       result = acos(result);
     else
     {
       result = (acos(0.0)-acos(result))/acos(0.0);
       if (norm2 != 0.0)
         reg1_mean = norm1/norm2;
       else
         reg1_mean = FLT_MAX;
       if (norm1 != 0.0)
         reg2_mean = norm2/norm1;
       else
         reg2_mean = FLT_MAX;
       if (reg2_mean < reg1_mean)
         reg1_mean = reg2_mean;
       if ((norm1 == 0.0) && (norm2 == 0.0))
         reg1_mean = 1.0;
       result = 1.0 - reg1_mean*result;
     }
   }
   else if (params.dissim_crit == 5)
   {
     result = 0.0;
     for (band=0; band < nbands; band++)
     {
       reg1_mean = region1->sum[band]/reg1_npix;
       reg2_mean = region2->sum[band]/reg2_npix;
       reg1_mean /= norm1;
       reg2_mean /= norm2;
       if ((reg1_mean/reg2_mean) < 1.0)
         result += reg2_mean*log(reg2_mean/reg1_mean) - reg1_mean*log(reg1_mean/reg2_mean);
       else
         result += reg1_mean*log(reg1_mean/reg2_mean) - reg2_mean*log(reg2_mean/reg1_mean);
     }
   }
   else if (params.dissim_crit == 9)
   {
     result = entropy;
     if (result < 0.0)
       result = 0.0;
   }
   else
   {
     if (params.dissim_crit == 2)
       sumsqdiff = sqrt(sumsqdiff);
     if ((params.dissim_crit == 6) || (params.dissim_crit == 7))
     {
       sumsqdiff *= reg1_npix*reg2_npix;
       sumsqdiff /= (reg1_npix+reg2_npix);
#ifdef MSE_SQRT
       sumsqdiff = sqrt(sumsqdiff);  // Added to make dimensionality consistent.
#endif
     }

     if (params.dissim_crit == 10)
       sumsqdiff *= sqrt((reg1_npix*reg2_npix)*(reg1_npix+reg2_npix));

     result = sumsqdiff;
   }

   if ((params.std_dev_image_flag) && (result < FLT_MAX) &&
       (params.dissim_crit != 5) && (params.dissim_crit != 9))  // Should have params.std_dev_image_flag = false for these values of dissim_crit
   {
     double std_dev_result;
     for (band=0; band < nbands; band++)
     {
       reg1_mean = region1->get_std_dev(band);
       reg2_mean = region2->get_std_dev(band);
       if ((params.dissim_crit == 4) || (params.dissim_crit == 8))
       {
         norm1 += reg1_mean*reg1_mean;
         norm2 += reg2_mean*reg2_mean;
         scalar_prod += reg1_mean*reg2_mean;
       }
       else
       {
         sqdiff = reg1_mean - reg2_mean;
         if ((params.dissim_crit == 2) || (params.dissim_crit == 6) || (params.dissim_crit == 7))
           sqdiff = sqdiff*sqdiff;
         else if (sqdiff < 0.0)
           sqdiff = -sqdiff;
         if (params.dissim_crit == 10)
         {
           sqdiff /= (reg1_npix*reg1_mean + reg2_npix*reg2_mean);
         }
         if ((params.dissim_crit == 3) || (params.dissim_crit == 7))
         {
           if (sqdiff > sumsqdiff)
             sumsqdiff = sqdiff;
         }
         else
           sumsqdiff += sqdiff;
       }
     }
     if ((params.dissim_crit == 4) || (params.dissim_crit == 8))
     {
       std_dev_result = scalar_prod/sqrt(norm1*norm2);
       if (params.dissim_crit == 4)
         std_dev_result = acos(std_dev_result);
       else
       {
         std_dev_result = (acos(0.0)-acos(std_dev_result))/acos(0.0);
         if (norm2 != 0.0)
           reg1_mean = norm1/norm2;
         else
           reg1_mean = FLT_MAX;
         if (norm1 != 0.0)
           reg2_mean = norm2/norm1;
         else
           reg2_mean = FLT_MAX;
         if (reg2_mean < reg1_mean)
           reg1_mean = reg2_mean;
         if ((norm1 == 0.0) && (norm2 == 0.0))
           reg1_mean = 1.0;
         std_dev_result = 1.0 - reg1_mean*std_dev_result;
       }
     }
     else
     {
       if (params.dissim_crit == 2)
         sumsqdiff = sqrt(sumsqdiff);
       if ((params.dissim_crit == 6) || (params.dissim_crit == 7))
       {
         sumsqdiff *= reg1_npix*reg2_npix;
         sumsqdiff /= (reg1_npix+reg2_npix);
#ifdef MSE_SQRT
         sumsqdiff = sqrt(sumsqdiff);  // Added to make dimensionality consistent.
#endif
       }

       if (params.dissim_crit == 10)
         sumsqdiff *= sqrt((reg1_npix*reg2_npix)*(reg1_npix+reg2_npix));
 
       std_dev_result = sumsqdiff;
     }
     result += params.std_dev_wght*std_dev_result;
   } // if ((params.std_dev_image_flag) && (result < FLT_MAX))
  
   if ((merge_accel_flag) && ((region1->npix < params.min_npixels) || (region2->npix < params.min_npixels)))
   {
     double max_npix, factor;

     if (region1->npix > params.min_npixels)
       reg1_npix = params.min_npixels;
     if (region2->npix > params.min_npixels)
       reg2_npix = params.min_npixels;
     max_npix = reg1_npix;
     if (reg2_npix > max_npix)
       max_npix = reg2_npix;
     factor = ((double) 2*reg1_npix*reg2_npix)/((double) (max_npix*(reg1_npix+reg2_npix)));
     factor = sqrt(factor);
     result = factor*result;
   }

   if (result < SMALL_EPSILON)
     result = 0.0;

   return result;
 }

#ifdef PARALLEL
#include <parallel/region_class.cc>
#endif

#endif // RHSEG_RUN

} // namespace HSEGTilton
