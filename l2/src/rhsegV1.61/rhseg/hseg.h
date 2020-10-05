/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose: General include file for hseg
   >>>>
   >>>>    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
   >>>>        E-Mail: James.C.Tilton@nasa.gov
   >>>>
   >>>>          Date: December 16, 2002
   >>>> Modifications: January 29, 2003 - Added update_region_index function
   >>>>                February 10, 2003 - Changed region_index to region_label
   >>>>                May 20, 2003 - Added option to initialize with Classical Region Growing.
   >>>>                June 9, 2003 - Changed certain lists to sets for efficiency
   >>>>                June 11, 2003 - Modified the temp_data structure and its use.
   >>>>                August 20, 2003 - Reorganized program and added user controlled byteswapping
   >>>>                October 6, 2004 - Eliminated index_data.
   >>>>                October 18, 2003 - Added boundary_npix_file option
   >>>>                October 30, 2003 - Added sub_rlblmap_file option
   >>>>                November 6, 2003 - Revised Params structure
   >>>>                November 18, 2003 - Added RegionObject class
   >>>>                March 2, 2004 - Changed boundary_flag to boundary_map
   >>>>                December 22, 2004 - Improved sorting efficiency
   >>>>                December 22, 2004 - Changed region label from short unsigned int to unsigned int
   >>>>                December 22, 2004 - Reversed sorting order for size of regions (NpixLessThan -> NpixMoreThan)
   >>>>                January 4, 2005 - Changed from sorting lists to utilizing a heap structure
   >>>>                January 6, 2005 - Added update_ngbhr_heap and update_region_heap functions
   >>>>                May 31, 2005 - Added temporary file I/O for faster processing of large data sets
   >>>>                October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
   >>>>                December 23, 2005 - Made sorting for region labeling for final stage to be by distance
   >>>>                                    from minimum vector region instead of by number of pixels
   >>>>                February 28, 2007 - Added first_merge_reg_grow and nghbrs_label_set_init functions
   >>>>                June 26, 2007 - Added NGHBRS_LABEL_SET_SIZE_LIMIT constant
   >>>>                September 5, 2008 - Modified calls for rhseg and lrhseg vis-a-vis nregions & max_threshold
   >>>>                December 30, 2008 - Removed NGHBRS_LABEL_SET_SIZE_LIMIT approximation from hseg merges, but
   >>>>                                    retained it in first merge region growing algorithm
   >>>>                May 22, 2013 - Removed the NGHBRS_LABEL_SET_SIZE_LIMIT approximation completely.
   >>>>                September 16, 2013 - Removed eliminate_artifacts, remerge, update_new_nghbrs and update_new_regions functions.
   >>>>                September 18, 2013 - Renamed lhseg_edge as artifact_elimination
   >>>>                January 27, 2014 - Revised the artifact_elimination function.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */


#ifndef HSEG_H
#define HSEG_H

#include <defines.h>
#include <set>
#include <map>
#include <vector>

using namespace std;

namespace HSEGTilton
{
    class RegionClass;
    class Spatial;
    class Pixel;
    class Index;

   // Data structure used in saving and restoring data between processing sections.
   // For parallel: Data structure used in transferring data between parallel processes.
    struct Temp
    {
     // Data buffers
      unsigned char      *byte_buffer;
      short unsigned int *short_buffer;
      unsigned int       *int_buffer;
      float              *float_buffer;
      double             *double_buffer;
      unsigned int       byte_buf_size, short_buf_size, int_buf_size, float_buf_size, double_buf_size;
#ifdef PARALLEL
    // Packing buffer (parallel only)
      int                buf_size, position;
      char               *buffer;
#ifdef TIME_IT
      float start_time;
      float setup;
      float compute;
      float transfer;
      float wait;
#endif
#endif
    };

    // Global function definitions
    bool hseg();

#ifdef GTKMM
    void *hseg_thread(void *threadid);
#endif

    void check_buf_size(const unsigned int& byte_buf_size,
                        const unsigned int& short_buf_size, const unsigned int& int_buf_size,
                        const unsigned int& float_buf_size, const unsigned int& double_buf_size,
                        Temp& temp_data);
#ifdef THREEDIM
    void find_nghbr(const int& col, const int& row, const int& slice,
                    const short unsigned int& nbdir, int& nbcol, int& nbrow, int& nbslice);
#else
    void find_nghbr(const int& col, const int& row,
                    const short unsigned int& nbdir, int& nbcol, int& nbrow);
#endif

    unsigned int rhseg(unsigned int& nregions, double& max_threshold, Spatial& spatial_data, 
                       vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, 
                       vector<RegionClass *>& nghbr_heap, vector<RegionClass *>& region_heap, Temp& temp_data);
#ifdef THREEDIM
    void first_merge_reg_grow(const int &ncols, const int& nrows, const int& nslices,
                              vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, unsigned int& nregions);
#else
    void first_merge_reg_grow(const int &ncols, const int& nrows,
                              vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, unsigned int& nregions);
#endif
#ifdef THREEDIM
    unsigned int lrhseg(const unsigned int& region_label_offset,
                        const short unsigned int& recur_level, const short unsigned int& section,
                        const int& ncols, const int& nrows, const int& nslices, unsigned int& global_nregions,
                        double& max_threshold, Spatial& spatial_data, vector<Pixel>& pixel_data, 
                        vector<RegionClass>& region_classes, vector<RegionClass *>& nghbr_heap,
                        vector<RegionClass *>& region_heap, Temp& temp_data);

    void artifact_elimination(const short unsigned int& recur_level, const short unsigned int& section,
                              const int& ncols, const int& nrows, const int& nslices, double& max_threshold,
                              vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data, vector<Index>& slice_seam_index_data,
                              unsigned int& nregions, vector<RegionClass>& seam_region_classes, vector<RegionClass>& region_classes,
                              vector<RegionClass *>& nghbr_heap, vector<Pixel>& pixel_data, Temp& temp_data);
#else
    unsigned int lrhseg(const unsigned int& region_label_offset,
                        const short unsigned int& recur_level, const short unsigned int& section,
                        const int& ncols, const int& nrows, unsigned int& global_nregions,
                        double& max_threshold, Spatial& spatial_data, vector<Pixel>& pixel_data, 
                        vector<RegionClass>& region_classes, vector<RegionClass *>& nghbr_heap, 
                        vector<RegionClass *>& region_heap, Temp& temp_data);

    void artifact_elimination(const short unsigned int& recur_level, const short unsigned int& section,
                              const int& ncols, const int& nrows, double& max_threshold,
                              vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                              unsigned int& nregions, vector<RegionClass>& seam_region_classes, vector<RegionClass>& region_classes,
                              vector<RegionClass *>& nghbr_heap, vector<Pixel>& pixel_data, Temp& temp_data);
#endif
    void lhseg(const short unsigned int& recur_level, const bool& last_stage_flag, const short unsigned int& section,
               unsigned int& converge_nregions, unsigned int& nregions, double& max_threshold,
               vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
               vector<RegionClass *>& nghbr_heap, vector<RegionClass *>& region_heap, Temp& temp_data);

    void seam_merge(unsigned int& nregions, vector<RegionClass>& seam_region_classes,
                    vector<RegionClass>& region_classes, map<unsigned int,unsigned int>& region_class_relabel_pairs);

    unsigned int heap_div2(const unsigned int& num);

    void make_nghbr_heap(vector<RegionClass *>& nghbr_heap,
                         const unsigned int& nghbr_heap_size);

    void remove_from_nghbr_heap(const unsigned int& heap_index,
                                vector<RegionClass *>& nghbr_heap, unsigned int& ngbhr_heap_size);

    bool nghbr_upheap(unsigned int& heap_index, vector<RegionClass *>& nghbr_heap,
                      const unsigned int& nghbr_heap_size);

    bool nghbr_downheap(unsigned int& heap_index, vector<RegionClass *>& nghbr_heap,
                        const unsigned int& nghbr_heap_size);

    bool check_nghbr_heap(vector<RegionClass *>& nghbr_heap,
                          const unsigned int& nghbr_heap_size);

    void make_region_heap(vector<RegionClass *>& region_heap,
                          const unsigned int& region_heap_size);

    void remove_from_region_heap(const unsigned int& heap_index,
                                 vector<RegionClass *>& region_heap, unsigned int& region_heap_size);

    void insert_into_region_heap(RegionClass *insert_region,
                                 vector<RegionClass *>& region_heap, unsigned int& region_heap_size);

    bool region_upheap(unsigned int& heap_index, vector<RegionClass *>& region_heap,
                       const unsigned int& region_heap_size);

    bool region_downheap(unsigned int& heap_index, vector<RegionClass *>& region_heap,
                         const unsigned int& region_heap_size);

    bool check_region_heap(vector<RegionClass *>& region_heap,
                           const unsigned int& region_heap_size);

    void check_region_heap_symmetry(vector<RegionClass>& region_classes,
                                    vector<RegionClass *>& region_heap, const unsigned int& region_heap_size);

#ifdef PARALLEL
#include <parallel/hseg.h>
#endif

} // HSEGTilton

#endif /*-- HSEG_H --*/
