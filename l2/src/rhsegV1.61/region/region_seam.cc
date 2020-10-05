/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>          File:  region_seam.cc
   >>>>
   >>>>          See region_seam.h for documentation
   >>>>
   >>>>          Date:  March 27, 2014
   >>>>
   >>>> Modifications:  
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "region_seam.h"
#include <params/params.h>
#include <iostream>

extern HSEGTilton::Params params;

namespace HSEGTilton
{

  RegionSeam::RegionSeam()
  {
    npix = 0;
    sum_edge = 0.0;

    return;
  }

  RegionSeam::RegionSeam(const RegionSeam& source)
  {

  // Copy member variables
    npix = source.npix;
    sum_edge = source.sum_edge;

    return;
  }

  RegionSeam::RegionSeam(const unsigned int& npix_value, const float& sum_edge_value)
  {

  // Set member variables
    npix = npix_value;
    sum_edge = sum_edge_value;

    return;
  }

  RegionSeam::~RegionSeam()
  {
    return;
  }

  void RegionSeam::operator =(const RegionSeam& source)
  {

    if (this == &source)
      return;

  // Copy member variables
    npix = source.npix;
    sum_edge = source.sum_edge;

    return;
  }

  void RegionSeam::operator +=(const RegionSeam& source)
  {

  // Add member variables
    npix += source.npix;
    sum_edge += source.sum_edge;

    return;
  }

  void RegionSeam::clear()
  {

    npix = 0;
    sum_edge = 0.0;

    return;
  }

  void RegionSeam::print()
  {
    params.log_fs << "seam_edge_npix = " << npix << " and sum_seam_edge_value = " << sum_edge << endl;

    return;
  }

} // namespace HSEGTilton
