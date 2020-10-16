/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the RegionObject class, which is a class dealing
   >>>>                 with region object data
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  November 18, 2003
   >>>>
   >>>> Modifications:  August 8, 2005 - Added the (static) sumsq_flag member variable.
   >>>>                 January 27, 2006 - Eliminated the merge_nghbr_set member variable.
   >>>>                 September 7, 2006 - Added the (static) sumxlogx_flag member variable.
   >>>>                 November 15, 2007 - Modified to work with hseg_read program.
   >>>>                 November 27, 2007 - Added class_nghbrs_set and object_nghbrs_set member variables.
   >>>>                 May 12, 2008 - Modified to work with new Params object class
   >>>>			February 8, 2013 - Replaced the std_dev member variable with sum_pixel_std_dev as part of the expansion
   >>>>					   of the use of the standard deviation spatial feature.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#ifndef REGIONOBJECT_H
#define REGIONOBJECT_H

#include <defines.h>
#include <string>
#include <vector>
#include <set>
#include <float.h>

using namespace std;

namespace HSEGTilton
{

  class RegionClass;
  class Pixel;
  class Spatial;
  class Index;
  struct Temp;

  class RegionObject
  {
    public:
      //  CONSTRUCTORS and DESTRUCTOR
      RegionObject();
      RegionObject(const RegionObject& source);
      ~RegionObject();
      //  MODIFICATION MEMBER FUNCTIONS
      static void set_static_vals();
      static void set_static_vals(const bool& classes_flag, const bool& objects_flag);
      void operator =(const RegionObject& source);
      void operator +=(const RegionObject& source);
      void clear();
      void clear_region_object_info();
      void set_active_flag(const bool& value)
           { active_flag = value; }
      void set_label(const unsigned int& value)
           { label = value; }
      void set_label_active(const unsigned int& value)
           { label = value; active_flag = true; }
      void set_boundary_npix(const unsigned int& value)
           { boundary_npix = value; }
      void set_merge_region_label(const unsigned int& value)
           { merge_region_label = value; }
      void increment_boundary_npix()
           { boundary_npix++ ; }
      void set_min_region_dissim();
#ifdef RHSEG_RUN
      void init(Pixel *this_pixel);
#endif
//      void calc_std_dev();
      void set_band_max_std_dev();
      void merge_regions(RegionObject *merge_region, vector<RegionObject>& region_objects);
      void class_nghbrs_set_copy(const RegionObject& source);
      void object_nghbrs_set_copy(const RegionObject& source);
      void class_nghbrs_set_clear()
           { class_nghbrs_set.clear(); }
      void object_nghbrs_set_clear()
           { object_nghbrs_set.clear(); }
#ifdef RHSEG_READ
#ifdef THREEDIM
      void object_nghbrs_set_init(const short unsigned int& hlevel, Spatial& spatial_data,
                                vector<RegionClass>& region_classes, vector<RegionObject>& region_objects, 
                                const int& col, const int& row, const int& slice);
#else
      void object_nghbrs_set_init(const short unsigned int& hlevel, Spatial& spatial_data,
                                vector<RegionClass>& region_classes, vector<RegionObject>& region_objects, 
                                const int& col, const int& row);
#endif
#endif // RHSEG_READ
      //  CONSTANT MEMBER FUNCTIONS
      bool get_active_flag() const { return active_flag; }
      unsigned int get_label() const { return label; }
      unsigned int get_npix() const { return npix; }
      double get_sum(int index) const { return sum[index]; }
      double get_unscaled_mean(int band) const;
      double get_unscaled_std_dev(int band) const;
      double get_std_dev(int band) const;
      double get_band_max_std_dev() const;
      unsigned int get_merge_region_label() const
                   { return merge_region_label; }
      unsigned int get_boundary_npix( ) const { return boundary_npix; }
      void print(vector<RegionObject>& region_objects);
      void print(string& strMessage);
      // FRIEND FUNCTIONS and CLASSES
      friend double calc_region_object_dissim(RegionObject *region_object1, RegionObject *region_object2);
#ifdef THREEDIM
      friend void connected_component(const int& ncols, const int& nrows, const int& nslices,
                                      const short unsigned int& nslevels, vector<RegionObject>& region_objects,
                                      Spatial& spatial_data);
      friend void connected_component_seams(const short unsigned int& recur_level, const short unsigned int& section,
                                            const short unsigned int& nb_sections,  const unsigned char& seam_flag,
                                            const short unsigned int& nslevels,
                                            const int& ncols, const int& nrows, const int& nslices,
                                            vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                                            vector<Index>& slice_seam_index_data, vector<RegionObject>& region_objects,
                                            Spatial& spatial_data, Temp& temp_data);
#else
      friend void connected_component(const int& ncols, const int& nrows,
                                      const short unsigned int& nslevels, vector<RegionObject>& region_objects,
                                      Spatial& spatial_data);
      friend void connected_component_seams(const short unsigned int& recur_level, const short unsigned int& section,
                                            const short unsigned int& nb_sections, const unsigned char& seam_flag,
                                            const short unsigned int& nslevels,
                                            const int& ncols, const int& nrows,
                                            vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                                            vector<RegionObject>& region_objects,
                                            Spatial& spatial_data, Temp& temp_data);
#endif
      friend class Results;
      friend class Spatial;
      friend class ObjectDistToMin;
      friend class ObjectNpixMoreThan;
      friend class ObjectStdDevLessThan;
      friend class ObjectBPRatioLessThan;
    private:
      //  PRIVATE DATA
      bool               active_flag;        // true if active, false if not-active or merged into another region
      unsigned int       label;              // Label of this connected region
      unsigned int       npix;               // Number of pixels in this connected region
      double             *sum;               // Sum of image values (in each band)
      double             *sumsq;             // Sum of squares of image values (in each band)
      double             *sumxlogx;          // Sum of xlogx of image values x (in each band)
      double             *sum_pixel_std_dev; // Sum of the local pixel standard deviation feature value (in each band)
      double		 band_max_std_dev;   // Band maximum standard deviation for this region
      set<unsigned int>  class_nghbrs_set;   // Set of labels of the neighboring region classes for this region object
      set<unsigned int>  object_nghbrs_set;  // Set of labels of the neighboring region objects for this region object
      unsigned int       merge_region_label; // Label of region into which this region was merged (= 0 if not merged)
      double             min_region_dissim;  // Dissimilarity of region to minimum vector
      unsigned int       boundary_npix;      // Number of boundary pixels in the region.
      //  PRIVATE STATIC DATA
      static bool    sumsq_flag;        // true iff the sumsq feature is to be calculated.
      static bool    sumxlogx_flag;     // true iff the sumxlogx feature is to be calculated.
      static bool    std_dev_flag;      // true iff the spatial (std_dev) feature is to be used.
      static bool    region_class_nghbrs_list_flag; // Used in rhseg_read
      static bool    region_object_nghbrs_list_flag; // Used in rhseg_read
      static int     nbands;            // Number of spectral bands in the input image data.
      static double  *scale, *offset;   // Scale and offset values used in input image data scaling and normalization
      static double  *minval;           // Minimum value in each band (after normalization)
      static double  *meanval;          // Mean value in each band (after normalization)
#ifdef PARALLEL
#include <parallel/region_object.h>
#endif
  } ;

  class ObjectDistToMin
  {
    public:
      bool operator () (const RegionObject& region_object1, const RegionObject& region_object2) const
      { if ((region_object1.min_region_dissim == region_object2.min_region_dissim) && (region_object1.npix == region_object2.npix))
          return (region_object1.label < region_object2.label);
        else if (region_object1.min_region_dissim == region_object2.min_region_dissim)
          return (region_object1.npix > region_object2.npix);
        else
          return (region_object1.min_region_dissim < region_object2.min_region_dissim); }
  };

  // Comparison criteria for object_heap
  class ObjectNpixMoreThan
  {
     public:
        bool operator ( ) (const RegionObject *region1, const RegionObject *region2) const
        { if (region1->npix == region2->npix)
            return (region1->label > region2->label);
          else
            return (region1->npix < region2->npix); }
  };

  class ObjectStdDevLessThan
  {
     public:
        bool operator ( ) (const RegionObject *region1, const RegionObject *region2) const
        { 
          float std_dev1 = region1->get_band_max_std_dev();
          float std_dev2 = region2->get_band_max_std_dev();
          if (region1->npix < MIN_STD_DEV_NPIX)
            std_dev1 = FLT_MAX;
          if (region2->npix < MIN_STD_DEV_NPIX)
            std_dev2 = FLT_MAX;
          if (std_dev1 == std_dev2)
            return (region1->label > region2->label);
          else
            return (std_dev1 > std_dev2);
        }
  };

  class ObjectBPRatioLessThan
  {
     public:
        bool operator ( ) (const RegionObject *region1, const RegionObject *region2) const
        { 
          float bp_ratio1 = ((float) region1->boundary_npix)/((float) region1->npix);
          float bp_ratio2 = ((float) region2->boundary_npix)/((float) region2->npix);
          if (bp_ratio1 == bp_ratio2)
            return (region1->label > region2->label);
          else
            return (bp_ratio1 > bp_ratio2); }
  };

  // Other related functions
#ifdef THREEDIM
  void connected_component_init(const int& ncols, const int& nrows, const int& nslices, unsigned int& nb_objects,
                                vector<RegionObject>& region_objects, Spatial& spatial_data);

  void connected_component_init(const short unsigned int& recur_level, const short unsigned int& section,
                                const int& ncols, const int& nrows, const int& nslices,
                                unsigned int& nb_objects, vector<RegionObject>& region_objects,
                                Spatial& spatial_data, Temp& temp_data);

  void connected_component_seams(const short unsigned int& recur_level, const short unsigned int& section, 
                                 const short unsigned int& nb_sections,  const unsigned char& seam_flag,
                                 const short unsigned int& nslevels,
                                 const int& ncols, const int& nrows, const int& nslices,
                                 vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                                 vector<Index>& slice_seam_index_data, vector<RegionObject>& region_objects,
                                 Spatial& spatial_data, Temp& temp_data);

  void connected_component(const int& ncols, const int& nrows, const int& nslices,
                           const short unsigned int& nslevels, vector<RegionObject>& region_objects,
                           Spatial& spatial_data);

  void connected_component(const short unsigned int& recur_level, const short unsigned int& section,
                           const int& ncols, const int& nrows, const int& nslices, const short unsigned int& nslevels,
                           vector<RegionObject>& region_objects, Spatial& spatial_data,
                           Temp& temp_data);

  void do_region_objects_init(const int& ncols, const int& nrows, const int& nslices,
                                vector<Pixel>& pixel_data, Spatial& spatial_data,
                                vector<RegionObject>& region_objects);

  unsigned int do_region_objects_init(const short unsigned int& recur_level, const short unsigned int& section, 
                                        const int& ncols, const int& nrows, const int& nslices, vector<Pixel>& pixel_data,
                                        Spatial& spatial_data, vector<RegionObject>& region_objects,
                                        Temp& temp_data);

  void update_region_object_info(const int& ncols, const int& nrows,
                               const int& nslices, const short unsigned int& nslevels,
                               Spatial& spatial_data, vector<RegionObject>& region_objects);
#else
  void connected_component_init(const int& ncols, const int& nrows, unsigned int& nb_objects,
                                vector<RegionObject>& region_objects, Spatial& spatial_data);

  void connected_component_init(const short unsigned int& recur_level, const short unsigned int& section,
                                const int& ncols, const int& nrows,
                                unsigned int& nb_objects, vector<RegionObject>& region_objects,
                                Spatial& spatial_data, Temp& temp_data);

  void connected_component_seams(const short unsigned int& recur_level, const short unsigned int& section,
                                 const short unsigned int& nb_sections, const unsigned char& seam_flag,
                                 const short unsigned int& nslevels,
                                 const int& ncols, const int& nrows,
                                 vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                                 vector<RegionObject>& region_objects,
                                 Spatial& spatial_data, Temp& temp_data);

  void connected_component(const int& ncols, const int& nrows,
                           const short unsigned int& nslevels, vector<RegionObject>& region_objects,
                           Spatial& spatial_data);

  void connected_component(const short unsigned int& recur_level, const short unsigned int& section,
                           const int& ncols, const int& nrows, const short unsigned int& nslevels,
                           vector<RegionObject>& region_objects, Spatial& spatial_data,
                           Temp& temp_data);

  void do_region_objects_init(const int& ncols, const int& nrows,
                                vector<Pixel>& pixel_data, Spatial& spatial_data,
                                vector<RegionObject>& region_objects);

  unsigned int do_region_objects_init(const short unsigned int& recur_level, const short unsigned int& section,
                                        const int& ncols, const int& nrows, vector<Pixel>& pixel_data,
                                        Spatial& spatial_data, vector<RegionObject>& region_objects,
                                        Temp& temp_data);

  void update_region_object_info(const int& ncols, const int& nrows,
                               const short unsigned int& nslevels, Spatial& spatial_data,
                               vector<RegionObject>& region_objects);
#endif
  void update_region_object_info(const short unsigned int& recur_level,
                               const short unsigned int& section, const short unsigned int& nslevels,
                               Spatial& spatial_data,  vector<RegionObject>& region_objects,
                               Temp& temp_data);
} // HSEGTilton

#endif /* REGIONOBJECT_H */
