/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Spatial class, which is a class dealing
   >>>>                 with spatial data
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 E-Mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  December 9, 2002
   >>>>
   >>>> Modifications:  June 18, 2003 - Replaced scaled_value variable member with
   >>>>                 byte_input_data and short_input_data members (to save disk space)
   >>>>                 August 20, 2003 - Reorganized program and added user controlled byteswapping
   >>>>                 November 5, 2003 - Made all member variables private.
   >>>>                 November 24, 2003 - Added connected_component_init function.
   >>>>                 November 24, 2003 - Added boundary_flag member variable.
   >>>>                 March 3, 2004 - Changed boundary_flag to boundary_map.
   >>>>                 May 9, 2005 - Integrated with VisiQuest
   >>>>                 May 27, 2005 - Added temporary file usage for handling large files
   >>>>                 October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
   >>>>                 February 13, 2006 - Added minval member variable
   >>>>                 August 28, 2006 - Added meanval member variable
   >>>>                 May 30, 2007 - region_class_label_map upgraded to unsigned int (from short unsigned int)
   >>>>                 November 1, 2007 - Eliminated VisiQuest related code.
   >>>>                 February 20, 2008 - Modified to work with hsegreader program.
   >>>>                 May 12, 2008 - Modified to work with new Params object class
   >>>>                 May 12, 2009 - Upgraded to work with float input image data and GDAL image data I/O.
   >>>>                 July 30, 2013 - Added code to input std_dev_image and edge_image and corresponding masks.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#ifndef SPATIAL_H
#define SPATIAL_H

#include <defines.h>
#ifdef GDAL
#include <image/image.h>
#endif
#include <cstring>
#include <fstream>
#include <vector>
#include <set>

using namespace std;
#ifdef GDAL
using namespace CommonTilton;
#endif

namespace HSEGTilton
{

    class Pixel;
    class RegionClass;
    class RegionObject;
    class Index;
    struct Temp;

    class Spatial
    {
      public:
          //  CONSTRUCTORS and DESTRUCTOR
          Spatial();
          ~Spatial();
          //  MODIFICATION MEMBER FUNCTIONS
#ifdef RHSEG_RUN
          void read_data(vector<Pixel>& pixel_data, Temp& temp_data);
#ifdef GDAL
#ifdef THREEDIM
          void read_byte(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                         const int& number_of_bands, Image& byteImage,
                         const string& file_type, unsigned char *byte_buffer,
                         Temp& temp_data);
#else
          void read_byte(const int& io_ncols, const int& io_nrows,
                         const int& number_of_bands, Image& byteImage,
                         const string& file_type, unsigned char *byte_buffer,
                         Temp& temp_data);
#endif
#else // GDAL
#ifdef THREEDIM
          void read_byte(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                         const int& number_of_bands, const char *byte_file_name,
                         const string& file_type, unsigned char *byte_buffer,
                         Temp& temp_data);
#else
          void read_byte(const int& io_ncols, const int& io_nrows,
                         const int& number_of_bands, const char *byte_file_name,
                         const string& file_type, unsigned char *byte_buffer,
                         Temp& temp_data);
#endif
#endif // GDAL
#ifdef GDAL
#ifdef THREEDIM
          void read_short(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                          const int& number_of_bands, Image& shortImage,
                          const string& file_type, short unsigned int *short_buffer,
                          Temp& temp_data);
#else
          void read_short(const int& io_ncols, const int& io_nrows,
                          const int& number_of_bands, Image& shortImage,
                          const string& file_type, short unsigned int *short_buffer,
                          Temp& temp_data);
#endif
#else // GDAL
#ifdef THREEDIM
          void read_short(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                          const int& number_of_bands, const char *short_file_name,
                          const string& file_type, short unsigned int *short_buffer,
                          Temp& temp_data);
#else
          void read_short(const int& io_ncols, const int& io_nrows,
                          const int& number_of_bands, const char *short_file_name,
                          const string& file_type, short unsigned int *short_buffer,
                          Temp& temp_data);
#endif
#endif // GDAL
#ifdef GDAL
#ifdef THREEDIM
          void read_float(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                          const int& number_of_bands, Image& floatImage,
                          const string& file_type, float *float_buffer,
                          Temp& temp_data);
#else
          void read_float(const int& io_ncols, const int& io_nrows,
                          const int& number_of_bands, Image& floatImage,
                          const string& file_type, float *float_buffer,
                          Temp& temp_data);
#endif
#else // GDAL
#ifdef THREEDIM
          void read_float(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                          const int& number_of_bands, const char *float_file_name,
                          const string& file_type, float *float_buffer,
                          Temp& temp_data);
#else
          void read_float(const int& io_ncols, const int& io_nrows,
                          const int& number_of_bands, const char *float_file_name,
                          const string& file_type, float *float_buffer,
                          Temp& temp_data);
#endif
#endif // GDAL
#ifdef GDAL
#ifdef THREEDIM
          void read_rlblmap(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                            Image& rlblmapImage, const string& file_type, 
                            unsigned int *rlblmap_buffer, Temp& temp_data);
#else
          void read_rlblmap(const int& io_ncols, const int& io_nrows, Image& rlblmapImage,
                            const string& file_type, unsigned int *rlblmap_buffer,
                            Temp& temp_data);
#endif
#else // GDAL
#ifdef THREEDIM
          void read_rlblmap(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                            const char *rlblmap_file_name, const string& file_type,
                            unsigned int *rlblmap_buffer, Temp& temp_data);
#else
          void read_rlblmap(const int& io_ncols, const int& io_nrows, const char *rlblmap_file_name,
                            const string& file_type, unsigned int *rlblmap_buffer,
                            Temp& temp_data);
#endif
#endif // GDAL
#ifndef PARALLEL
          void restore_input_data(const short unsigned int& section,
                                  bool *mask, unsigned char *byte_input_data,
                                  short unsigned int *short_input_data, float *float_input_data);
          void restore_std_dev_data(const short unsigned int& section, bool *std_dev_mask,
                                 unsigned char *byte_input_data, float *float_std_dev_data);
          void restore_edge_data(const short unsigned int& section, bool *edge_mask,
                                 unsigned char *byte_input_data, float *float_edge_data);
          void restore_region_class_label_map(const short unsigned int& section);
          void restore_boundary_map(const short unsigned int& section);
          void restore_region_object_label_map(const short unsigned int& section);
#endif
#endif // RHSEG_RUN
          void update_region_label(vector<Pixel>& pixel_data);
          void update_region_label_map(vector<Pixel>& pixel_data);
#ifdef RHSEG_RUN
          void update_region_label_map(const short unsigned int& recur_level, const short unsigned int& section, 
                                       vector<Pixel>& pixel_data, Temp& temp_data);
#endif
          void set_boundary_map(const unsigned int& nslevels,
                                const unsigned int& int_buf_size, Temp& temp_data);
          void set_region_object_label(const unsigned int& label, const unsigned int& index)
               { region_object_label_map[index] = label; }
          void read_region_maps( );
          void update_region_maps(const short unsigned int& hseg_level, 
                                  vector<RegionClass>& region_classes, vector<RegionObject>& region_objects);
#if (!defined(PARALLEL) && defined(RHSEG_RUN))
          void save_region_class_label_map(const short unsigned int& section);
          void save_boundary_map(const short unsigned int& section);
          void save_region_object_label_map(const short unsigned int& section);
#endif
#ifdef RHSEG_RUN
          void write_region_label_map(Temp& temp_data);
          void write_boundary_map(Temp& temp_data);
#endif
#ifndef GDAL
          void open_region_label_map_output();
          void close_region_label_map_output();
          void open_boundary_map_output();
          void close_boundary_map();
#endif
          //  CONSTANT MEMBER FUNCTIONS
          unsigned int get_region_class_label(const unsigned int& pixel_index) const
                       { return region_class_label_map[pixel_index]; }
          unsigned int get_region_object_label(const unsigned int& pixel_index) const
                       { return region_object_label_map[pixel_index]; }
          short unsigned int get_boundary_map(const unsigned int& pixel_index) const
                        { return boundary_map[pixel_index]; }

          void write_class_labels_map_ext(const string& class_labels_map_ext_file, vector<RegionClass>& region_classes);
          void write_class_npix_map_ext(const string& class_npix_map_ext_file, vector<RegionClass>& region_classes);
          void write_class_mean_map_ext(const string& class_mean_map_ext_file, vector<RegionClass>& region_classes);
          void write_class_std_dev_map_ext(const string& class_std_dev_map_ext_file, vector<RegionClass>& region_classes);
          void write_class_bpratio_map_ext(const string& class_bpratio_map_ext_file, vector<RegionClass>& region_classes);
          void write_object_labels_map_ext(const string& object_labels_map_ext_file, vector<RegionObject>& region_objects);
          void write_object_npix_map_ext(const string& object_npix_map_ext_file, vector<RegionObject>& region_objects);
          void write_object_mean_map_ext(const string& object_mean_map_ext_file, vector<RegionObject>& region_objects);
          void write_object_std_dev_map_ext(const string& object_std_dev_map_ext_file, vector<RegionObject>& region_objects);
          void write_object_bpratio_map_ext(const string& object_bpratio_map_ext_file, vector<RegionObject>& region_objects);
#ifdef SHAPEFILE
          void write_class_object_shapefile_ext(const string& class_shapefile_ext_file, const string& object_shapefile_ext_file, 
                                                const short unsigned int& hseg_level,
                                                vector<RegionClass>& region_classes, vector<RegionObject>& region_objects);
          void write_object_shapefile_ext(const string& class_shapefile_ext_file, const string& object_shapefile_ext_file, 
                                          const short unsigned int& hseg_level,
                                          vector<RegionClass>& region_classes, vector<RegionObject>& region_objects);
#endif
          void print_class_label_map(const short unsigned int& hlevel, vector<RegionClass>& region_classes);
          void print_object_label_map(const short unsigned int& hlevel, vector<RegionObject>& region_objects);
  
    // FRIEND FUNCTIONS and CLASSES
          friend class RegionClass;
          friend class RegionObject;
          friend class Pixel;
          friend class Index;
          friend class Results;
#ifdef THREEDIM
          friend void boundary_map(const int& ncols,
                                   const int& nrows, const int& nslices,
                                   const short unsigned int& nslevels,
                                   Spatial& spatial_data);
          friend void connected_component_init(const int& ncols, const int& nrows, const int& nslices,
                                               unsigned int& max_region_object_label,
                                               vector<RegionObject>& region_objects, Spatial& spatial_data);
          friend void connected_component(const int& ncols, const int& nrows,
                                          const int& nslices, const short unsigned int& nslevels,
                                          vector<RegionObject>& region_objects,
                                          Spatial& spatial_data);
#else
          friend void boundary_map(const int& ncols,
                                   const int& nrows, const short unsigned int& nslevels,
                                   Spatial& spatial_data);
          friend void connected_component_init(const int& ncols, const int& nrows,
                                               unsigned int& max_region_object_label,
                                               vector<RegionObject>& region_objects, Spatial& spatial_data);
          friend void connected_component(const int& ncols, const int& nrows,
                                          const short unsigned int& nslevels, vector<RegionObject>& region_objects,
                                          Spatial& spatial_data);
#endif
#ifdef PARALLEL
          friend void do_object_label_offset(const short unsigned int& recur_level,
                                             unsigned int object_label_offset, Spatial& spatial_data,
                                             Temp& temp_data);
#endif
      private:
          //  PRIVATE DATA
          unsigned int  *region_class_label_map;  // Array of region class label map values.
          short unsigned int *boundary_map;       // Last hierarchical level where pixel is on a region boundary.
          unsigned int  *region_object_label_map; // Array of region object label map values.
          fstream  region_class_label_map_fs;     // Output region class label map file stream.
          fstream  region_object_label_map_fs;    // Output region object label map file stream.
          fstream  boundary_map_fs;               // Output boundary map file stream.
    };

// Related functions
#ifdef THREEDIM
    short unsigned int find_section(const int& col_offset, const int& row_offset, const int& slice_offset);

    void boundary_map(const int& ncols, const int& nrows, const int& nslices,
                      const short unsigned int& nslevels,
                      Spatial& spatial_data);

    void boundary_map(const short unsigned int& recur_level, const short unsigned int& section,
                      const int& ncols, const int& nrows, const int& nslices, const short unsigned int& nslevels,
                      Spatial& spatial_data, Temp& temp_data);

    void boundary_map_seams(const short unsigned int& recur_level, const short unsigned int& section,
                            const short unsigned int& nb_sections, const unsigned char& seam_flag, 
                            const short unsigned int& nslevels, Spatial& spatial_data,
                            const int& ncols, const int& nrows, const int& nslices,
                            vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                            vector<Index>& slice_seam_index_data, Temp& temp_data);
#else
    short unsigned int find_section(const int& col_offset, const int& row_offset);

    void boundary_map(const int& ncols, const int& nrows,
                      const short unsigned int& nslevels,
                      Spatial& spatial_data);

    void boundary_map(const short unsigned int& recur_level, const short unsigned int& section,
                      const int& ncols, const int& nrows, const short unsigned int& nslevels,
                      Spatial& spatial_data, Temp& temp_data);

    void boundary_map_seams(const short unsigned int& recur_level, const short unsigned int& section,
                            const short unsigned int& nb_sections, const unsigned char& seam_flag, 
                            const short unsigned int& nslevels, Spatial& spatial_data,
                            const int& ncols, const int& nrows,
                            vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                            Temp& temp_data);
#endif
    void update_region_label(const short unsigned int& recur_level, const short unsigned int& section,
                            Spatial& spatial_data, vector<Pixel>& pixel_data,
                            Temp& temp_data);
#ifdef SHAPEFILE
    void set_two_label_map(const unsigned int& region_label, unsigned int* region_label_map, 
                           unsigned int* two_label_map, int* label_map_bounds);
    void set_connected_label_map(unsigned int* class_label_map, int* label_map_bounds, unsigned int* object_label_map);
    void set_boundary_mask(const unsigned int& region_label, unsigned int* connected_label_map, 
                           bool* boundary_mask, int* boundary_mask_bounds); 
    void trace_region_outer_boundary(bool* boundary_mask, int* boundary_mask_bounds, 
                                     vector<double>& X, vector<double>& Y, vector<int>& partStart);
    void find_region_hole_labels(const unsigned int& region_label, unsigned int* connected_label_map, 
                                 bool* boundary_mask, int* boundary_mask_bounds, set<unsigned int>& region_hole_label_set);
    void trace_region_hole_boundary(bool* boundary_mask, int* boundary_mask_bounds,
                                    vector<double>& X, vector<double>& Y, vector<int>& partStart);
#endif
#ifndef PARALLEL
    void save_byte_data(const string file_type,
                        const short unsigned int& section, const unsigned int& nelements,
                        unsigned char *byte_buffer);

    void save_short_data(const string file_type,
                         const short unsigned int& section, const unsigned int& nelements,
                         short unsigned int *short_buffer);

    void save_int_data(const string file_type,
                       const short unsigned int& section, const unsigned int& nelements,
                       unsigned int *int_buffer);

    void save_float_data(const string file_type,
                       const short unsigned int& section, const unsigned int& nelements,
                       float *float_buffer);

    void restore_byte_data(const string file_type,
                           const short unsigned int& section, const unsigned int& nelements,
                           unsigned char *byte_buffer);

    void restore_short_data(const string file_type,
                            const short unsigned int& section, const unsigned int& nelements,
                            short unsigned int *short_buffer);

    void restore_int_data(const string file_type,
                          const short unsigned int& section, const unsigned int& nelements,
                          unsigned int *int_buffer);

    void restore_float_data(const string file_type,
                            const short unsigned int& section, const unsigned int& nelements,
                            float *float_buffer);

    void remove_temp_file(const string file_type, const short unsigned int& section);
#endif

} // HSEGTilton

#endif /* SPATIAL_H */
