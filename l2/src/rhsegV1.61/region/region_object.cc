/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>          File:  region_object.cc
   >>>>
   >>>>          See region_object.h for documentation
   >>>>
   >>>>          Date:  November 18, 2003
   >>>> Modifications:  January 27, 2006 - Eliminated the merge_nghbr_set member variable.
   >>>>                 November 15, 2007 - Modified to work with hseg_read program.
   >>>>                 March 31, 2008 - Replaced std_dev feature with mean normalized std_dev feature.
   >>>>                 October 7, 2010 - Set result of dissimilarity calculations to 0.0 if computed result is less than SMALL_EPSILON.
   >>>>			March 1, 2013 - Replaced the std_dev member variable with sum_pixel_std_dev as part of the expansion
   >>>>				        of the use of the standard deviation spatial feature.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "region_object.h"
#include "region_class.h"
#include <params/params.h>
#include <pixel/pixel.h>
#include <spatial/spatial.h>
#include <iostream>
#include <float.h>
#include <cmath>

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{

 bool RegionObject::sumsq_flag;
 bool RegionObject::sumxlogx_flag;
 bool RegionObject::std_dev_flag;
 bool RegionObject::region_class_nghbrs_list_flag;
 bool RegionObject::region_object_nghbrs_list_flag;
 int  RegionObject::nbands;
 double *RegionObject::scale;
 double *RegionObject::offset;
 double *RegionObject::minval;
 double *RegionObject::meanval;

 RegionObject::RegionObject( )
 {
  int band;

// Initialize member variables
  active_flag = false;
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
  sumxlogx = NULL;
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
  band_max_std_dev = -1.0;  // Value less than zero signifies this variable is uninitialized.
  class_nghbrs_set.clear( );
  object_nghbrs_set.clear( );
  merge_region_label = 0;
  min_region_dissim = FLT_MAX;
  boundary_npix = 0;

  return;
 }

 RegionObject::RegionObject(const RegionObject& source)
 {
  int band;

// Copy member variables
  active_flag = source.active_flag;
  label = source.label;
  npix = source.npix;
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

  class_nghbrs_set_copy(source);
  object_nghbrs_set_copy(source);
  merge_region_label = source.merge_region_label;
  min_region_dissim = source.min_region_dissim;
  boundary_npix = source.boundary_npix;

  return;
 }

 RegionObject::~RegionObject( )
 {
  delete [ ] sum;
  if (sumsq_flag)
    delete [ ] sumsq;
  if (sumxlogx_flag)
    delete [ ] sumxlogx;
  if (std_dev_flag)
    delete [ ] sum_pixel_std_dev;
  return;
 }

 void RegionObject::set_static_vals()
 {
  int band;

  sumsq_flag = params.region_sumsq_flag;
  sumxlogx_flag = params.region_sumxlogx_flag;
  std_dev_flag = params.std_dev_image_flag;
  nbands = params.nbands;
  scale = new double[nbands];
  offset = new double[nbands];
  minval = new double[nbands];
  meanval = new double[nbands];
 // Coarsen to float to maintain consistency of results between different computing platforms and operating systems.
  for (band = 0; band < nbands; band++)
  {
    scale[band] = (float) (oparams.scale[band]*params.scale[band]);
    offset[band] = (float) (params.offset[band] + (oparams.offset[band]/params.scale[band]));
    minval[band] = (float) oparams.minval[band];
    meanval[band] = (float) oparams.meanval[band];
  }
  return;
 }

 void RegionObject::set_static_vals(const bool& classes_flag, const bool& objects_flag)
 {
  int band;

  sumsq_flag = params.region_sumsq_flag;
  sumxlogx_flag = params.region_sumxlogx_flag;
  std_dev_flag = params.std_dev_image_flag;
  region_class_nghbrs_list_flag = classes_flag;
  region_object_nghbrs_list_flag = objects_flag;
  nbands = params.nbands;
  scale = new double[nbands];
  offset = new double[nbands];
  for (band = 0; band < nbands; band++)
  {
    scale[band] = oparams.scale[band];
    offset[band] = oparams.offset[band];
  }
  return;
 }

 void RegionObject::operator =(const RegionObject& source)
 {
  int band;

  if (this == &source)
    return;

// Copy member variables
  active_flag = source.active_flag;
  label = source.label;
  npix = source.npix;
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

  class_nghbrs_set_copy(source);
  object_nghbrs_set_copy(source);
  merge_region_label = source.merge_region_label;
  min_region_dissim = source.min_region_dissim;
  boundary_npix = source.boundary_npix;

  return;
 }

 void RegionObject::operator +=(const RegionObject& source)
 {
  int band;

// Update member variables
  npix += source.npix;
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
  band_max_std_dev = -1.0;
  class_nghbrs_set.clear( );
  object_nghbrs_set.clear( );
  merge_region_label = 0;
  min_region_dissim = FLT_MAX;
  boundary_npix = 0;

 }

 void RegionObject::clear( )
 {
      int band;

      active_flag = false;
// Clear everything EXCEPT label and merge_region_label!!!
      npix = 0;
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
      band_max_std_dev = -1.0;

      class_nghbrs_set.clear( );
      object_nghbrs_set.clear( );
      min_region_dissim = FLT_MAX;
      boundary_npix = 0;

      return;
 }

 void RegionObject::clear_region_object_info( )
 {
      boundary_npix = 0;
      min_region_dissim = FLT_MAX;

      return;
 }

 void RegionObject::set_min_region_dissim()
 {
 // Current region is compared to minimum vector region of same size.
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
         (params.dissim_crit == 9) || (params.dissim_crit == 10))
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
       (params.dissim_crit == 8))
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

#ifdef RHSEG_RUN
 void RegionObject::init(Pixel *this_pixel)
 {
   int band;
   double temp_value;

   active_flag = true;
   npix++;
   for (band = 0; band < nbands; band++)
   {
     temp_value = (double) this_pixel->get_input_data(band);
     temp_value = scale[band]*(temp_value - offset[band]);
     sum[band] += temp_value;
     if (sumsq_flag)
       sumsq[band] += temp_value*temp_value;
     if (sumxlogx_flag)
       sumxlogx[band] += temp_value*log(temp_value);
     if (std_dev_flag)
     {
       sum_pixel_std_dev[band] += this_pixel->get_local_std_dev(band);
     }
   }

   return;
 }
#endif
/*
 void RegionObject::calc_std_dev( )
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
 void RegionObject::set_band_max_std_dev()
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

 void RegionObject::merge_regions(RegionObject *merge_region,
                                vector<RegionObject>& region_objects)
 {
    int band;
#ifdef DEBUG
    if (params.debug > 3)
    {
      params.log_fs << endl << "Merging regions with labels " << merge_region->label << " and " << this->label << endl;
    }
    if (params.debug > 3)
    {
      merge_region->print(region_objects);
      this->print(region_objects);
    }
#endif
    if (!active_flag)
    {
      if (params.debug > 0)
        params.log_fs << "Region label " << label << " is inactive:  can't participate in a merge!!" << endl;
      else
        cout << "Region label " << label << " is inactive:  can't participate in a merge!!" << endl;
    }
    if (!(merge_region->active_flag))
    {
      if (params.debug > 0)
        params.log_fs << "Region label " << merge_region->label << " is inactive:  can't participate in a merge!!" << endl;
      else
        cout << "Region label " << merge_region->label << " is inactive:  can't participate in a merge!!" << endl;
    }

// Update merged region npix, sum values, and, if necessary, update
// the sumsq values and calculate the std_dev value.
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

// Flag merged out region (merge_region) as inactive and set merge_region.
    merge_region->active_flag = false;
    merge_region->merge_region_label = label;
#ifdef DEBUG
    if (params.debug > 3)
    {
      params.log_fs << "After merging: " << endl;
      this->print(region_objects);
    }
#endif
    return;
 }

 void RegionObject::class_nghbrs_set_copy(const RegionObject& source)
 {
    class_nghbrs_set.clear( );
    if (!source.class_nghbrs_set.empty( ))
    {
      set<unsigned int>::const_iterator class_nghbrs_iter = source.class_nghbrs_set.begin( );
      while (class_nghbrs_iter != source.class_nghbrs_set.end( ))
      {
        class_nghbrs_set.insert(*class_nghbrs_iter);
        ++class_nghbrs_iter;
      }
    }
    return;
 }

 void RegionObject::object_nghbrs_set_copy(const RegionObject& source)
 {
    object_nghbrs_set.clear( );
    if (!source.object_nghbrs_set.empty( ))
    {
      set<unsigned int>::const_iterator object_nghbrs_iter = source.object_nghbrs_set.begin( );
      while (object_nghbrs_iter != source.object_nghbrs_set.end( ))
      {
        object_nghbrs_set.insert(*object_nghbrs_iter);
        ++object_nghbrs_iter;
      }
    }
    return;
 }

#ifdef RHSEG_READ
#ifdef THREEDIM
 void RegionObject::object_nghbrs_set_init(const short unsigned int& hlevel, Spatial& spatial_data,
                                           vector<RegionClass>& region_classes, vector<RegionObject>& region_objects, 
                                           const int& col, const int& row, const int& slice)
#else
 void RegionObject::object_nghbrs_set_init(const short unsigned int& hlevel, Spatial& spatial_data,
                                           vector<RegionClass>& region_classes, vector<RegionObject>& region_objects, 
                                           const int& col, const int& row)
#endif
 {
 // Initializes this region's class_nghbrs_set and object_nghbrs_set from the spatial_data information
   short unsigned int nbdir;
   unsigned int nghbr_pixel_index, nghbr_region_object_index, nghbr_region_class_index;
   unsigned int nghbr_region_object_label, nghbr_region_class_label;
   int nbcol, nbrow;
#ifdef THREEDIM
   int nbslice;
#endif

   for (nbdir = 0; nbdir < params.maxnbdir; ++nbdir)
   {
#ifdef THREEDIM
     find_nghbr(col,row,slice,nbdir,nbcol,nbrow,nbslice);
     if ((nbcol>=0)&&(nbrow>=0)&&(nbslice>=0)&&
         (nbcol<params.ncols)&&(nbrow<params.nrows)&&(nbslice<params.nslices))
     {
       nghbr_pixel_index = nbcol + nbrow*params.ncols + nbslice*params.nrows*params.ncols;
#else
     find_nghbr(col,row,nbdir,nbcol,nbrow);
     if ((nbcol>=0)&&(nbrow>=0)&&(nbcol<params.ncols)&&(nbrow<params.nrows))
     {
       nghbr_pixel_index = nbcol + nbrow*params.ncols;
#endif
       nghbr_region_object_label = spatial_data.get_region_object_label(nghbr_pixel_index);
       if ((hlevel > 0) && (nghbr_region_object_label > 0))
       {
         nghbr_region_object_index = nghbr_region_object_label - 1;
         if (!region_objects[nghbr_region_object_index].get_active_flag())
           nghbr_region_object_label = region_objects[nghbr_region_object_index].get_merge_region_label();
       }
       if ((nghbr_region_object_label > 0) &&
           (nghbr_region_object_label != label))
       {
         if (region_class_nghbrs_list_flag)
         {
           nghbr_region_class_label = spatial_data.get_region_class_label(nghbr_pixel_index);
           if (nghbr_region_class_label > 0)
           {
             if (hlevel > 0)
             {
               nghbr_region_class_index = nghbr_region_class_label - 1;
               if (!region_classes[nghbr_region_class_index].get_active_flag())
                 nghbr_region_class_label = region_classes[nghbr_region_class_index].get_merge_region_label();
             }
             class_nghbrs_set.insert(nghbr_region_class_label);
           }
         }
         if (region_object_nghbrs_list_flag)
           object_nghbrs_set.insert(nghbr_region_object_label);
       }
     }
   }

   return;
 }
#endif // RHSEG_READ

 double RegionObject::get_unscaled_mean(int band) const
 {
   return (((sum[band]/npix)/oparams.scale[band])+oparams.offset[band]);
 }

 double RegionObject::get_unscaled_std_dev(int band) const
 {
   return (get_std_dev(band)/oparams.scale[band]);
 }

 double RegionObject::get_std_dev(int band) const
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

 double RegionObject::get_band_max_std_dev() const
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

 void RegionObject::print(vector<RegionObject>& region_objects)
 {
   if (active_flag)
   {
    int band;

    params.log_fs << endl << "For connected region label " << label << ", npix = " << npix;
    if (std_dev_flag)
      params.log_fs << ", std_dev = " << this->get_band_max_std_dev() << endl;
    else
      params.log_fs << endl;
    params.log_fs.precision(12);
    params.log_fs << "Region sum values:" << endl;
    for (band = 0; band < nbands; band++)
      params.log_fs << sum[band] << "  ";
    params.log_fs << endl;
    if (sumsq_flag)
    {
      params.log_fs << "Region sum square values:" << endl;
      for (band = 0; band < nbands; band++)
        params.log_fs << sumsq[band] << " " ;
      params.log_fs << endl;
    }
    if (sumxlogx_flag)
    {
      params.log_fs << "Region sum xlogx values:" << endl;
      for (band = 0; band < nbands; band++)
        params.log_fs << sumxlogx[band] << " " ;
      params.log_fs << endl;
    }
    if (std_dev_flag)
    {
      params.log_fs << "Region sum pixel standard deviation values:" << endl;
      for (band = 0; band < nbands; band++)
        params.log_fs << sum_pixel_std_dev[band] << " " ;
      params.log_fs << endl;
    }
    params.log_fs.precision(6);
    if (boundary_npix > 0)
      params.log_fs << endl << "Region has " << boundary_npix << " boundary pixels";
    params.log_fs << endl << endl;
   }
   else
   {
     if (merge_region_label != label)
     {
      params.log_fs << endl << "Connected region label " << label << " was merged into connected region label ";
      params.log_fs << merge_region_label << endl << endl;
     }
   }
   return;
 }

 void RegionObject::print(string& strMessage)
 {
   if (active_flag)
   {
    int band, max_print_bands = 10;
    float bp_ratio = 0.0;

    if (nbands < max_print_bands)
      max_print_bands = nbands;

    if (params.region_boundary_npix_flag)
      bp_ratio = ((float) boundary_npix)/((float) npix);

    strMessage = "Feature Values for Region Object " + stringify_int(label) + ":\n\n";
    strMessage += "npix = " + stringify_int(npix) + "\n";
    params.log_fs << "For Region Object " << label << ": npix = " << npix;
    if (params.region_std_dev_flag)
    {
      strMessage += "std_dev = " + stringify_float((float) this->get_band_max_std_dev()) + "\n";
      params.log_fs << ", std_dev = " << this->get_band_max_std_dev();
    }
    if (params.region_boundary_npix_flag)
    {
      strMessage += "boundary pixel ratio = " + stringify_float(bp_ratio) + "\n";
      params.log_fs << ", and boundary pixel ratio = " << bp_ratio << endl;
    }
    if (params.region_sum_flag)
    {
      strMessage += "mean values:\n";
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
    if (params.region_boundary_npix_flag)
      strMessage += "Region object has " + stringify_int(boundary_npix) + " boundary pixels\n";
    if (!class_nghbrs_set.empty( ))
      strMessage += "Region object has " + stringify_int(class_nghbrs_set.size()) + " class neighbors\n";
    else
      strMessage += "Region class neighbor list is empty\n";
    if (!object_nghbrs_set.empty( ))
      strMessage += "Region object has " + stringify_int(object_nghbrs_set.size()) + " object neighbors\n";
    else
      strMessage += "Region object neighbor is empty\n";
   }
   else
   {
    if (merge_region_label != 0)
    {
      strMessage += "Region object " + stringify_int(label) + " was merged into region object ";
      strMessage += stringify_int(merge_region_label) + "\n";
    }
    else
      strMessage += "Region object " + stringify_int(label) + " is inactive\n";
   }
   return;
 }

 double calc_region_object_dissim(RegionObject *region_object1, RegionObject *region_object2)
 {
   int band, nbands = region_object1->nbands;
   double reg1_mean, reg2_mean, reg_sum, reg_mean, reg_npix;
   double result, sqdiff, entropy1, entropy2;
   double reg1_npix = (double) region_object1->npix;
   double reg2_npix = (double) region_object2->npix;

   double sumsqdiff = 0.0, norm1 = 0.0, norm2 = 0.0, scalar_prod = 0.0, entropy = 0.0;
   reg_npix = reg1_npix + reg2_npix;
   for (band=0; band < nbands; band++)
   {
     reg1_mean = region_object1->sum[band]/reg1_npix;
     reg2_mean = region_object2->sum[band]/reg2_npix;
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
       reg_sum = reg1_npix*reg1_mean + reg2_npix*reg2_mean;
       reg_mean = reg_sum/reg_npix;
    // The next four lines ensure that the entropy result is symmetric
       entropy1 = region_object1->sum[band]*log(reg1_mean) + region_object2->sum[band]*log(reg2_mean);
       entropy2 = region_object2->sum[band]*log(reg2_mean) + region_object1->sum[band]*log(reg1_mean);
       entropy1 = (entropy1 + entropy2)/2.0;
       entropy1 = (entropy1 - reg_sum*log(reg_mean));
       if (params.normind == 1)
         entropy1 /= region_object1->meanval[band];
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
         sqdiff *= (reg1_npix + reg2_npix);
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
       reg1_mean = region_object1->sum[band]/reg1_npix;
       reg2_mean = region_object2->sum[band]/reg2_npix;
       reg1_mean /= norm1;
       reg2_mean /= norm2;
       if ((reg1_mean/reg2_mean) < 1.0)
         result += reg2_mean*log(reg2_mean/reg1_mean) - reg1_mean*log(reg1_mean/reg2_mean);
       else
         result += reg1_mean*log(reg1_mean/reg2_mean) - reg2_mean*log(reg2_mean/reg1_mean);
     }
     result = ((float) result);
   }
   else if (params.dissim_crit == 9)
   {
     result = ((float) (entropy));
     if (result < 0.0)
       result = 0.0;
   }
   else
   {
     if (params.dissim_crit == 2)
       sumsqdiff = sqrt(sumsqdiff);
     if ((params.dissim_crit == 6) || (params.dissim_crit == 7))
     {
       sumsqdiff = reg1_npix*reg2_npix*sumsqdiff;
       sumsqdiff = sumsqdiff/(reg1_npix+reg2_npix);
#ifdef MSE_SQRT
       sumsqdiff = sqrt(sumsqdiff); // Added to make dimensionality consistent.
#endif
     }

     if (params.dissim_crit == 10)
       sumsqdiff *= sqrt((reg1_npix*reg2_npix)/(reg1_npix+reg2_npix));

     result = sumsqdiff;
   }

   if ((params.std_dev_image_flag) && (result < FLT_MAX) &&
       (params.dissim_crit != 5) && (params.dissim_crit != 9))  // Should have params.std_dev_image_flag = false for these values of dissim_crit
   {
     double std_dev_result;
     for (band=0; band < nbands; band++)
     {
       reg1_mean = region_object1->get_std_dev(band);
       reg2_mean = region_object2->get_std_dev(band);
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

   if (result < SMALL_EPSILON)
     result = 0.0;

   return result;
 }
#ifdef PARALLEL
#include <parallel/region_object.cc>
#endif
} // namespace HSEGTilton
