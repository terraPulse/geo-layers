/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the RegionClass class, which is a class dealing
   >>>>                 with region data
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  December 10, 2002
   >>>>
   >>>> Modifications:  February 7, 2003 - Changed index to label
   >>>>                 May 30, 2003 - Added min_npixels_flag to update_best_nghbr()
   >>>>                 June 4, 2003: Changed list<short unsigned int>nghbrs_label_list to
   >>>>                               set<short unsigned int>nghbrs_label_set for efficiency
   >>>>                 June 9, 2003:  Changed other lists to sets for efficiency
   >>>>                 June 16, 2003 - Made std_dev_flag and nbands static
   >>>>                 August 21, 2003 - Reorganized program and added user controlled byteswapping
   >>>>                 September 29, 2003 - Added subregion_set and boundary_npix member variables
   >>>>                 October 1, 2003 - Added renumber function to renumber nghbrs_label_set
   >>>>                 November 5, 2003 - Made all member variables private and added accessor routines.
   >>>>                 November 7, 2003 - Added load_data function.
   >>>>                 May 25, 2004 - Added max_merge_threshold member variable.
   >>>>                 December 22, 2004 - Changed region label from unsigned short int to unsigned int
   >>>>                 August 9, 2005 - Added the (static) sumsq_flag member variable.
   >>>>                 August 9, 2005 - Changed the max_merge_threshold member variable to conv_crit_value
   >>>>                 August 15, 2005 - Added the conv_ratio member variable.
   >>>>                 August 18, 2005 - Added the merged_flag and one_pixel_flag member variables.
   >>>>                 October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
   >>>>                 November 22, 2005 - Added the col_init, row_init and slice_init functions (for 3D)
   >>>>                 December 23, 2005 - Added the min_region_dissim member variable.
   >>>>                 February 27, 2006 - Added nghbr_heap_npix and region_heap_npix member variables.
   >>>>                 February 28, 2007 - Added fm_init, find_merge, and do_merge functions.
   >>>>                 March 24, 2007 - Moved scale, offset, minval and meanval from spatial object.
   >>>>                 November 1, 2007 - Changed the conv_crit_value member variable to merge_threshold
   >>>>                 November 1, 2007 - Eliminated conv_ratio, one_pixel_flag and conv_criterion member variables
   >>>>                 November 15, 2007 - Modified to work with hseg_read program.
   >>>>                 May 12, 2008 - Modified to work with new Params object class
   >>>>                 October 1, 2010 - Change data type of best_nghbr_dissim and best_region_dissim from double to float
   >>>>                                   to help insure computational consistency between computing platforms and configurations
   >>>>                 January 29, 2011 - Added the large_nghbr_merged_flag.
   >>>>			February 8, 2013 - Replaced the std_dev member variable with sum_pixel_std_dev as part of the expansion
   >>>>					   of the use of the standard deviation spatial feature.
   >>>>                 April 16, 2013 - Added the related RegionEdge class and replaced nghbrs_label_set with nghbr_info_map
   >>>>                 August 9, 2013 - Added the best_edge_nghbr_init function.
   >>>>                 September 6, 2013 - Replaced the best_edge_nghbr_init function with the seam_nghbr_init function
   >>>>                 September 16, 2013 - Removed set_contagious_flags and set_candidate_region_labels functions.
   >>>>                 September 18, 2013 - Removed candidate_region_label_set and related functions.
   >>>>                 September 18, 2013 - Removed load_remerge_data and set_remerge_data functions.
   >>>>                 September 18, 2013 - Removed new_region_flag and contagious_flag.
   >>>>                 November 19, 2013 - Added the region_classes_check and compare_region_classes functions.
   >>>>                 January 8, 2014 - Removed the RegionEdge and Location classes and replaced nghbr_info_map with nghbrs_label_set
   >>>>                 February 3, 2014 - Added seam_nghbr_npix_map, and sum_seam_edge_value for used in artifact elimination
   >>>>                 February 3, 2014 - Removed seam_nghbr_init function
   >>>>                 February 28, 2014 - Added the initial_merge_flag member variable.
   >>>>                 March 5, 2014 - Added seam_flag member variable.
   >>>>                 March 27, 2014 - Added the RegionSeam class.
   >>>>                 March 27, 2014 - Changed seam_nghbr_label_npix_map to nghbrs_label_seam_map and dropped sum_seam_edge_value.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#ifndef REGIONCLASS_H
#define REGIONCLASS_H

#include "../defines.h"
#include <float.h>
#include <string>
#include <vector>
#include <set>
#include <map>

using namespace std;

namespace HSEGTilton
{

  class RegionSeam;
  class RegionObject;
  class Spatial;
  class Pixel;
  class Index;
  struct Temp;

  class RegionClass
  {
    public:
      //  CONSTRUCTORS and DESTRUCTOR
      RegionClass();
      RegionClass(const RegionClass& source);
      ~RegionClass();
      //  MODIFICATION MEMBER FUNCTIONS
      static void set_static_vals();
      void operator =(const RegionClass& source);
      void operator +=(const RegionClass& source);
      void operator -=(const RegionClass& source);
      void clear();
      void partial_clear();
      void nghbrs_label_set_clear()
           { nghbrs_label_set.clear(); }
#ifdef RHSEG_RUN
      void clear_best_nghbr()
           { best_nghbr_label.clear(); best_nghbr_dissim = FLT_MAX; }
      void clear_best_region()
           { best_region_label.clear(); best_region_dissim = FLT_MAX; }
      void clear_best_merge();
#endif // RHSEG_RUN
      void clear_region_info();
      void nghbrs_label_set_copy(const RegionClass& source);
#ifdef RHSEG_RUN
      void best_nghbr_copy(const RegionClass& source);
      void best_region_copy(const RegionClass& source);
      void nghbrs_label_seam_map_copy(const RegionClass& source);
#endif
      void region_objects_set_copy(const RegionClass& source);
      void set_active_flag(const bool& value)
           { active_flag = value; }
      void set_label(const unsigned int& value)
           { label = value; }
      void set_band_max_std_dev();
#ifdef RHSEG_RUN
      void set_npix(const unsigned int& value)
           { npix = value; nghbr_heap_npix = value; region_heap_npix = value; }
      void set_sum_pixel_gdissim(const double& value)
           { sum_pixel_gdissim = value; }
      void set_merge_region_label(const unsigned int& value)
           { merge_region_label = value; }
      void set_min_region_dissim();
      void set_initial_merge_flag(const bool& value)
           { initial_merge_flag = value; }
      void set_seam_flag(const bool& value)
           { seam_flag = value; }
      void set_merged_flag(const bool& value)
           { merged_flag = value; }
      void set_large_nghbr_merged_flag(const bool& value)
           { large_nghbr_merged_flag = value; }
      void set_nghbr_heap_index(const unsigned int& value)
           { nghbr_heap_index = value; }
      void set_region_heap_index(const unsigned int& value)
           { region_heap_index = value; }
      void set_max_edge_value(const float& value)
           { max_edge_value = value; }
      void reset_merged_flag();
#ifdef THREEDIM
      void fm_init(vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                   const int& col, const int& row, const int& slice,
                   const int& ncols, const int& nrows, const int& nslices);
      void do_merge(const int &ncols, const int& nrows, const int& nslices, const unsigned int& nregions,
                    vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, unsigned int& region_classes_size);
#else
      void fm_init(vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                   const int& col, const int& row, const int& ncols, const int& nrows);
      void do_merge(const int &ncols, const int& nrows, const unsigned int& nregions,
                    vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, unsigned int& region_classes_size);
#endif
      bool find_merge(vector<Pixel>& pixel_data);
      void merge_regions(RegionClass *merge_region, vector<RegionClass>& region_classes);
#ifdef THREEDIM
      void init(vector<Pixel>& pixel_data,
                const int& col, const int& row, const int& slice,
                const int& ncols, const int& nrows, const int& nslices);
      void col_init(vector<Index>& col_seam_index_data,
                    const int& col, const int& row, const int& slice,
                    const int& nrows, const int& nslices);
      void row_init(vector<Index>& row_seam_index_data,
                    const int& col, const int& row, const int& slice,
                    const int& ncols, const int& nslices);
      void slice_init(vector<Index>& slice_seam_index_data,
                      const int& col, const int& row, const int& slice,
                      const int& ncols, const int& nrows);
#else
      void init(vector<Pixel>& pixel_data,
                const int& col, const int& row,
                const int& ncols, const int& nrows);
      void col_init(vector<Index>& col_seam_index_data,
                    const int& col, const int& row, const int& nrows);
      void row_init(vector<Index>& row_seam_index_data,
                    const int& col, const int& row, const int& ncols);
#endif
#ifdef THREEDIM
      void nghbrs_label_set_init(vector<Pixel>& pixel_data,
                                 const int& col, const int& row, const int& slice,
                                 const int& ncols, const int& nrows, const int& nslices);
#else
      void nghbrs_label_set_init(vector<Pixel>& pixel_data,
                                 const int& col, const int& row, const int& ncols, const int& nrows);
#endif
      void best_nghbr_init(vector<RegionClass>& region_classes);
      void best_region_init(const unsigned int& heap_index, vector<RegionClass *>& region_heap,
                            const unsigned int& region_heap_size);
      void merge_regions(const bool& last_stage_flag, const double& merge_threshold,
                         RegionClass *merge_region, vector<RegionClass>& region_classes);
      void merge_seam_regions(RegionClass *seam_merge_region, vector<RegionClass>& seam_region_classes);
      void update_best_nghbr(RegionClass *nghbr_region);
      void update_best_region(RegionClass *other_region);
      void update_nghbr_heap(vector<RegionClass *>& nghbr_heap,
                             unsigned int& nghbr_heap_size);
      void nghbr_bring_to_top_heap(vector<RegionClass *>& nghbr_heap,
                                   const unsigned int& nghbr_heap_size);
      void update_region_heap(vector<RegionClass *>& region_heap,
                              unsigned int& region_heap_size);
      void region_bring_to_top_heap(vector<RegionClass *>& region_heap,
                                    const unsigned int& region_heap_size);
      void nghbrs_label_set_renumber(map<unsigned int,unsigned int>& region_relabel_pairs);
      void update_region_class_info(const bool& boundary_flag,
                              const unsigned int& region_object_label);
      void calc_sum_pixel_gdissim();
      void update_sum_pixel_gdissim(Pixel *this_pixel);
#ifdef PARALLEL
      void update_nghbrs_label_set(Temp& temp_data, unsigned int& int_buf_position);
#endif
#endif // RHSEG_RUN

      //  CONSTANT MEMBER FUNCTIONS
      bool get_active_flag() const { return active_flag; }
      double get_scale(int index) const { return scale[index]; }
      double get_offset(int index) const { return offset[index]; }
#ifdef RHSEG_RUN
      bool get_initial_merge_flag() const { return initial_merge_flag; }
      bool get_seam_flag() const { return seam_flag; }
      bool get_merged_flag() const { return merged_flag; }
      bool get_large_nghbr_merged_flag() const { return large_nghbr_merged_flag; }
#endif
      unsigned int get_label() const { return label; }
      unsigned int get_npix() const { return npix; }
#ifdef RHSEG_RUN
      unsigned int get_nghbr_heap_npix() const { return nghbr_heap_npix; }
      unsigned int get_region_heap_npix() const { return region_heap_npix; }
#endif
      double get_sum(int index) const { return sum[index]; }
      double get_sumsq(int index) const { return sumsq[index]; }
      double get_sumxlogx(int index) const { return sumxlogx[index]; }
      double get_sum_pixel_gdissim() const { return sum_pixel_gdissim; }
      double get_min_region_dissim() const { return min_region_dissim; }
#ifdef RHSEG_RUN
      bool   is_nghbr(unsigned int nghbr_label);
      bool   is_best_nghbr(unsigned int nghbr_label);
      bool   is_best_region(unsigned int region_label);
      float  get_best_nghbr_dissim() const { return best_nghbr_dissim; }
      unsigned int get_nghbr_heap_index() const { return nghbr_heap_index; }
      float  get_best_region_dissim() const { return best_region_dissim; }
      unsigned int get_region_heap_index() const { return region_heap_index; }
      int get_best_region_label_set_size() const { return best_region_label.size(); }
#endif // RHSEG_RUN
      unsigned int get_merge_region_label() const { return merge_region_label; }
      int get_region_objects_set_size() const { return region_objects_set.size(); }
      int get_nb_region_objects () const { return nb_region_objects; }
      int get_nghbrs_label_set_size() const { return nghbrs_label_set.size(); }
      unsigned int get_boundary_npix( ) const { return boundary_npix; }
      double get_merge_threshold( ) const { return merge_threshold; }
      set<unsigned int>::iterator get_region_objects_set_begin() const
                                  { return region_objects_set.begin(); }
      set<unsigned int>::iterator get_region_objects_set_end() const
                                  { return region_objects_set.end(); }
      unsigned int get_best_nghbr_label(vector<RegionClass>& region_classes);
      unsigned int get_best_region_label(vector<RegionClass>& region_classes);
      bool check_nghbrs_label_set(vector<RegionClass>& region_classes);
#ifdef RHSEG_RUN
      bool check_best_nghbr_label_empty() const { return best_nghbr_label.empty(); }
#ifdef PARALLEL
      void load_nghbrs_label_set(Temp& temp_data, unsigned int& int_buf_position);
#endif
      void print(vector<RegionClass>& region_classes);
      void seam_print(vector<RegionClass>& seam_region_classes);
      void print(vector<RegionClass>& region_classes, const unsigned int& offset);
      void print();
#endif // RHSEG_RUN
      void print(vector<RegionClass>& region_classes, vector<RegionObject>& region_objects);
      void print(string& strMessage);
      double get_unscaled_mean(int band) const;
      double get_unscaled_std_dev(int band) const;
      double get_std_dev(int band) const;
      double get_band_max_std_dev() const;
      float get_max_edge_value() const { return max_edge_value; }
      // FRIEND FUNCTIONS and CLASSES
      friend RegionClass operator -(const RegionClass& arg1, const RegionClass& arg2);
      friend bool compare_region_classes(const RegionClass& region_class, const RegionClass& check_region_class);
#ifdef RHSEG_RUN
      friend double calc_region_dissim(RegionClass *region1, RegionClass *region2, bool merge_accel_flag);
      friend double calc_region_pixel_dissim(RegionClass *region, Pixel *pixel);
      friend void update_nghbrs(set<RegionClass *>& update_nghbrs_set, vector<RegionClass *>& nghbr_heap,
                                unsigned int& nghbr_heap_size, vector<RegionClass>& region_classes);
      friend void update_regions(set<RegionClass *>& update_regions_set, set<RegionClass *>& added_regions_set, 
                                 set<RegionClass *>& removed_regions_set,
                                 vector<RegionClass *>& region_heap, unsigned int& region_heap_size);
      friend bool nghbr_more_than(const RegionClass *region1, const RegionClass *region2);
      friend bool nghbr_upheap(unsigned int& heap_index, vector<RegionClass *>& nghbr_heap,
                               const unsigned int& nghbr_heap_size);
      friend bool nghbr_downheap(unsigned int& heap_index, vector<RegionClass *>& nghbr_heap,
                                 const unsigned int& nghbr_heap_size);
      friend void make_nghbr_heap(vector<RegionClass *>& nghbr_heap,
                                  const unsigned int& nghbr_heap_size);
      friend void remove_from_nghbr_heap(const unsigned int& remove_heap_index,
                                         vector<RegionClass *>& nghbr_heap, unsigned int& nghbr_heap_size);
      friend bool region_more_than(const RegionClass *region1, const RegionClass *region2);
      friend void make_region_heap(vector<RegionClass *>& region_heap,
                                   const unsigned int& region_heap_size);
      friend void remove_from_region_heap(const unsigned int& remove_heap_index,
                                          vector<RegionClass *>& region_heap, unsigned int& region_heap_size);
      friend void insert_into_region_heap(RegionClass *merge_region,
                                          vector<RegionClass *>& region_heap, unsigned int& region_heap_size);
      friend bool region_upheap(unsigned int& heap_index, vector<RegionClass *>& region_heap,
                                const unsigned int& region_heap_size);
      friend bool region_downheap(unsigned int& heap_index, vector<RegionClass *>& region_heap,
                                  const unsigned int& region_heap_size);
      friend void seam_merge(unsigned int& nregions, vector<RegionClass>& seam_region_classes, 
                             vector<RegionClass>& region_classes, map<unsigned int,unsigned int>& region_class_relabel_pairs);
#ifdef THREEDIM
      friend void seam_region_classes_init(const bool& col_flag, const bool& row_flag, const bool& slice_flag,
                                           const int& ncols, const int& nrows, const int& nslices,
                                           vector<Index>& seam_index_data, vector<RegionClass>& seam_region_classes);
#else
      friend void seam_region_classes_init(const bool& col_flag, const bool& row_flag, const int& ncols, const int& nrows,
                                           vector<Index>& seam_index_data, vector<RegionClass>& seam_region_classes);
#endif
#ifdef PARALLEL
      friend void recur_send(const short unsigned int& section, const short unsigned int& recur_level,
                             vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, 
                             const unsigned int& nregions, const unsigned int& global_nregions, 
                             const double& max_threshold, Temp& temp_data);

      friend void parallel_server(const short unsigned int& section, Spatial& spatial_data, vector<Pixel>& pixel_data,
                                  vector<RegionClass>& region_classes, vector<RegionClass *>& nghbr_heap, vector<RegionClass *>& region_heap,
                                  Temp& temp_data);
#endif
#endif // RHSEG_RUN
      friend class Pixel;
      friend class Index;
      friend class Results;
      friend class RegionMoreThan;
      friend class NghbrMoreThan;
      friend class NpixMoreThan;
      friend class DistToMin;
      friend class ClassNpixMoreThan;
      friend class ClassStdDevLessThan;
      friend class ClassBPRatioLessThan;
    private:
      //  PRIVATE DATA
      bool               active_flag;  // true if active, false if not-active or merged into another region
#ifdef RHSEG_RUN
      bool               initial_merge_flag; // true if npix > INITIAL_MERGE_NPIX at end of first_merge_reg_grow.
      bool               seam_flag;    // true if region borders a processing window seam.
      bool               merged_flag;  // true if involved in merge in current iteration, false otherwise
      bool		 large_nghbr_merged_flag; // true if involved in a neighbor merge between large regions.
#endif // RHSEG_RUN
      unsigned int       label;        // RegionClass label
      unsigned int       npix;         // Number of pixels in the region
#ifdef RHSEG_RUN
      unsigned int       nghbr_heap_npix; // Value of npix used by the nghbr_heap (not updated until the nghbr_heap is updated)
      unsigned int       region_heap_npix;// Value of npix used by the region_heap (not updated until the region_heap is updated)
#endif
      double             *sum;         // Sum of image values (in each band)
      double             *sumsq;       // Sum of squares of image values (in each band)
      double             *sumxlogx;    // Sum of xlogx of image values x (in each band)
      double             *sum_pixel_std_dev; // Sum of the local pixel standard deviation feature value (in each band)
      float              max_edge_value;     // Maximum edge value in region
      double		 band_max_std_dev;   // Band maximum standard deviation for this region
      set<unsigned int>  nghbrs_label_set;   // Set of labels of the neighbors of this region
#ifdef RHSEG_RUN
      set<unsigned int>  best_nghbr_label;   // RegionClass labels of the most similar neighboring region
      float              best_nghbr_dissim;  // Dissimilarity value of most similar neighboring region
      unsigned int       nghbr_heap_index;   // Index for this region in nghbr_heap
      set<unsigned int>  best_region_label;  // RegionClass labels of the most similar regions
      float              best_region_dissim; // Dissimilarity value of most similar region
      unsigned int       region_heap_index;  // Index for this region in region_heap
      map<unsigned int, RegionSeam> nghbrs_label_seam_map;       // Used in artifact elimination only
#endif
      unsigned int       merge_region_label; // RegionClass label of region into which this region was merged (=0 if not merged)
      double             sum_pixel_gdissim;  // Sum of the dissimilarity values across all region pixels
      double             min_region_dissim;  // Dissimilarity of region to minimum vector
      unsigned int       nb_region_objects;    // Number of region objects contained in this region class.
      set<unsigned int>  region_objects_set;   // Set of labels of spatially connected subregions contained in this region
      unsigned int       boundary_npix;      // Number of boundary pixels in the region.
      double             merge_threshold;    // Merge threshold for most recent merge involving this region.
      //  PRIVATE STATIC DATA
      static bool    sumsq_flag;        // true iff the sumsq feature is to be calculated.
      static bool    sumxlogx_flag;     // true iff the sumxlogx feature is to be calculated.
      static bool    std_dev_flag;      // true iff the spatial (std_dev) feature is to be used.
      static int     nbands;            // Number of spectral bands in the input image data.
      static double  *scale, *offset;   // Scale and offset values used in input image data scaling and normalization
#ifdef RHSEG_RUN
      static double  *minval;           // Minimum value in each band (after normalization)
      static double  *meanval;          // Mean value in each band (after normalization)
#endif

#ifdef PARALLEL
#include <parallel/region_class.h>
#endif
  };

#ifdef RHSEG_RUN
  // Comparison criterion for nghbr_heap
  class NghbrMoreThan
  {
    public:
      bool operator () (const RegionClass *region1, const RegionClass *region2) const
      { if ((region1->best_nghbr_dissim == region2->best_nghbr_dissim) &&
            (region1->nghbr_heap_npix == region2->nghbr_heap_npix))
          return (region1->label < region2->label);
        else if (region1->best_nghbr_dissim == region2->best_nghbr_dissim)
          return (region1->nghbr_heap_npix > region2->nghbr_heap_npix);
        else
          return (region1->best_nghbr_dissim > region2->best_nghbr_dissim); }
  };

  // Comparison criterion for region_heap
  class RegionMoreThan
  {
    public:
      bool operator () (const RegionClass *region1, const RegionClass *region2) const
      { if ((region1->best_region_dissim == region2->best_region_dissim) &&
            (region1->region_heap_npix == region2->region_heap_npix))
          return (region1->label < region2->label);
        else if (region1->best_region_dissim == region2->best_region_dissim)
          return (region1->region_heap_npix > region2->region_heap_npix);
        else
          return (region1->best_region_dissim > region2->best_region_dissim); }
 };
#endif // RHSEG_RUN

  // Sort functions specifying sorting by the npix RegionClass data element
  class NpixMoreThan
  {
    public:
      bool operator () (const RegionClass& region1, const RegionClass& region2) const
      { if (region1.npix == region2.npix)
          return (region1.label < region2.label);
        else
          return (region1.npix > region2.npix); }
  };

  // Sort functions specifying sorting by the min_region_dissim RegionClass data element
  class DistToMin
  {
    public:
      bool operator () (const RegionClass& region1, const RegionClass& region2) const
      { if ((region1.min_region_dissim == region2.min_region_dissim) && (region1.npix == region2.npix))
          return (region1.label < region2.label);
        else if (region1.min_region_dissim == region2.min_region_dissim)
          return (region1.npix > region2.npix);
        else
          return (region1.min_region_dissim < region2.min_region_dissim); }
  };

  // Comparison criteria for class_heap
  class ClassNpixMoreThan
  {
     public:
        bool operator ( ) (const RegionClass *region1, const RegionClass *region2) const
        { 
          if (region1->npix == region2->npix)
            return (region1->label > region2->label);
          else
            return (region1->npix < region2->npix);
        }
  };

  class ClassStdDevLessThan
  {
     public:
        bool operator ( ) (const RegionClass *region1, const RegionClass *region2) const
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

  class ClassBPRatioLessThan
  {
     public:
        bool operator ( ) (const RegionClass *region1, const RegionClass *region2) const
        { 
          float bp_ratio1 = ((float) region1->boundary_npix)/((float) region1->npix);
          float bp_ratio2 = ((float) region2->boundary_npix)/((float) region2->npix);
          if (bp_ratio1 == bp_ratio2)
            return (region1->label > region2->label);
          else
            return (bp_ratio1 > bp_ratio2); }
  };

  // Related functions
#ifdef RHSEG_RUN
#ifdef THREEDIM
  unsigned int do_region_classes_init(const int& ncols, const int& nrows,
                                      const int& nslices, vector<Pixel>& pixel_data,
                                      vector<RegionClass>& region_classes);

  unsigned int do_region_classes_init(const short unsigned int& recur_level, const short unsigned int& section,
                                      const int& ncols, const int& nrows, const int& nslices,
                                      vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                                      Temp& temp_data);

  bool region_classes_check(const short unsigned int& recur_level, const short unsigned int& section,
                            const int& ncols, const int& nrows, const int& nslices,
                            vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                            Temp& temp_data);

  void update_nghbrs_label_set(const short unsigned int& recur_level, const int& ncols, const int& nrows, const int& nslices,
                              vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                              vector<Index>& slice_seam_index_data,
                              vector<RegionClass>& region_classes);

  void seam_region_classes_init(const bool& col_flag, const bool& row_flag, const bool& slice_flag,
                                const int& ncols, const int& nrows, const int& nslices,
                                vector<Index>& seam_index_data, vector<RegionClass>& seam_region_classes);

  void update_region_class_info(const int& ncols, const int& nrows,
                                const int& nslices, const short unsigned int& nslevels,
                                Spatial& spatial_data, vector<RegionClass>& region_classes);

  void update_sum_pixel_gdissim(const int& ncols, const int& nrows,
                                const int& nslices, vector<Pixel>& pixel_data,
                                vector<RegionClass>& region_classes);
#else
  unsigned int do_region_classes_init(const int& ncols, const int& nrows,
                                      vector<Pixel>& pixel_data,
                                      vector<RegionClass>& region_classes);

  unsigned int do_region_classes_init(const short unsigned int& recur_level, const short unsigned int& section,
                                      const int& ncols, const int& nrows,
                                      vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                                      Temp& temp_data);

  bool region_classes_check(const short unsigned int& recur_level, const short unsigned int& section,
                            const int& ncols, const int& nrows,
                            vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                            Temp& temp_data);

  void update_nghbrs_label_set(const short unsigned int& recur_level, const int& ncols, const int& nrows,
                              vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                              vector<RegionClass>& region_classes);

  void seam_region_classes_init(const bool& col_flag, const bool& row_flag, const int& ncols, const int& nrows,
                                vector<Index>& seam_index_data, vector<RegionClass>& seam_region_classes);

  void update_region_class_info(const int& ncols, const int& nrows,
                                const short unsigned int& nslevels,
                                Spatial& spatial_data, vector<RegionClass>& region_classes);

  void update_sum_pixel_gdissim(const int& ncols, const int& nrows,
                                vector<Pixel>& pixel_data,
                                vector<RegionClass>& region_classes);
#endif
  void update_region_class_info(const short unsigned int& recur_level,
                                const short unsigned int& section, const short unsigned int& nslevels,
                                Spatial& spatial_data, vector<RegionClass>& region_classes,
                                Temp& temp_data);

  void update_sum_pixel_gdissim(const short unsigned int& recur_level, const short unsigned int& section, 
                                vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                                Temp& temp_data);

  bool merge_regions(const bool& last_stage_flag, unsigned int& converge_nregions,
                     vector<RegionClass *>& nghbr_heap, unsigned int& nghbr_heap_size,
                     vector<RegionClass *>& region_heap, unsigned int& region_heap_size,
                     vector<RegionClass>& region_classes, unsigned int& nregions,
                     double& max_threshold);

  void merge_seam_regions(vector<RegionClass *>& nghbr_heap, unsigned int& nghbr_heap_size,
                          vector<RegionClass>& seam_region_classes, vector<RegionClass>& region_classes,
                          unsigned int& nregions, double& max_threshold);

  void update_nghbrs(set<RegionClass *>& update_nghbrs_set,
                     vector<RegionClass *>& nghbr_heap, unsigned int& nghbr_heap_size,
                     vector<RegionClass>& region_classes);

  void update_regions(set<RegionClass *>& update_regions_set, set<RegionClass *>& removed_regions_set,
                      vector<RegionClass *>& region_heap, unsigned int& region_heap_size);

#ifdef THREEDIM
    void nghbrs_label_set_init(const int& ncols, const int& nrows, const int& nslices,
                              vector<Pixel>& pixel_data, vector<RegionClass>& region_classes);

    void nghbrs_label_set_init(const short unsigned int& recur_level, const short unsigned int& section, 
                              unsigned int max_region_label, const int& ncols, const int& nrows, const int& nslices,
                              vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                              Temp& temp_data);
#else
    void nghbrs_label_set_init(const int& ncols, const int& nrows,
                              vector<Pixel>& pixel_data, vector<RegionClass>& region_classes);

    void nghbrs_label_set_init(const short unsigned int& recur_level, const short unsigned int& section, 
                              unsigned int max_region_label, const int& ncols, const int& nrows,
                              vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                              Temp& temp_data);
#endif
#endif // RHSEG_RUN
} // HSEGTilton

#endif /* REGIONCLASS_H */
