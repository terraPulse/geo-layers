/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Pixel class, which is a class dealing
   >>>>                 with pixel data.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  December 9, 2002
   >>>>
   >>>> Modifications:  February 7, 2003 - Changed region_index to region_label
   >>>>                 June 16, 2003 - Made std_dev_flag and nbands static
   >>>>                 June 17, 2003 - Eliminated flag member variable
   >>>>                 December 22, 2004 - Changed region label from short unsigned int to unsigned int
   >>>>                 May 31, 2005 - Added temporary file usage for handling large files
   >>>>                 August 8, 2005 - Added static sumsq_flag
   >>>>                 October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
   >>>>                 September 7, 2006 - Added static sumxlog_flag
   >>>>                 February 14, 2007 - Added init_flag
   >>>>                 March 12, 2007 - Eliminated sum, sumsq and sumxlogx members (redundant), and replaced them
   >>>>                                  with mask and input_data members.
   >>>>                 May 12, 2008 - Modified to work with new Params object class
   >>>>                 September 5, 2008 - Modified scale_offset to return number of image pixels.
   >>>>                 May 12, 2009 - Upgraded to work with float input image data.
   >>>>                 April 16, 2013 - Added the related Location class
   >>>>                 May 15, 2013 - Added location member variable to Pixel class
   >>>>                 September 16, 2013 - Removed the split_pixels functions.
   >>>>                 September 18, 2013 - Removed split_flag.
   >>>>                 January 8, 2014 - Removed the Location class
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#ifndef PIXEL_H
#define PIXEL_H

#include <defines.h>
#include <rhseg/hseg.h>
#ifdef GDAL
#include <image/image.h>
#endif
#ifndef IMAGE_H
  enum RHSEGDType { Unknown, UInt8, UInt16, UInt32, Float32 };
#define IMAGE_H
#endif
#include <map>

using namespace std;
#ifdef GDAL
using namespace CommonTilton;
#endif

namespace HSEGTilton
{

  class RegionClass;
  class Spatial;

  class Pixel
  {
    public:
    //  CONSTRUCTORS and DESTRUCTOR
      Pixel( );
      Pixel(const Pixel& source);
      ~Pixel( );

    //  MODIFICATION MEMBER FUNCTIONS
      static void set_static_vals();
      void operator =(const Pixel& source);
      void clear();
      void set_init_flag(const bool& value)
           { init_flag = value; }
      void set_mask(const bool& value)
           { mask = value; }
      void set_region_label(const unsigned int& value)
           { region_label = value; }
      void set_input_data(const int& band, const unsigned char & value)
           { byte_input_data[band] = value; }
      void set_input_data(const int& band, const short unsigned int& value)
           { short_input_data[band] = value; }
      void set_input_data(const int& band, const float& value);
      void set_local_std_dev(const int& band, const float& value)
           { local_std_dev[band] = value; }
      void set_edge_value(const float& value)
           { edge_value = value; }

    //  CONSTANT MEMBER FUNCTIONS
      bool  get_init_flag( ) const { return init_flag; }
      bool  get_mask( ) const { return mask; }
      unsigned int get_region_label( );
      float get_input_data(const int& band);
      bool  get_std_dev_mask( ) const { return std_dev_mask; }
      float get_local_std_dev(const int& band);
      bool  get_edge_mask( ) const { return edge_mask; }
      float get_edge_value( );
      double update_pixel_dissim(RegionClass *dissim_region);
      void print(const unsigned int& pixel_index);
//#ifdef DEBUG
      void print_region_label();
//#endif
    // FRIEND FUNCTIONS and CLASSES
      friend unsigned int scale_offset(vector<Pixel>& pixel_data, Temp& temp_data);
      friend float calc_edge_pixel_dissim(RegionClass *region, Pixel *pixel);
      friend double calc_region_pixel_dissim(RegionClass *region, Pixel *pixel);
#ifndef PARALLEL
      friend void save_pixel_data(const short unsigned int& section,
                                  vector<Pixel>& pixel_data, Temp& temp_data);
      friend void restore_pixel_data(const short unsigned int& section,
                                     vector<Pixel>& pixel_data, Temp& temp_data);
#endif
      friend class Spatial;
      friend class RegionClass;
      friend class RegionObject;
      friend class Index;

    private:
    //  PRIVATE DATA
      bool         init_flag;     // True if region initialization complete for this pixel
      bool         mask;          // Input data mask - true iff the input data values are valid
      unsigned int region_label;  // Label of the region this pixel belongs to
      unsigned char  *byte_input_data;    // Input data values
      unsigned short *short_input_data;   // Input data values
      float          *float_input_data;   // Input data values
      bool           std_dev_mask;	  // Standard deviation mask - true iff local_std_dev values are valid.
      float	     *local_std_dev;      // Minimum standard deviation of pixels in 3x3 neighborhoods including this pixel.
      bool           edge_mask;           // Edge mask - true iff edge_value is valid.
      float	     edge_value;          // Edge value for neighborhood of this pixel.

    //  PRIVATE STATIC DATA
      static int        nbands;           // Number of spectral bands in the input image data.
      static RHSEGDType dtype;            // Data type of the input image data.
      static bool       std_dev_flag;     // true iff the spatial (std_dev) feature is to be used.

#ifdef PARALLEL
#include <parallel/pixel.h>
#endif
  };

  // Other related functions
  void do_region_class_relabel(map<unsigned int,unsigned int>& region_class_relabel_pairs,
                               vector<Pixel>& pixel_data);

  void do_region_class_relabel(const short unsigned int& recur_level, const short unsigned int& section,
                               map<unsigned int,unsigned int>& region_class_relabel_pairs,
                               vector<Pixel>& pixel_data,
                               Temp& temp_data);
#ifdef THREEDIM
  void do_region_class_relabel(map<unsigned int,unsigned int>& region_class_relabel_pairs,
                               const int& ncols, const int& nrows, const int& nslices, 
                               vector<Index>& seam_index_data);

  void do_region_object_relabel(map<unsigned int,unsigned int>& region_object_relabel_pairs,
                                Spatial& spatial_data, const int& ncols, const int& nrows, const int& nslices);

  void do_region_object_relabel(const short unsigned int& recur_level, const short unsigned int& section,
                                map<unsigned int,unsigned int>& region_object_relabel_pairs,
                                Spatial& spatial_data, const int& ncols, const int& nrows, const int& nslices,
                                Temp& temp_data);
#else
  void do_region_class_relabel(map<unsigned int,unsigned int>& region_class_relabel_pairs,
                               const int& ncols, const int& nrows,
                               vector<Index>& seam_index_data);

  void do_region_object_relabel(map<unsigned int,unsigned int>& region_object_relabel_pairs,
                                Spatial& spatial_data, const int& ncols, const int& nrows);

  void do_region_object_relabel(const short unsigned int& recur_level, const short unsigned int& section,
                                map<unsigned int,unsigned int>& region_object_relabel_pairs,
                                Spatial& spatial_data, const int& ncols, const int& nrows,
                                Temp& temp_data);
#endif

} // HSEGTilton

#endif /* PIXEL_H */
