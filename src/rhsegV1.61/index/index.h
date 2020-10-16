/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Index class.  This class represents
   >>>>                 indices for itself, pixel data, taskid data and region data
   >>>>                 for selected portions of the spatial extent of the data
   >>>>                 processed for a given processing node.
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  September 13, 2002
   >>>>
   >>>> Modifications:  February 7, 2003 - Changed region_index to region_class_label
   >>>>                 June 17, 2003 - Eliminated member variable index_index
   >>>>                 October 6, 2003 - Added nghbr_region_label_set and region_object_label member variables
   >>>>                 October 18, 2003 - Added boundary_flag member variable
   >>>>                 March 3, 2004 - Changed boundary_flag to boundary_map
   >>>>                 December 22, 2004 - Changed region label from short unsigned int to unsigned int
   >>>>                 May 12, 2008 - Modified to work with new Params object class
   >>>>                 April 25, 2011 - Added seam/border_flag to get_seam_index_data and get_border_index_data to certain calls:
   >>>>                 1 => col only, 2 => row only, 3=> col & row, 4=> slice only, 5=> col & slice, 6 => row & slice, 7 => col, row & slice
   >>>>                 August 5, 2013 - Added edge_value and edge_location member variables
   >>>>                 September 16, 2013 - Removed the compare_across_seam functions.
   >>>>                 January 8, 2014 - Removed the edge_location member variable
   >>>>                 January 18, 2014 - Added edge_mask member variable.
   >>>>                 January 18, 2014 - Renamed section to pixel_section.
   >>>>                 January 18, 2014 - Added explicit constructors and destructor.
   >>>>                 January 18, 2014 - Removed nghbr_region_label_set (not used!).
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#ifndef INDEX_H
#define INDEX_H

#include <defines.h>
#include <rhseg/hseg.h>
#include <pixel/pixel.h>
#include <fstream>

using namespace std;

namespace HSEGTilton
{
    class Params;
    class Pixel;
    class Spatial;
    class RegionClass;

    class Index
    {
      public:
         //  CONSTRUCTORS and DESTRUCTOR
          Index( );
          Index(const Index& source);
          ~Index( );

         //  MODIFICATION MEMBER FUNCTIONS
          void operator =(const Index& source);
          void clear( );
          void set_pixel_index(const unsigned int& value)
               { pixel_index = value; }
          void set_region_class_label(const unsigned int& value)
               { region_class_label = value; }
          void set_region_object_label(const unsigned int& value)
               { region_object_label = value; }
          void set_boundary_map(const short unsigned int& value)
               { boundary_map = value; }
          void set_data(const short unsigned int& section_val,
                        vector<Pixel>& pixel_data, const unsigned int& index);
          void set_data(const short unsigned int& section_val,
                        Spatial& spatial_data, const unsigned int& index);
         //  CONSTANT MEMBER FUNCTIONS
          unsigned int  get_pixel_index( ) const { return pixel_index; }
          short unsigned int get_section( ) const { return pixel_section; }
          bool get_edge_mask( ) const { return edge_mask; }
          float get_edge_value( ) const { return edge_value; }
          unsigned int  get_region_class_label( ) const { return region_class_label; }
          unsigned int  get_region_object_label( ) const { return region_object_label; }
          short unsigned int get_boundary_map ( ) const { return boundary_map; }
          void print();
         // FRIEND FUNCTIONS and CLASSES
          friend class RegionClass;
        // PRIVATE DATA
      private:
          unsigned int         pixel_index;         // Index of associated pixel data
          short unsigned int   pixel_section;       // Processing section of associated pixel data
          bool                 edge_mask;           // Edge mask - true iff edge_value is valid.
          float                edge_value;          // Edge value for neighborhood of associated pixel data
          unsigned int         region_class_label;  // Label of region class to which this data element belongs
          unsigned int         region_object_label; // Label of the region object to which this data element belongs
          short unsigned int   boundary_map;        // Value is last hierarchical level pixel is on the boundary of a region.
      //  PRIVATE STATIC DATA

#ifdef PARALLEL
#include <parallel/index.h>
#endif
  };

  // Other related functions
#ifdef THREEDIM
  void get_seam_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                           const unsigned char& seam_flag, vector<Pixel>& pixel_data, 
                           const int& ncols, const int& nrows, const int& nslices,
                           vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                           vector<Index>& slice_seam_index_data);

  void get_seam_index_data(const short unsigned int& recur_level, const short unsigned int& section, 
                           const unsigned char& seam_flag, vector<Pixel>& pixel_data, 
                           const int& ncols, const int& nrows, const int& nslices,
                           vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                           vector<Index>& slice_seam_index_data, Temp& temp_data);

  void get_seam_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                           const unsigned char& seam_flag, Spatial& spatial_data, 
                           const int& ncols, const int& nrows, const int& nslices,
                           vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                           vector<Index>& slice_seam_index_data);

  void get_seam_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                           const unsigned char& seam_flag, Spatial& spatial_data, 
                           const int& ncols, const int& nrows, const int& nslices,
                           vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                           vector<Index>& slice_seam_index_data, Temp& temp_data);

  void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                             const unsigned char& border_flag, vector<Pixel>& pixel_data, 
                             const int& ncols, const int& nrows, const int& nslices,
                             vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                             vector<Index>& slice_border_index_data);

  void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                             const unsigned char& border_flag, vector<Pixel>& pixel_data, 
                             const int& ncols, const int& nrows, const int& nslices,
                             vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                             vector<Index>& slice_border_index_data, Temp& temp_data);

  void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                             const unsigned char& border_flag, Spatial& spatial_data,
                             const int& ncols, const int& nrows, const int& nslices,
                             vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                             vector<Index>& slice_border_index_data);

  void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                             const unsigned char& border_flag, Spatial& spatial_data,
                             const int& ncols, const int& nrows, const int& nslices,
                             vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                             vector<Index>& slice_border_index_data, Temp& temp_data);
#else
  void get_seam_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                           const unsigned char& seam_flag, vector<Pixel>& pixel_data,
                           const int& ncols, const int& nrows,
                           vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data);

  void get_seam_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                           const unsigned char& seam_flag, vector<Pixel>& pixel_data, 
                           const int& ncols, const int& nrows,
                           vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                           Temp& temp_data);

  void get_seam_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                           const unsigned char& seam_flag, Spatial& spatial_data,
                           const int& ncols, const int& nrows,
                           vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data);

  void get_seam_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                           const unsigned char& seam_flag, Spatial& spatial_data,
                           const int& ncols, const int& nrows,
                           vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                           Temp& temp_data);

  void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                             const unsigned char& border_flag, vector<Pixel>& pixel_data,
                             const int& ncols, const int& nrows,
                             vector<Index>& col_border_index_data, vector<Index>& row_border_index_data);

  void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                             const unsigned char& border_flag, vector<Pixel>& pixel_data, 
                             const int& ncols, const int& nrows,
                             vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                             Temp& temp_data);

  void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                             const unsigned char& border_flag, Spatial& spatial_data,
                             const int& ncols, const int& nrows,
                             vector<Index>& col_border_index_data, vector<Index>& row_border_index_data);

  void get_border_index_data(const short unsigned int& recur_level, const short unsigned int& section,
                             const unsigned char& border_flag, Spatial& spatial_data,
                             const int& ncols, const int& nrows,
                             vector<Index>& col_border_index_data, vector<Index>& row_border_index_data,
                             Temp& temp_data);
#endif
} // HSEGTilton

#endif /* INDEX_H */

