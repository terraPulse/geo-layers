#ifndef IMAGE_H
#define IMAGE_H

//#define MAX_IO_SIZE      402653184 // 8192*8192*6
#define MAX_IO_SIZE      805306368 // 8192*8192*12
#define MAX_COLORMAP_SIZE SHRT_MAX

// #include <cpl_port.h> added for error related to undefined GUIntBig, not sure if will cause compatibility issues
#include "../shape/shape.h"
#ifdef SHAPEFILE
#include "../point/point.h"
#endif
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#ifdef GDAL
#include <sstream>
#include <stdexcept>
#include <gdal_priv.h>
#include <gdalwarper.h>
#endif // GDAL

using namespace std;

namespace CommonTilton
{

  enum RHSEGDType { Unknown, UInt8, UInt16, UInt32, Float32 };
  enum MathOperation { Add, Subtract, Multiply, Divide, Equal, LessThan, MoreThan };
  enum EdgeOperation { Prewitt, Sobel, Scharr, Frei_Chen, Std_Dev };

  struct ColorMap
  {
    unsigned char  r,g,b;
    unsigned short sum;
  };

  // Comparison criterion for heap
  class ColorLessThan
  {
    public:
      bool operator ( ) (const ColorMap& colormap1, const ColorMap& colormap2)
      {
        if ((colormap1.sum == colormap2.sum) && (colormap1.b == colormap2.b) &&
            (colormap1.g == colormap2.g))
          return (colormap1.r < colormap2.r);
        else if ((colormap1.sum == colormap2.sum) && (colormap1.b == colormap2.b))
          return (colormap1.g < colormap2.g);
        else if (colormap1.sum == colormap2.sum)
          return (colormap1.b < colormap2.b);  // Previously (colormap1.r < colormap2.r) - error found by bjgslzif@gmail.com
        else
          return (colormap1.sum < colormap2.sum);
      }
  };

  class Image
  {
    public:
    // Constructors and Destructor
      Image();
      virtual ~Image();

    // Member functions
#ifdef GDAL
      bool open(const string& file_name);
#endif
#ifdef THREEDIM
      bool open(const string& file_name, const int& n_cols, const int& n_rows, const int& n_slices,
                const int& n_bands, const RHSEGDType& d_type);
#endif
      bool open(const string& file_name, const int& n_cols, const int& n_rows,
                const int& n_bands, const RHSEGDType& d_type);
      bool create(const string& file_name, const Image& baseImage);
      bool create(const string& file_name, const Image& baseImage, const int& n_bands);
#ifdef GDAL
      bool create(const string& file_name, const Image& baseImage, const int& n_bands, const GDALDataType& datatype);
      bool create(const string& file_name, const Image& baseImage, const int& n_bands,
                  const GDALDataType& datatype, const string& n_driver_description);
#endif
      bool create(const string& file_name, const Image& baseImage, const int& n_bands, const RHSEGDType& d_type);
#ifdef THREEDIM
      bool create(const string& file_name, const int& n_cols, const int& n_rows, const int& n_slices,
                  const int& n_bands, const RHSEGDType& d_type);
#else
      bool create(const string& file_name, const int& n_cols, const int& n_rows,
                  const int& n_bands, const RHSEGDType& d_type);
#endif
#ifdef GDAL
      void create(const string& file_name, const int& n_cols, const int& n_rows,
                  const int& n_bands, const GDALDataType& data_type,
                  const double& Xoffset, const double& Yoffset, const double& Xgsd, const double& Ygsd,
                  const string& n_driver_description);
      bool create(const string& file_name, const int& n_cols, const int& n_rows,
                  const int& n_bands, const GDALDataType& datatype,
                  const string& n_driver_description);
#endif
#ifdef GDAL
      bool create_copy(const string& file_name, const Image& baseImage);
      bool create_copy(const string& file_name, const Image& baseImage, const string& n_driver_description);
      bool create_mask(const string& file_name, Image& sourceImage, const string& n_driver_description); // why can't I use const Image here???
      void registered_copy(const Image& sourceImage);
#endif
      bool gdal_valid() const { return gdal_flag; }
      bool info_valid() const { return info_flag; }
#ifdef GDAL
      GDALDataset *get_imageDataset() const { return imageDataset; }
      void set_snow_color_table();
#else
#endif
#ifdef THREEDIM
#endif
      void set_geotransform(const double& Xoffset, const double& Yoffset, const double& Xgsd, const double& Ygsd);
#ifdef GDAL
      void set_geotransform(const double geotransform[6]);
      void put_geotransform();
      void get_geotransform(double geotransform[6]);
      void set_geotransform(const Image& sourceImage);
      void set_metadata(char** metadata_in, const char* domain);
#endif
      void set_no_data_value(const double& value);
      void set_rgb_display_bands(const int& red_band, const int& green_band,
                                 const int& blue_band);
      void set_rgb_image_stretch(const int& stretch_value);
      void set_rgb_image_stretch(const int& stretch_value, const float& from_value, const float& to_value);

      string get_file_name() const { return image_file; }
      int get_ncols() const { return ncols; }
      void set_ncols(int& value) { ncols = value; return; }
      int get_nrows() const { return nrows; }
      void set_nrows(int& value) { nrows = value; return; }
      int get_nslices() const { return nslices; }
      void set_nslices(int& value) { nslices = value; return; }
      int get_nbands() const { return nbands; }
      void set_nbands(int& value) { nbands = value; return; }
      int get_red_display_band() const { return red_display_band; }
      int get_green_display_band() const { return green_display_band; }
      int get_blue_display_band() const { return blue_display_band; }
      void get_rgb_image_stretch(int& stretch_value, float& from_value, float& to_value);
      RHSEGDType get_dtype() const { return dtype; }
#ifdef GDAL
      void set_gdal_flag(bool value) { gdal_flag = value; return; }
      GDALDataType get_data_type() const;
      void set_data_type(GDALDataType& value) { data_type = value; return; }
      string get_driver_description() const { return driver_description; }
      string get_projection_type() const { return projection_type; }
      void set_projection_type(string& value) { projection_type = value; return; }
      char** get_metadata(const char* domain);
      double get_imageGeoTransform(const int& index) { return imageGeoTransform[index]; }
#endif
      bool geotransform_valid() const { return geotransform_flag; }
      bool data_valid() const { return data_flag; }

      bool clip_and_mask(const double& clip_min, const double& clip_max, const double& no_data_val);
      bool threshold_and_mask(const double& threshold_val, const double& no_data_val);
      bool math(Image& inputImage1, Image& inputImage2, const MathOperation& operation);
      bool math(Image& inputImage1, Image& inputImage2, const MathOperation& operation, const int& output_value);
      bool math(Image& inputImage1, float& value, const MathOperation& operation);
      bool math(Image& inputImage1, float& value, const MathOperation& operation, const int& output_value);
      bool scale_power(Image& inputImage, Image& maskImage, const float& power_value);
      bool random_sample(Image& inputImage, const float& rate);

      bool no_data_value_valid(const int& band) const { return no_data_value_flag[band]; }
      double get_no_data_value(const int& band) { return no_data_value[band]; }

      void computeMinMax();
      void computeMinMax(Image& maskImage);
      void computeMinMax(const bool& approx_OK_flag);
      void computeMinMax(const bool& approx_OK_flag, Image& maskImage);
      bool copyMinMax(Image& sourceImage);
      void resetMinMax();
      void resetMinMax(Image& maskImage);
      double getMinimum(const int& band);
      double getMinimum(const int& band, Image& maskImage);
      double getMaximum(const int& band);
      double getMaximum(const int& band, Image& maskImage);

      void computeStdDev();
      void computeStdDev(Image& maskImage);
      void resetStdDev();
      void resetStdDev(Image& maskImage);
      double getStdDev(const int& band);
      double getStdDev(const int& band, Image& maskImage);

      bool stretch(const int& band, Image& sourceImage, const int& nb_levels);
      bool stretch(const int& band, Image& sourceImage, const int& nb_levels, Image& maskImage);
      void computeHistoEqMap(const int& band, const int& nb_levels);
      void computeHistoEqMap(const int& band, const int& nb_levels, Image& maskImage);
#ifdef GDAL
      void computeHistoEqMapGDAL(const int& band, const int& nb_levels);
      void computeHistoEqMapGDAL(const int& band, const int& nb_levels, Image& maskImage);
      void computeHistoEqMapGDAL(const bool& approx_OK_flag, const int& band, const int& nb_levels);
      void computeHistoEqMapGDAL(const bool& approx_OK_flag, const int& band, const int& nb_levels, Image& maskImage);
#endif
      void computeHistoEqMapNoGDAL(const int& band, const int& nb_levels);
      void computeHistoEqMapNoGDAL(const int& band, const int& nb_levels, Image& maskImage);
      bool performHistoEq(const int& band, Image& sourceImage);
      bool performHistoEq(const int& band, Image& sourceImage, Image& maskImage);
      bool histo_eq_map_valid(const int& band) const { if (histo_eq_map_flag.empty())
                                                         return false;
                                                       else
                                                         return histo_eq_map_flag[band]; }
      int get_histo_eq_map_size(const int& band) const { return histo_eq_map[band].size(); }
      int get_histo_eq_map(const int& band, const int& index) const {return histo_eq_map[band][index]; }
      void copy_histo_eq_map(const int& band, const Image& sourceImage, const int& sourceBand);
      double get_histo_scale(const int& band) const { return histo_scale[band]; }
      double get_histo_offset(const int& band) const { return histo_offset[band]; }

      double get_histo_lookup(const double& lookup_value, const int& band);
      void copy_histo_lookup(const int& band, const Image& sourceImage, const int& sourceBand);

      void resize_colormap(const unsigned int& colormap_size);
      void set_colormap_value(const unsigned int &index, const unsigned int& red_value,
                              const unsigned int& green_value, const unsigned int& blue_value);
      void compute_colormap(const unsigned int& colormap_size);
      unsigned char get_red_colormap(const unsigned int& data_value);
      unsigned char get_green_colormap(const unsigned int& data_value);
      unsigned char get_blue_colormap(const unsigned int& data_value);
      void copy_colormap(const Image& sourceImage);
      void print_colormap();
      void scale_offset(const float& scaled_min, const float& scaled_std_dev, Image& maskImage);
      void scale_offset(const float& scaled_min, const float& scaled_std_dev);
      void scale_offset(const int& new_min, const int& new_max, const int& band, Image& maskImage);
      void scale_offset_SI(Image& SIImage, const float SI_scale, const float SI_offset, const int& SI_band,
                           const int& min_valid, const int& max_valid);
      void chi_square_ratio(Image& posChiSquareImage, Image& negChiSquareImage, const double& chi_square_threshold,
                            Image& dofImage, const int& dof_threshold);
#ifdef MODIS
      void copy_swath_lat_long_data(Image& swathLatitudeImage, Image& swathLongitudeImage);
      void copy_proj_lat_long_data(Image& projLatitudeImage, Image& projLongitudeImage);
      void NNProjColRow(Image& swathLatLongImage, Image& projLatLongImage);
      void NNProj(Image& swathDataImage, Image& projColRowImage);
      void modis_scale_and_correct(Image& inputImage, const int& input_band, const float& scale, const float& offset,
                                   Image& solarZenithImage, const int& output_band);
      void compute_surface_temperature(Image& inputImage, vector<float>& scale, vector<float>& offset,
                                       const int& band_31, const int& band_32);
      void compute_thin_cirrus_cloud_mask(Image& cloudMaskImage, Image& cloudMaskQAImage, Image& snowCoverImage);
#endif
      void upsample_copy(Image& sourceImage, const double& continuity_ratio);
      void compute_ndsi(Image& inputImage, const int& green_band, const int& MIR_band, Image& maskImage);
      void compute_ndsi(Image& inputImage, const int& green_band, const int& MIR_band);

      void compute_ndvi(Image& inputImage, const int& NIR_band, const int& red_band, Image& maskImage);
      void compute_ndvi(Image& inputImage, const int& NIR_band, const int& red_band);
      void compute_ndvi(Image& NIRImage, Image& redImage, Image& maskImage);
      void compute_ndvi(Image& NIRImage, Image& redImage);

      void compute_difference(Image& firstImage, Image& secondImage);
      void compare(Image& firstImage, Image& secondImage);
      void compare(Image& firstImage, Image& secondImage, const double& UTM_X_shift, const double& UTM_Y_shift);
      void compare(Image& compareImage, const string& compare_type, const double& compare_value);
      bool edge(Image& inputImage, Image& maskImage, const int& bias_value, const EdgeOperation& edge_operation, const double& threshold,
                const int& output_type, Image& outputMaskImage);
      bool edge(Image& inputImage, Image& maskImage, const int& bias_value, const EdgeOperation& edge_operation, const double& threshold,
                const int& output_type, Image& outputMaskImage, const short int& option);
      double prewitt(const double window[3][3]);
      double sobel(const double window[3][3]);
      double scharr(const double window[3][3]);
      double frei_chen(const double window[3][3], const short int& option);
      double std_dev(const double window[3][3]);

#ifdef THREEDIM
      bool data_valid(const int& col, const int& row, const int& slice, const int& band);
#endif
      bool data_valid(const int& col, const int& row, const int& band);
      bool data_valid(const double& UTM_X, const double& UTM_Y, const int& band);
      double offset_for_rounding(const double& value);
#ifdef THREEDIM
      double get_data(const int& col, const int& row, const int& slice, const int& band);
#endif
      double get_data(const int& col, const int& row, const int& band);
      double get_data(const double& UTM_X, const double& UTM_Y, const int& band);
      double get_X_offset() const { return X_offset; }
      double get_Y_offset() const { return Y_offset; }
      double get_X_gsd() const { return X_gsd; }
      double get_Y_gsd() const { return Y_gsd; }
      double get_UTM_X(const int& col) const { return (X_offset + col*X_gsd); }
      double get_UTM_Y(const int& row) const { return (Y_offset + row*Y_gsd); }
      int get_col(double& UTM_X);
      int get_row(double& UTM_Y);

#ifdef THREEDIM
      void put_data(const double& value, const int& col, const int& row, const int& slice, const int& band);
#endif
      void put_data(const double& value, const int& col, const int& row, const int& band);
      void put_data_line(const double& value, const int& col1, const int& col2, const int& row1, const int& row2, const int& band);
      void put_data(const double& value, const double& UTM_X, const double& UTM_Y, const int& band);
      void put_data_line(const double& value, const double& UTM_X1, const double& UTM_X2,
                         const double& UTM_Y1, const double& UTM_Y2, const int& band);
      void put_data_values(const double& value);
      void putMinimum(const double& value, const int& band);
      void putMaximum(const double& value, const int& band);
#ifdef GDAL
      void put_no_data_value();
      bool update_mask(Image& sourceImage);
#endif
      void increment(const int& col, const int& row, const int& band);
      void registered_data_copy(const int& band, Image& sourceImage, const int& source_band);
      void registered_data_copy(const int& band, Image& sourceImage, const int& source_band, Image& maskImage);
/*
      void forward_data_copy(const int& band, Image& forwardProjRowColImage, Image& sourceImage, const int& source_band);
      void reverse_data_copy(const int& band, Image& reverseProjRowColImage, Image& forwardProjRowColImage,
                             Image& sourceImage, const int& source_band);
*/
      void registered_tc_copy(const int& band, Image& sourceImage, const int& tc_band, Image& maskImage);
      void registered_ndvi_copy(const int& band, Image& sourceImage, Image& maskImage);
      void registered_clipped_radar_copy(const int& band, Image& sourceImage, const int& source_band, Image& maskImage);
      void registered_feature_copy(const int& process_flag, const int& band,
                                   Image& sourceImage, const int& source_band, Image& maskImage);
      void registered_mask_copy(Image& sourceImage);
#ifdef SHAPEFILE
      void overlay_copy(const string& shape_base_file_name, const int& red, const int& green, const int& blue);
      void overlay_copy(Shape shapeFile, const int& red, const int& green, const int& blue);
      void shape_to_image(Shape shapeFile);
      bool check_bounds(const double& X_minBound, const double& Y_minBound, const double& X_maxBound, const double& Y_maxBound);
      bool check_bounds(Shape shapeFile);
      bool check_bounds2(const double& X_minBound, const double& Y_minBound, const double& X_maxBound, const double& Y_maxBound);
      bool check_bounds2(int& col_min, int& row_min, int& col_max, int& row_max);
      bool check_bounds2(Shape shapeFile);
      void convert_dpath_to_path(vector<dPoint>& dpath, vector<Point>& path);
#endif
#ifndef GDAL
      void clear_data();
      void close_and_reopen();
#endif
      void flush_data();
      void close();

      void print_info();

      // FRIEND FUNCTIONS and CLASSES
      friend void compute_ndvi(Image& NDVIImage, Image& NIRImage, Image& redImage);
      friend void compute_ndvi(Image& NDVIImage, Image& NIRImage, Image& redImage, Image& maskImage);

    protected:

    private:
      void construct(); // Called by constructors
      bool initialize(const bool& r_flag);
      void allocate_data();
      void read_data();
      void write_data();
      void plot_bool_line(const int& col1, const int& col2, const int& row1, const int& row2,
                          const int& nbcols, vector<bool>& bool_array);
      int make_histo_eq_map(const double& min_data_value, const double& max_data_value, const int& histogram_size,
                            GUIntBig* image_histogram, vector<double>& histo_eq_map);
      int make_histo_eq_map(vector<double>& heap, vector<double>& histo_eq_map);
      void make_histo_lookup(vector<double>& histo_eq_map, const int& min_histo_index, const double& histo_scale,
                             const double& histo_offset, vector<unsigned char>& histo_lookup);
      void interpolate(const double& continuity_ratio);

   // GDAL and image data file information
      bool gdal_flag;              // True if GDAL valid and associated with image data file
      string image_file;           // Image data file name
#ifdef GDAL
      GDALDataset *imageDataset;   // Pointer to GDAL data set
      GDALDriver  *driver;         // Pointer to GDAL driver for image data file access
      string driver_description;   // GDAL driver description
#endif
      fstream data_fs;

   // Image data information
      bool info_flag;              // True if image information is valid
      int  ncols, nrows, nslices, nbands;   // Number of columns, rows, slices and bands in the image data file
      RHSEGDType dtype;            // RHSEG style data type.
#ifdef GDAL
      GDALDataType data_type;      // GDAL data type of the image data file (complex data not supported)
#endif
      int    io_nrows;             // Number of rows for image data I/O
      int    io_nrows_step;        // Step size for incrementing rows offset for image I/O
      int    io_nrows_overlap;     // Overlap in image I/O when in read only or read and write mode (no overlap in write only mode)
      int    io_nrows_offset;      // Number of rows offset for image I/O (-1 if invalid)
      bool   read_flag;		   // true => read_only
#ifndef GDAL
      bool   write_flag;           // true => write_only
      int    io_band;              // In non_GDAL write_only mode, band being written (ignored in read_only mode - all bands read at once)
#endif
      vector<double> no_data_value;       // Data values for each band that flag invalid data elements.
      vector<bool>   no_data_value_flag;  // True if corresponding no_data_values are valid
      vector<double> min_value, max_value;           // Minimum and Maximum data values for each band
      vector<bool>   min_value_flag, max_value_flag; // True if corresponding minimum and maximum values are valid
      vector<double> std_dev_value; // Standard Deviation values for each band
      vector<bool>   std_dev_flag;  // True if corresponding minimum and maximum values are valid

   // Image data geotransform information
      bool geotransform_flag;      // True if geotransform information is valid
      double X_offset, Y_offset, X_gsd, Y_gsd; //  X and Y offset and ground sampling distance
#ifdef GDAL
      string   projection_type;    // Image projection type - empty string if invalid
   // imageGeoTransform[0] = X_offset; imageGeoTransform[1] = X_gsd; imageGeoTransform[2] = 0;
   // imageGeoTransform[3] = Y_offset; imageGeoTransform[4] = 0;     imageGeoTransform[5] = Y_gsd;
      double imageGeoTransform[6]; // GDAL GeoTransform vector
#endif
   // Permissible data arrays. Complex not supported.
      bool data_flag;                   // True if image data is valid
      unsigned char      *byte_data;    // GDT_Byte  or dtype = UInt8
      unsigned short int *ushort_data;  // GDT_UInt16 or dtype = UInt16
      unsigned int       *uint_data;    // GDT_Uint32 or dtype = UInt32
      float              *float_data;   // GDT_Float32 or dtype = Float32
#ifdef GDAL
      short int          *short_data;   // GDT_Int16
      int                *int_data;     // GDT_Int32
      double             *double_data;  // GDT_Float64
#endif

   // Other member variables
      int    red_display_band, green_display_band, blue_display_band;
      int    rgb_image_stretch;
      float  range[2];
      vector<bool>   histo_eq_map_flag;
      vector<double> histo_scale;
      vector<double> histo_offset;
      vector< vector<double> > histo_eq_map;
      vector< vector<unsigned char> > histo_lookup;
      vector<ColorMap> colormap;
      bool             colormap_flag;  // True if colormap is valid, false otherwise.
  };

} // CommonTilton

#endif /* IMAGE_H */
