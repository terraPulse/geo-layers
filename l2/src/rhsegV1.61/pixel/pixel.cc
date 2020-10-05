/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>          File:  pixel.cc
   >>>>
   >>>>          See pixel.h for documentation
   >>>>
   >>>>          Date:  December 9, 2002
   >>>>
   >>>> Modifications:  February 7, 2003 - Changed region_index to region_label
   >>>>                 June 16, 2003 - Made std_dev_flag and nbands static
   >>>>                 August 8, 2005 - Added static sumsq_flag
   >>>>                 October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
   >>>>                 September 7, 2006 - Added static sumxlog_flag
   >>>>                 February 14, 2007 - Added init_flag
   >>>>                 March 12, 2007 - Updated functions to reflect change in member variables (see pixel.h)
   >>>>                 April 12, 2007 - Added scale_offset function (adapted from function previously in spatial.cc)
   >>>>                 June 27, 2007 - Modified factor for small region merge acceleration
   >>>>                 March 31, 2008 - Changed normalization to minumum value = 1.0 and std_dev = 1.0 for
   >>>>                                  all dissimilarity criterion (not just Entropy). This is needed in
   >>>>                                  to make the mean normalized std_dev feature to behave properly.
   >>>>                 May 12, 2008 - Modified to work with new Params object class
   >>>>                 September 5, 2008 - Modified scale_offset to return number of image pixels
   >>>>                 May 12, 2009 - Upgraded to work with float input image data.
   >>>>                 October 7, 2010 - Set result of dissimilarity calculations to 0.0 if computed result is less than SMALL_EPSILON.
   >>>>                 April 16, 2013 - Added the related Location class
   >>>>                 May 15, 2013 - Added location member variable to Pixel class
   >>>>                 January 8, 2014 - Removed the related Location class and location member variable
   >>>>                 January 10, 2014 - Changed operation of the calc_edge_pixel_dissim function
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "pixel.h"
#include <params/params.h>
#include <spatial/spatial.h>
#include <region/region_class.h>
#include <iostream>
#include <cmath>

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{

// Declaration of static member variables
 int        Pixel::nbands;
 RHSEGDType Pixel::dtype;
 bool       Pixel::std_dev_flag;

 Pixel::Pixel( )
 {
  int band;

  init_flag = false;
  mask = true;
  region_label = 0;
  switch(dtype)
  { 
    case UInt8:   byte_input_data = new unsigned char[nbands];
                  for (band = 0; band < nbands; band++)
                    byte_input_data[band] = 0;
                  break;
    case UInt16:  short_input_data = new unsigned short[nbands];
                  for (band = 0; band < nbands; band++)
                    short_input_data[band] = 0;
                  break;
    case Float32: float_input_data = new float[nbands];
                  for (band = 0; band < nbands; band++)
                    float_input_data[band] = 0.0;
                  break;
    default:      cout << "RHSEG style data type is invalid (p1)" << endl;
                  break;
  }
  std_dev_mask = false;
  if (std_dev_flag)
  {
    local_std_dev = new float[nbands];
    for (band = 0; band < nbands; band++)
      local_std_dev[band] = 0.0;
  }
  edge_mask = false;
  edge_value = -FLT_MAX;

  return;
 }

 Pixel::Pixel(const Pixel& source)
 {
  int band;

// Public variables
  init_flag = source.init_flag;
  mask = source.mask;
  region_label = source.region_label;
  switch(dtype)
  { 
    case UInt8:   byte_input_data = new unsigned char[nbands];
                  for (band = 0; band < nbands; band++)
                    byte_input_data[band] = source.byte_input_data[band];
                  break;
    case UInt16:  short_input_data = new unsigned short[nbands];
                  for (band = 0; band < nbands; band++)
                    short_input_data[band] = source.short_input_data[band];
                  break;
    case Float32: float_input_data = new float[nbands];
                  for (band = 0; band < nbands; band++)
                    float_input_data[band] = source.float_input_data[band];
                  break;
    default:      cout << "RHSEG style data type is invalid (p2)" << endl;
                  break;
  }
  if (std_dev_flag)
  {
    std_dev_mask = source.std_dev_mask;
    local_std_dev = new float[nbands];
    for (band = 0; band < nbands; band++)
      local_std_dev[band] = source.local_std_dev[band];
  }
  if (params.edge_image_flag)
  {
    edge_mask = source.edge_mask;
    edge_value = source.edge_value;
  }

  return;
 }

 Pixel::~Pixel( )
 {
  switch(dtype)
  { 
    case UInt8:   delete [ ] byte_input_data;
                  break;
    case UInt16:  delete [ ] short_input_data;
                  break;
    case Float32: delete [ ] float_input_data;
                  break;
    default:      cout << "RHSEG style data type is invalid (p3)" << endl;
                  break;
  }
  if (std_dev_flag)
    delete [ ] local_std_dev;

  return;
 }

 void Pixel::set_static_vals()
 {
  nbands = params.nbands;
  dtype  = params.dtype;
  std_dev_flag = params.std_dev_image_flag;

  return;
 }

 void Pixel::operator =(const Pixel& source)
 {
  int band;

  if (this == &source)
    return;

 // Public variables
  init_flag = source.init_flag;
  mask = source.mask;
  region_label = source.region_label;
  switch(dtype)
  { 
    case UInt8:   for (band = 0; band < nbands; band++)
                    byte_input_data[band] = source.byte_input_data[band];
                  break;
    case UInt16:  for (band = 0; band < nbands; band++)
                    short_input_data[band] = source.short_input_data[band];
                  break;
    case Float32: for (band = 0; band < nbands; band++)
                    float_input_data[band] = source.float_input_data[band];
                  break;
    default:      cout << "RHSEG style data type is invalid (p4)" << endl;
                  break;
  }
  if (std_dev_flag)
  {
    std_dev_mask = source.std_dev_mask;
    for (band = 0; band < nbands; band++)
      local_std_dev[band] = source.local_std_dev[band];
  }
  if (params.edge_image_flag)
  {
    edge_mask = source.edge_mask;
    edge_value = source.edge_value;
  }

  return;
 }

 void Pixel::clear( )
 {
  int band;

  init_flag = false;
  mask = false;
  region_label = 0;
  switch(dtype)
  { 
    case UInt8:   for (band = 0; band < nbands; band++)
                    byte_input_data[band] = 0;
                  break;
    case UInt16:  for (band = 0; band < nbands; band++)
                    short_input_data[band] = 0;
                  break;
    case Float32: for (band = 0; band < nbands; band++)
                    float_input_data[band] = 0.0;
                  break;
    default:      cout << "RHSEG style data type is invalid (p5)" << endl;
                  break;
  }
  std_dev_mask = false;
  if (std_dev_flag)
  {
    for (band = 0; band < nbands; band++)
      local_std_dev[band] = 0.0;
  }
  edge_mask = false;
  edge_value = -FLT_MAX;

  return;
 }

 void Pixel::print(const unsigned int& pixel_index)
 {
    int band;

    params.log_fs << endl << "Element " << pixel_index << ":" << endl;
    if (mask)
    {
      params.log_fs << "Pixel input data values:" << endl;
      for (band = 0; band < nbands; band++)
        params.log_fs << get_input_data(band) << "  ";
      params.log_fs << endl;
      if ((std_dev_flag) && (std_dev_mask))
      {
        params.log_fs << "Local pixel standard deviation values:" << endl;
        for (band = 0; band < nbands; band++)
          params.log_fs << get_local_std_dev(band) << "  ";
        params.log_fs << endl;
      }
      if ((params.edge_image_flag) && (edge_mask))
      {
        params.log_fs << "Edge value = " << edge_value << endl;
      }
      if (region_label != 0)
        params.log_fs << "Associated region has label " << region_label << endl;
      if (init_flag)
        params.log_fs << "init_flag = true." << endl;
    }
    else
      params.log_fs << "Pixel is masked." << endl;

    return;
 }

#ifdef DEBUG
 void Pixel::print_region_label()
 {
   if (!mask)
   {
     params.log_fs << "    0";
   }
   else
   {
     if (region_label < 10)
       params.log_fs << "    " << region_label;
     else if (region_label < 100)
       params.log_fs << "   " << region_label;
     else if (region_label < 1000)
       params.log_fs << "  " << region_label;
     else
       params.log_fs << " " << region_label;
   }
 }
#endif

void Pixel::set_input_data(const int& band, const float& value)
{ 
  switch(dtype)
  { 
    case UInt8:   byte_input_data[band] = (unsigned char) value;
                  break;
    case UInt16:  short_input_data[band] = (unsigned short) value;
                  break;
    case Float32: float_input_data[band] = value;
                  break;
    default:      break;
  }
  return;
}

unsigned int Pixel::get_region_label( )
{
  if (mask)
    return region_label;
  else
    return 0;
}

float Pixel::get_edge_value( )
{
  if (edge_mask)
    return (edge_value);
  else
    return -FLT_MAX;
}

float Pixel::get_input_data(const int& band)
{
  if (mask)
  {
    switch(dtype)
    { 
      case UInt8:   return byte_input_data[band];
                    break;
      case UInt16:  return short_input_data[band];
                    break;
      case Float32: return float_input_data[band];
                    break;
      default:      return 0.0;
                    break;
    }
  }
  else
    return 0.0;
}

float Pixel::get_local_std_dev(const int& band)
{
  if ((mask) && (std_dev_mask))
    return local_std_dev[band];
  else
    return 0.0;
}

double Pixel::update_pixel_dissim(RegionClass *dissim_region)
{
  double pixel_dissim;

  if (!dissim_region->get_active_flag( ))
  {
    if (params.debug > 0)
      params.log_fs << "Region label " << dissim_region->label << " is inactive:  can't use to update pixel_dissim!!" << endl;
    else
      cout << "Region label " << dissim_region->label << " is inactive:  can't use to update pixel_dissim!!" << endl;
  }
  if (dissim_region->npix == 1)
  {
    pixel_dissim = 0.0;
  }
  else
  {
    pixel_dissim = calc_region_pixel_dissim(dissim_region,this);
  }

  return pixel_dissim;
}

#ifdef PARALLEL
#include <parallel/pixel.cc>
#endif

 unsigned int scale_offset(vector<Pixel>& pixel_data, Temp& temp_data)
 {
 // Find the data scale and offset values, if not already provided.
  unsigned int npixels=0, index;
  int row, col, band;
#ifdef THREEDIM
  int slice;
#endif

  oparams.scale.resize(params.nbands);
  oparams.offset.resize(params.nbands);
  oparams.minval.resize(params.nbands);
  oparams.meanval.resize(params.nbands);

  params.max_edge_value = 0.0;
  params.min_edge_value = FLT_MAX;
  float pixel_edge_value;

  if (params.normind == 1)
  {
   // No additional normalization for this case
    for (band = 0; band < params.nbands; band++)
    {
      oparams.scale[band] = 1.0;
      oparams.offset[band] = 0.0;
    }

   // Still need to find minval and meanval
    double value, *min_stat, *sum_stat;
    min_stat = new double[params.nbands];
    sum_stat = new double[params.nbands];
    for (band = 0; band < params.nbands; band++)
    {
      min_stat[band] = FLT_MAX;
      sum_stat[band] = 0.0;
    }
#ifndef PARALLEL
    short unsigned int section;
    for (section = 0; section < params.nb_sections; ++section)
    {
      if (params.nb_sections > 1)
        restore_pixel_data(section,pixel_data,temp_data);
#endif
#ifdef THREEDIM
      for (slice = 0; slice < params.inb_nslices; slice++)
      {
#endif
        for (row = 0; row < params.inb_nrows; row++)
        {
          for (col = 0; col < params.inb_ncols; col++)
          {
#ifdef THREEDIM
            index = col + row*params.pixel_ncols + slice*params.pixel_ncols*params.pixel_nrows;
#else
            index = col + row*params.pixel_ncols;
#endif
            if (pixel_data[index].mask)
            {
              npixels++;
              for (band = 0; band < params.nbands; band++)
              {
                value = (double) pixel_data[index].get_input_data(band);
                value = params.scale[band]*(value - params.offset[band]);
                sum_stat[band] += value;
                if (min_stat[band] > value)
                  min_stat[band] = value;
              }
            }
            if (params.edge_image_flag)
            {
              if (pixel_data[index].edge_mask)
              {
                pixel_edge_value = pixel_data[index].get_edge_value();
                if (params.min_edge_value > pixel_edge_value)
                  params.min_edge_value = pixel_edge_value;
                if (params.max_edge_value < pixel_edge_value)
                  params.max_edge_value = pixel_edge_value;
              }
            }
          }
        }
#ifdef THREEDIM
      }
#endif
#ifndef PARALLEL
    }
#endif

#ifdef PARALLEL
   // Gather results from other tasks
    unsigned int temp_unsigned;
    temp_unsigned = npixels;
    MPI::COMM_WORLD.Allreduce(&temp_unsigned, &npixels, 1,
                              MPI::UNSIGNED, MPI::SUM);

    double temp_double;
    for (band = 0; band < params.nbands; band++)
    {
      temp_double = min_stat[band];
      MPI::COMM_WORLD.Allreduce(&temp_double, &min_stat[band], 1,
                                MPI::DOUBLE, MPI::MIN);
      temp_double = sum_stat[band];
      MPI::COMM_WORLD.Allreduce(&temp_double, &sum_stat[band], 1,
                                MPI::DOUBLE, MPI::SUM);
    }
    if (params.edge_image_flag)
    {
      float temp_float;
      temp_float = params.min_edge_value;
      MPI::COMM_WORLD.Allreduce(&temp_float, &params.min_edge_value, 1,
                                MPI::FLOAT, MPI::MIN);
      temp_float = params.max_edge_value;
      MPI::COMM_WORLD.Allreduce(&temp_float, &params.max_edge_value, 1,
                                MPI::FLOAT, MPI::MAX);
    }
#endif
    for (band = 0; band < params.nbands; band++)
    {
      oparams.minval[band] = ((float) (min_stat[band]));
      oparams.meanval[band] = ((float) (sum_stat[band]/((double) npixels)));
    }
#ifdef DEBUG
    if (params.debug > 2)
    {
      params.log_fs << "npixels = " << npixels << endl;
      for (band = 0; band < params.nbands; band++)
      {
        params.log_fs << "For band " << band << ", minimum value = " << oparams.minval[band];
        params.log_fs << " and mean value = " << oparams.meanval[band] << endl;
      }
      if (params.edge_image_flag)
        params.log_fs << "MAX(edge_value) = " << params.max_edge_value << " and MIN(edge_value) = " << params.min_edge_value << endl;
    }
#endif
  }
  else
  {
   // Need to compute scale and offset for normalization (as requested)

   // Calculate image data statistics
    double value, *min_stat, *sum_stat, *sumsq_stat;
    min_stat = new double[params.nbands];
    sum_stat = new double[params.nbands];
    sumsq_stat = new double[params.nbands];
    for (band = 0; band < params.nbands; band++)
    {
      min_stat[band] = FLT_MAX;
      sum_stat[band] = 0.0;
      sumsq_stat[band] = 0.0;
    }
#ifndef PARALLEL
    short unsigned int section;
    for (section = 0; section < params.nb_sections; ++section)
    {
      if (params.nb_sections > 1)
        restore_pixel_data(section,pixel_data,temp_data);
#endif
#ifdef THREEDIM
      for (slice = 0; slice < params.inb_nslices; slice++)
      {
#endif
        for (row = 0; row < params.inb_nrows; row++)
        {
          for (col = 0; col < params.inb_ncols; col++)
          {
#ifdef THREEDIM
            index = col + row*params.pixel_ncols + slice*params.pixel_ncols*params.pixel_nrows;
#else
            index = col + row*params.pixel_ncols;
#endif
            if (pixel_data[index].mask)
            {
              npixels++;
              for (band = 0; band < params.nbands; band++)
              {
                value = (double) pixel_data[index].get_input_data(band);
                value = params.scale[band]*(value - params.offset[band]);
                if (min_stat[band] > value)
                  min_stat[band] = value;
                sum_stat[band] += value;
                sumsq_stat[band] += value*value;
              }
            }
            if (params.edge_image_flag)
            {
              if (pixel_data[index].edge_mask)
              {
                pixel_edge_value = pixel_data[index].get_edge_value();
                if (params.min_edge_value > pixel_edge_value)
                  params.min_edge_value = pixel_edge_value;
                if (params.max_edge_value < pixel_edge_value)
                  params.max_edge_value = pixel_edge_value;
              }
            }
          }
        }
#ifdef THREEDIM
      }
#endif
#ifndef PARALLEL
    }
#endif

#ifdef DEBUG
    if (params.debug > 2)
    {
      params.log_fs << "npixels = " << npixels << endl;
      for (band = 0; band < params.nbands; band++)
      {
        params.log_fs << "For band " << band << ", minimum = " << min_stat[band];
        params.log_fs << " sum = " << sum_stat[band] << ", and " ;
        params.log_fs << "sum squared = " << sumsq_stat[band] << endl;
      }
      if (params.edge_image_flag)
        params.log_fs << "MAX(edge_value) = " << params.max_edge_value << " and MIN(edge_value) = " << params.min_edge_value << endl;
    }
#endif

#ifdef PARALLEL
   // Gather results from other tasks
    unsigned int temp_unsigned;
    temp_unsigned = npixels;
    MPI::COMM_WORLD.Allreduce(&temp_unsigned, &npixels, 1,
                              MPI::UNSIGNED, MPI::SUM);

    double temp_double;
    for (band = 0; band < params.nbands; band++)
    {
      temp_double = min_stat[band];
      MPI::COMM_WORLD.Allreduce(&temp_double, &min_stat[band], 1,
                                MPI::DOUBLE, MPI::MIN);
      temp_double = sum_stat[band];
      MPI::COMM_WORLD.Allreduce(&temp_double, &sum_stat[band], 1,
                                MPI::DOUBLE, MPI::SUM);
      temp_double = sumsq_stat[band];
      MPI::COMM_WORLD.Allreduce(&temp_double, &sumsq_stat[band], 1,
                                MPI::DOUBLE, MPI::SUM);
    }
    if (params.edge_image_flag)
    {
      float temp_float;
      temp_float = params.min_edge_value;
      MPI::COMM_WORLD.Allreduce(&temp_float, &params.min_edge_value, 1,
                                MPI::FLOAT, MPI::MIN);
      temp_float = params.max_edge_value;
      MPI::COMM_WORLD.Allreduce(&temp_float, &params.max_edge_value, 1,
                                MPI::FLOAT, MPI::MAX);
    }
#ifdef DEBUG
    if (params.debug > 2)
    {
      params.log_fs << "After results gathered from all tasks, npixels = " << npixels << endl;
      for (band = 0; band < params.nbands; band++)
      {
        params.log_fs << "For band " << band << ", minimum = " << min_stat[band];
        params.log_fs << " sum = " << sum_stat[band] << ", and " ;
        params.log_fs << "sum squared = " << sumsq_stat[band] << endl;
      }
      if (params.edge_image_flag)
        params.log_fs << "MAX(edge_value) = " << params.max_edge_value << " and MIN(edge_value) = " << params.min_edge_value << endl;
    }
#endif
#endif
    double *mean_stat, *stddev_stat;
    mean_stat = new double[params.nbands];
    stddev_stat = new double[params.nbands];
   // Calculate image variance from the image sum and sum of image squared values
    if (((params.debug > 0) && (params.nbands < 15)) || (params.debug > 2))
      params.log_fs << endl << "For Original Data:" << endl;
    for (band = 0; band < params.nbands; band++)
    {
      mean_stat[band] = sum_stat[band]/((double) npixels);
      stddev_stat[band] = (sum_stat[band]*sum_stat[band])/((double) npixels);
      stddev_stat[band] = sumsq_stat[band] - stddev_stat[band];
      stddev_stat[band] = stddev_stat[band]/(((double) npixels)-1.0);
      stddev_stat[band] = sqrt(stddev_stat[band]);
      if (((params.debug > 0) && (params.nbands < 15)) || (params.debug > 2))
      {
        params.log_fs << "For band " << band << ", minimum = " << min_stat[band];
        params.log_fs << ", mean = " << mean_stat[band];
        params.log_fs << ", and standard deviation = " << stddev_stat[band] << endl;
      }
    }
    if (((params.debug > 0) && (params.nbands < 15)) || (params.debug > 2))
      params.log_fs << endl;
    if (params.normind==2)
    {
     // For this case, use the MAX(stddev_stat) value.
      for (band = 1; band < params.nbands; band++)
      {
        if (stddev_stat[0] < stddev_stat[band])
          stddev_stat[0] = stddev_stat[band];
      }
      for (band = 1; band < params.nbands; band++)
      {
        stddev_stat[band] = stddev_stat[0];
      }
    }
  // Compute the overall scale and offset values such that the mean data value will
  // be 0.0 and the data variance (referenced to input scaling) will be 1.0.
    for (band = 0; band < params.nbands; band++)
    {
      oparams.scale[band] = (1.0/stddev_stat[band]);
      oparams.offset[band] = mean_stat[band];
      oparams.minval[band] = oparams.scale[band]*(min_stat[band] - mean_stat[band]);
      oparams.meanval[band] = 0.0;
    }
  // Now readjust the overall scale and offset values such that the minimum data value will be 1.0, 
  // mean data values will be equal and the data variance (referenced to input scaling) will be 1.0.
    double min_val = oparams.minval[0];
    for (band = 1; band < params.nbands; band++)
      if (oparams.minval[band] < min_val)
        min_val = oparams.minval[band];
    for (band = 0; band < params.nbands; band++)
    {
      oparams.offset[band] = oparams.offset[band] + (min_val - 1.0)/oparams.scale[band];
      oparams.meanval[band] -= (min_val - 1.0);
      oparams.minval[band] -= (min_val - 1.0);
    }

    delete [ ] min_stat;
    delete [ ] mean_stat;
    delete [ ] sum_stat;
    delete [ ] sumsq_stat;
    delete [ ] stddev_stat;

  }

  if (((params.debug > 0) && (params.nbands < 15)) || (params.debug > 2))
  {
    for (band = 0; band < params.nbands; band++)
    {
      params.log_fs << "Input scale[" << band << "] = " << params.scale[band] ;
      params.log_fs << " and offset[" << band << "] = " << params.offset[band] << endl;
      params.log_fs << "Normalization scale[" << band << "] = " << oparams.scale[band] ;
      params.log_fs << " and offset[" << band << "] = " << oparams.offset[band] << endl;
    }
    params.log_fs << endl;
    if (params.edge_image_flag)
      params.log_fs << "MAX(edge_value) = " << params.max_edge_value << " and MIN(edge_value) = " << params.min_edge_value << endl;
  }

  if (((params.debug > 0) && (params.nbands < 15)) || (params.debug > 2))
  {
    float scale, offset;
    for (band = 0; band < params.nbands; band++)
    {
      scale = (float) (oparams.scale[band]*params.scale[band]);
      offset = (float) (params.offset[band] + (oparams.offset[band]/params.scale[band]));
      params.log_fs << "Overall scale[" << band << "] = " << scale;
      params.log_fs << ", offset[" << band << "] = " << offset;
      params.log_fs << ", meanval[" << band << "] = " << oparams.meanval[band];
      params.log_fs << ", and minval[" << band << "] = " << oparams.minval[band] << endl;
    }
    params.log_fs << endl;
  }

  return npixels;
 }

 float calc_edge_pixel_dissim(RegionClass *region, Pixel *pixel)
 {
   float max_edge_value1, max_edge_value2;

   max_edge_value1 = region->get_max_edge_value();

   if (pixel->get_edge_mask())
     max_edge_value2 = pixel->get_edge_value();
   else
     max_edge_value2 = -FLT_MAX;

   if ((max_edge_value1 < 0.0) || (max_edge_value2 < 0.0))
     return params.max_edge_value;

  // At this point will either have (max_edge_value1 >= 0.0) and (max_edge_value2 >= 0.0) 
   if (max_edge_value1 >= max_edge_value2)
     return max_edge_value1;
   else
     return max_edge_value2;
 }

 double calc_region_pixel_dissim(RegionClass *region, Pixel *pixel)
 {
   int band, nbands = region->nbands;
   double temp_value, region_mean, pixel_mean, result, sqdiff;
   double merged_npix, merged_sum, merged_mean, entropy1, entropy2;
   double region_npix = (double) region->npix;

   double sumsqdiff = 0.0, region_norm = 0.0, pixel_norm = 0.0, scalar_prod = 0.0, entropy = 0.0;
   for (band=0; band < nbands; band++)
   {
     region_mean = region->sum[band]/region_npix;
     temp_value = (double) pixel->get_input_data(band);
     temp_value = region->scale[band]*((double) temp_value - region->offset[band]);
     pixel_mean = temp_value;
     if ((params.dissim_crit == 4) || (params.dissim_crit == 8))
     {
       region_norm += region_mean*region_mean;
       pixel_norm += pixel_mean*pixel_mean;
       scalar_prod += region_mean*pixel_mean;
     }
     else if (params.dissim_crit == 5)
     {
       region_norm += region_mean;
       pixel_norm += pixel_mean;
     }
     else if (params.dissim_crit == 9)
     {
       merged_npix = region_npix + 1;
       merged_sum = region->sum[band] + pixel_mean;
       merged_mean = merged_sum/merged_npix;
    // The next four lines ensure that the entropy result is symmetric
    // (NOTE: pixel_mean = pixel_sum because pixel_npix = 1)
       entropy1 = region->sum[band]*log(region_mean) + pixel_mean*log(pixel_mean);
       entropy2 = pixel_mean*log(pixel_mean) + region->sum[band]*log(region_mean);
       entropy1 = (entropy1 + entropy2)/2.0;
       entropy1 = (entropy1 - merged_sum*log(merged_mean));
       if (params.normind == 1)
         entropy1 /= region->meanval[band];
       entropy += entropy1;
     }
     else
     {
       sqdiff = region_mean - pixel_mean;
       if ((params.dissim_crit == 2) || (params.dissim_crit == 6) || (params.dissim_crit == 7))
         sqdiff = sqdiff*sqdiff;
       else if (sqdiff < 0.0)
         sqdiff = -sqdiff;
       if (params.dissim_crit == 10)
       {
         sqdiff /= (region_npix*region_mean + pixel_mean);
         sqdiff *= (region_npix + 1.0);
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
     result = scalar_prod/sqrt(region_norm*pixel_norm);
     if (params.dissim_crit == 4)
       result = acos(result);
     else
     {
       result = (acos(0.0)-acos(result))/acos(0.0);
       if (pixel_norm != 0.0)
         region_mean = region_norm/pixel_norm;
       else
         region_mean = FLT_MAX;
       if (region_norm != 0.0)
         pixel_mean = pixel_norm/region_norm;
       else
         pixel_mean = FLT_MAX;
       if (pixel_mean < region_mean)
         region_mean = pixel_mean;
       if ((region_norm == 0.0) && (pixel_norm == 0.0))
         region_mean = 1.0;
       result = 1.0 - region_mean*result;
     }
   }
   else if (params.dissim_crit == 5)
   {
     result = 0.0;
     for (band=0; band < nbands; band++)
     {
       region_mean = region->sum[band]/region_npix;
       pixel_mean = region->scale[band]*((double) pixel->get_input_data(band) - region->offset[band]);
       region_mean /= region_norm;
       pixel_mean /= pixel_norm;
       if ((region_mean/pixel_mean) < 1.0)
         result += pixel_mean*log(pixel_mean/region_mean) - region_mean*log(region_mean/pixel_mean);
       else
         result += region_mean*log(region_mean/pixel_mean) - pixel_mean*log(pixel_mean/region_mean);
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
       sumsqdiff *= region_npix;
       sumsqdiff /= (region_npix+1.0);
#ifdef MSE_SQRT
       sumsqdiff = sqrt(sumsqdiff);  // Added to make dimensionality consistent.
#endif
     }

     if (params.dissim_crit == 10)
       sumsqdiff *= sqrt((region_npix)/(region_npix + 1.0));

     result = sumsqdiff;
   }

   if ((params.std_dev_image_flag) && (result < FLT_MAX) &&
       (params.dissim_crit != 5) && (params.dissim_crit != 9))  // Should have params.std_dev_image_flag = false for these values of dissim_crit
   {
     double std_dev_result;
     for (band=0; band < nbands; band++)
     {
       region_mean = region->get_std_dev(band);
       temp_value = (double) pixel->get_local_std_dev(band);
       temp_value = region->scale[band]*((double) temp_value);
       pixel_mean = temp_value;
       if ((params.dissim_crit == 4) || (params.dissim_crit == 8))
       {
         region_norm += region_mean*region_mean;
         pixel_norm += pixel_mean*pixel_mean;
         scalar_prod += region_mean*pixel_mean;
       }
       else
       {
         sqdiff = region_mean - pixel_mean;
         if ((params.dissim_crit == 2) || (params.dissim_crit == 6) || (params.dissim_crit == 7))
           sqdiff = sqdiff*sqdiff;
         else if (sqdiff < 0.0)
           sqdiff = -sqdiff;
         if (params.dissim_crit == 10)
         {
           sqdiff /= (region_npix*region_mean + pixel_mean);
           sqdiff *= (region_npix + 1.0);
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
       std_dev_result = scalar_prod/sqrt(region_norm*pixel_norm);
       if (params.dissim_crit == 4)
         std_dev_result = acos(std_dev_result);
       else
       {
         std_dev_result = (acos(0.0)-acos(std_dev_result))/acos(0.0);
         if (pixel_norm != 0.0)
           region_mean = region_norm/pixel_norm;
         else
           region_mean = FLT_MAX;
         if (region_norm != 0.0)
           pixel_mean = pixel_norm/region_norm;
         else
           pixel_mean = FLT_MAX;
         if (pixel_mean < region_mean)
           region_mean = pixel_mean;
         if ((region_norm == 0.0) && (pixel_norm == 0.0))
           region_mean = 1.0;
         std_dev_result = 1.0 - region_mean*std_dev_result;
       }
     }
     else
     {
       if (params.dissim_crit == 2)
         sumsqdiff = sqrt(sumsqdiff);
       if ((params.dissim_crit == 6) || (params.dissim_crit == 7))
       {
         sumsqdiff *= region_npix;
         sumsqdiff /= (region_npix+1.0);
#ifdef MSE_SQRT
         sumsqdiff = sqrt(sumsqdiff);  // Added to make dimensionality consistent.
#endif
       }

       if (params.dissim_crit == 10)
         sumsqdiff *= sqrt((region_npix)/(region_npix + 1.0));
 
       std_dev_result = sumsqdiff;
     }
     result += params.std_dev_wght*std_dev_result;
   } // if ((params.std_dev_image_flag) && (result < FLT_MAX))
  
   if (result < SMALL_EPSILON)
     result = 0.0;

   return result;
 }
#ifndef PARALLEL
 void save_pixel_data(const short unsigned int& section, vector<Pixel>& pixel_data, Temp& temp_data)
 {
   if (params.debug > 3)
     params.log_fs << "Saving pixel_data for section " << section << endl;

   char *char_section;
   char_section = (char *) malloc(4*sizeof(char));

   string temp_file_name;
   temp_file_name = params.temp_file_name + ".pixel_sec";

   string io_file_name;
   sprintf(char_section,"%03d",section);
   io_file_name = temp_file_name + char_section;

   fstream io_file_fs;
   io_file_fs.open(io_file_name.c_str( ), ios_base::out | ios_base::binary );

   int index, band;
#ifdef THREEDIM
   int nelements = params.ionb_ncols*params.ionb_nrows*params.ionb_nslices;
#else
   int nelements = params.ionb_ncols*params.ionb_nrows;
#endif
   int byte_nelements, short_nelements, int_nelements, float_nelements;
   byte_nelements = short_nelements = int_nelements = float_nelements = 0;
   switch(params.dtype)
   {
     case UInt8:   byte_nelements = nelements*params.nbands;
                   short_nelements = 0;
                   float_nelements = 0;
                   if (params.std_dev_image_flag)
                     float_nelements = nelements*params.nbands;
                   else if (params.edge_image_flag)
                     float_nelements = nelements;
                   break;
     case UInt16:  byte_nelements = nelements;
                   short_nelements = nelements*params.nbands;
                   float_nelements = 0;
                   if (params.std_dev_image_flag)
                     float_nelements = nelements*params.nbands;
                   else if (params.edge_image_flag)
                     float_nelements = nelements;
                   break;
     case Float32: byte_nelements = nelements;
                   short_nelements = 0;
                   float_nelements = nelements*params.nbands;
                   break;
     default:      cout << "RHSEG style data type is invalid (p6)" << endl;
                   break;
   }
   int_nelements = nelements;
   check_buf_size(byte_nelements,short_nelements,int_nelements,float_nelements,0,temp_data);

   for (index = 0; index < nelements; ++index)
     if (pixel_data[index].init_flag)
       temp_data.byte_buffer[index] = 1;
     else
       temp_data.byte_buffer[index] = 0;
   io_file_fs.write(reinterpret_cast<char *>(temp_data.byte_buffer),nelements);
   if ((params.mask_flag || params.padded_flag))
   {
     for (index = 0; index < nelements; ++index)
       if (pixel_data[index].mask)
         temp_data.byte_buffer[index] = 1;
       else
         temp_data.byte_buffer[index] = 0;
     io_file_fs.write(reinterpret_cast<char *>(temp_data.byte_buffer),nelements);
   }
   for (index = 0; index < nelements; ++index)
     temp_data.int_buffer[index] = pixel_data[index].region_label;
   io_file_fs.write(reinterpret_cast<char *>(temp_data.int_buffer),4*nelements);
   switch(params.dtype)
   { 
     case UInt8:   for (index = 0; index < nelements; ++index)
                     for (band = 0; band < params.nbands; ++band)
                       temp_data.byte_buffer[index + band*nelements] = pixel_data[index].byte_input_data[band];
                   io_file_fs.write(reinterpret_cast<char *>(temp_data.byte_buffer),nelements*params.nbands);
                   break;
     case UInt16:  for (index = 0; index < nelements; ++index)
                     for (band = 0; band < params.nbands; ++band)
                       temp_data.short_buffer[index + band*nelements] = pixel_data[index].short_input_data[band];
                   io_file_fs.write(reinterpret_cast<char *>(temp_data.short_buffer),2*nelements*params.nbands);
                   break;
     case Float32: for (index = 0; index < nelements; ++index)
                     for (band = 0; band < params.nbands; ++band)
                       temp_data.float_buffer[index + band*nelements] = pixel_data[index].float_input_data[band];
                   io_file_fs.write(reinterpret_cast<char *>(temp_data.float_buffer),4*nelements*params.nbands);
                   break;
     default:      cout << "RHSEG style data type is invalid (p7)" << endl;
                   break;
   }
   if (params.std_dev_image_flag)
   {
     for (index = 0; index < nelements; ++index)
       for (band = 0; band < params.nbands; ++band)
         temp_data.float_buffer[index + band*nelements] = pixel_data[index].local_std_dev[band];
     io_file_fs.write(reinterpret_cast<char *>(temp_data.float_buffer),4*nelements*params.nbands);
     for (index = 0; index < nelements; ++index)
       if (pixel_data[index].std_dev_mask)
         temp_data.byte_buffer[index] = 1;
       else
         temp_data.byte_buffer[index] = 0;
     io_file_fs.write(reinterpret_cast<char *>(temp_data.byte_buffer),nelements);
   }                   
   if (params.edge_image_flag)
   {
     for (index = 0; index < nelements; ++index)
       temp_data.float_buffer[index] = pixel_data[index].edge_value;
     io_file_fs.write(reinterpret_cast<char *>(temp_data.float_buffer),4*nelements);
     for (index = 0; index < nelements; ++index)
       if (pixel_data[index].edge_mask)
         temp_data.byte_buffer[index] = 1;
       else
         temp_data.byte_buffer[index] = 0;
     io_file_fs.write(reinterpret_cast<char *>(temp_data.byte_buffer),nelements);
   }

   io_file_fs.close( );

   return;
 }

 void restore_pixel_data(const short unsigned int& section,
                         vector<Pixel>& pixel_data, Temp& temp_data)
 {
   if (params.debug > 3)
     params.log_fs << "Restoring pixel_data for section " << section << endl;

   char *char_section;
   char_section = (char *) malloc(4*sizeof(char));

   string temp_file_name;
   temp_file_name = params.temp_file_name + ".pixel_sec";

   string io_file_name;
   sprintf(char_section,"%03d",section);
   io_file_name = temp_file_name + char_section;

   fstream io_file_fs;
   io_file_fs.open(io_file_name.c_str( ), ios_base::in | ios_base::binary );

   int index, band;
#ifdef THREEDIM
   int nelements = params.ionb_ncols*params.ionb_nrows*params.ionb_nslices;
#else
   int nelements = params.ionb_ncols*params.ionb_nrows;
#endif
   int byte_nelements, short_nelements, int_nelements, float_nelements;
   byte_nelements = short_nelements = int_nelements = float_nelements = 0;
   switch(params.dtype)
   {
     case UInt8:   byte_nelements = nelements*params.nbands;
                   short_nelements = 0;
                   float_nelements = 0;
                   if (params.std_dev_image_flag)
                     float_nelements = nelements*params.nbands;
                   else if (params.edge_image_flag)
                     float_nelements = nelements;
                   break;
     case UInt16:  byte_nelements = nelements;
                   short_nelements = nelements*params.nbands;
                   float_nelements = 0;
                   if (params.std_dev_image_flag)
                     float_nelements = nelements*params.nbands;
                   else if (params.edge_image_flag)
                     float_nelements = nelements;
                   break;
     case Float32: byte_nelements = nelements;
                   short_nelements = 0;
                   float_nelements = nelements*params.nbands;
                   break;
     default:      cout << "RHSEG style data type is invalid (p6)" << endl;
                   break;
   }
   int_nelements = nelements;
   check_buf_size(byte_nelements,short_nelements,int_nelements,float_nelements,0,temp_data);

   io_file_fs.read(reinterpret_cast<char *>(temp_data.byte_buffer),nelements);
   for (index = 0; index < nelements; ++index)
     pixel_data[index].init_flag = (temp_data.byte_buffer[index] == 1);
   if ((params.mask_flag || params.padded_flag))
   {
     io_file_fs.read(reinterpret_cast<char *>(temp_data.byte_buffer),nelements);
     for (index = 0; index < nelements; ++index)
       pixel_data[index].mask = (temp_data.byte_buffer[index] == 1);
   }
   io_file_fs.read(reinterpret_cast<char *>(temp_data.int_buffer),4*nelements);
   for (index = 0; index < nelements; ++index)
     pixel_data[index].region_label = temp_data.int_buffer[index];
   switch(params.dtype)
   { 
     case UInt8:   io_file_fs.read(reinterpret_cast<char *>(temp_data.byte_buffer),nelements*params.nbands);
                   for (index = 0; index < nelements; ++index)
                     for (band = 0; band < params.nbands; ++band)
                       pixel_data[index].byte_input_data[band] = temp_data.byte_buffer[index + band*nelements];
                   break;
     case UInt16:  io_file_fs.read(reinterpret_cast<char *>(temp_data.short_buffer),2*nelements*params.nbands);
                   for (index = 0; index < nelements; ++index)
                     for (band = 0; band < params.nbands; ++band)
                       pixel_data[index].short_input_data[band] = temp_data.short_buffer[index + band*nelements];
                   break;
     case Float32: io_file_fs.read(reinterpret_cast<char *>(temp_data.float_buffer),4*nelements*params.nbands);
                   for (index = 0; index < nelements; ++index)
                     for (band = 0; band < params.nbands; ++band)
                       pixel_data[index].float_input_data[band] = temp_data.float_buffer[index + band*nelements];
                   break;
     default:      cout << "RHSEG style data type is invalid (p9)" << endl;
                   break;
   }
   if (params.std_dev_image_flag)
   {
     io_file_fs.read(reinterpret_cast<char *>(temp_data.float_buffer),4*nelements*params.nbands);
     for (index = 0; index < nelements; ++index)
       for (band = 0; band < params.nbands; ++band)
         pixel_data[index].local_std_dev[band] = temp_data.float_buffer[index + band*nelements];
     io_file_fs.read(reinterpret_cast<char *>(temp_data.byte_buffer),nelements);
     for (index = 0; index < nelements; ++index)
       pixel_data[index].std_dev_mask = (temp_data.byte_buffer[index] == 1);
   }
   if (params.edge_image_flag)
   {
     io_file_fs.read(reinterpret_cast<char *>(temp_data.float_buffer),4*nelements);
     for (index = 0; index < nelements; ++index)
     {
       pixel_data[index].edge_value = temp_data.float_buffer[index];
     }
     io_file_fs.read(reinterpret_cast<char *>(temp_data.byte_buffer),nelements);
     for (index = 0; index < nelements; ++index)
       pixel_data[index].edge_mask = (temp_data.byte_buffer[index] == 1);
   }

   io_file_fs.close( );

   return;
 }
#endif
} // namespace HSEGTilton
