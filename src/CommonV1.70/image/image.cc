// image.cc

#include "image.h"
#include <float.h>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <limits.h>
#ifdef GDAL
#include <ogr_spatialref.h>
#endif

namespace CommonTilton
{

 // Basic constructor
  Image::Image()
  {
    construct();

    return;
  }

 // Destructor...
  Image::~Image()
  {
    return;
  }

 // Open existing image
#ifdef GDAL
  bool Image::open(const string& file_name)
  {
    image_file = file_name;
    imageDataset = (GDALDataset *) GDALOpen( image_file.c_str(), GA_ReadOnly );
    if (imageDataset == NULL)
    {
      cout << "WARNING(Image::open): Could not open " << image_file << endl;
      return false;
    }
    driver = imageDataset->GetDriver();
    if (driver == NULL)
    {
      cout << "ERROR(Image::open): Could not find driver for " << image_file << endl;
      return false;
    }
    driver_description = imageDataset->GetDriver()->GetDescription();
    gdal_flag = true;

    info_flag = false;
    ncols = imageDataset->GetRasterXSize();
    nrows = imageDataset->GetRasterYSize();
    nbands = imageDataset->GetRasterCount();
    GDALDataType band_data_type;
    data_type = GDT_Unknown;
    dtype = Unknown;
    int band;
    for (band = 0; band < nbands; band++)
    {
      band_data_type = imageDataset->GetRasterBand(band+1)->GetRasterDataType();
      switch(band_data_type)
      {
        case GDT_Byte:    if (data_type < band_data_type)
                          {
                            data_type = band_data_type;
                            dtype = UInt8;
                          }
                          break;
        case GDT_UInt16:  if (data_type < band_data_type)
                          {
                            data_type = band_data_type;
                            dtype = UInt16;
                          }
                          break;
        case GDT_Int16:   if (data_type == GDT_UInt16)
                          {
                            data_type = GDT_Int32;
                            dtype = Unknown;
                          }
                          else if (data_type < band_data_type)
                          {
                            data_type = band_data_type;
                            dtype = Unknown;
                          }
                          break;
        case GDT_UInt32:  if (data_type < band_data_type)
                          {
                            data_type = band_data_type;
                            dtype = Unknown;
                          }
                          break;
        case GDT_Int32:   if (data_type == GDT_UInt32)
                          {
                            data_type = GDT_Float32;
                            dtype = Unknown;
                          }
                          else if (data_type < band_data_type)
                          {
                            data_type = band_data_type;
                            dtype = Unknown;
                          }
                          break;
        case GDT_Float32: if (data_type < band_data_type)
                          {
                            data_type = band_data_type;
                            dtype = Float32;
                          }
                          break;
        case GDT_Float64: if (data_type < band_data_type)
                          {
                            data_type = band_data_type;
                            dtype = Unknown;
                          }
                          break;
        default:          cout << "ERROR(image): Band " << band;
                          cout << " is of unknown or unsupported image data type" << endl;
                          return false;
      }
    }

    geotransform_flag = false;
    projection_type = imageDataset->GetProjectionRef();
    if ( imageDataset->GetGeoTransform( imageGeoTransform ) == CE_None )
    {
      X_offset = imageGeoTransform[0];
      Y_offset = imageGeoTransform[3];
      X_gsd = imageGeoTransform[1];
      Y_gsd = imageGeoTransform[5];
      geotransform_flag = true;
    }

    initialize(true);

    return true;
  }
#endif // GDAL

#ifdef THREEDIM
  bool Image::open(const string& file_name, const int& n_cols, const int& n_rows, const int& n_slices,
                   const int& n_bands, const RHSEGDType& d_type)
  {
    image_file = file_name;
    data_fs.open(image_file.c_str(), fstream::in | fstream::out | fstream::binary); // Need to include "out" for seekg to work
    if (!data_fs)
    {
      cout << "ERROR(Image::open): Error opening image file " << image_file << " for reading" << endl;
      return false;
    }
    gdal_flag = false;

    info_flag = false;
    ncols = n_cols;
    nrows = n_rows;
    nslices = n_slices;
    nbands = n_bands;
    dtype = d_type;

    geotransform_flag = false;

    initialize(true);

    return true;
  }
#endif

  bool Image::open(const string& file_name, const int& n_cols, const int& n_rows,
                   const int& n_bands, const RHSEGDType& d_type)
  {
    image_file = file_name;
    data_fs.open(image_file.c_str(), fstream::in | fstream::out | fstream::binary); // Need to include "out" for seekg to work
    if (!data_fs)
    {
      cout << "ERROR(Image::open): Error opening image file " << image_file << " for reading" << endl;
      return false;
    }
    gdal_flag = false;

    info_flag = false;
    ncols = n_cols;
    nrows = n_rows;
    nbands = n_bands;
    dtype = d_type;

    geotransform_flag = false;

    initialize(true);

    return true;
  }

  bool Image::create(const string& file_name, const Image& baseImage)
  {
    bool status;
    int n_bands = baseImage.get_nbands();

    status = create(file_name, baseImage, n_bands);

    return status;
  }

  bool Image::create(const string& file_name, const Image& baseImage, const int& n_bands)
  { 
    bool status;
#ifdef GDAL
    GDALDataType datatype = baseImage.get_data_type();
    status = create(file_name, baseImage, n_bands, datatype);
#else
    RHSEGDType d_type = baseImage.get_dtype();
    status = create(file_name, baseImage, n_bands, d_type);
#endif

    return status;
  }

#ifdef GDAL
  bool Image::create(const string& file_name, const Image& baseImage, const int& n_bands, 
                     const GDALDataType& datatype)
  {
    bool status;
    int n_cols, n_rows;
    string n_driver_description;

    n_cols = baseImage.get_ncols();
    n_rows = baseImage.get_nrows();
    n_driver_description = baseImage.get_driver_description();
// This creates an image with no georectification
    status = create(file_name, n_cols, n_rows, n_bands, datatype, n_driver_description);

// Add the georectification
    set_geotransform(baseImage);
    geotransform_flag = false;
    if (baseImage.geotransform_valid())
    {
      projection_type = baseImage.projection_type;
      if ((projection_type != "") && ((projection_type.find("Unknown") == string::npos) || (projection_type.size() > 10)))
        imageDataset->SetProjection( projection_type.c_str());
      set_geotransform(baseImage.get_X_offset(),baseImage.get_Y_offset(),baseImage.get_X_gsd(),baseImage.get_Y_gsd());
    }

    return status;
  }
#endif

  bool Image::create(const string& file_name, const Image& baseImage, const int& n_bands,
                     const RHSEGDType& d_type)
  {
    bool status;
    int n_cols, n_rows;
#ifdef THREEDIM
    int n_slices;
#endif

    n_cols = baseImage.get_ncols();
    n_rows = baseImage.get_nrows();
#ifdef THREEDIM
    n_slices = baseImage.get_nslices();
    status = create(file_name, n_cols, n_rows, n_slices, n_bands, d_type);
#else
    status = create(file_name, n_cols, n_rows, n_bands, d_type);
#endif // THREEDIM

    return status;
  }

#ifdef GDAL
  bool Image::create(const string& file_name, const Image& baseImage, const int& n_bands, 
                     const GDALDataType& datatype, const string& n_driver_description)
  {
    bool status;
    int n_cols, n_rows;

    n_cols = baseImage.get_ncols();
    n_rows = baseImage.get_nrows();
// This creates an image with no georectification
    status = create(file_name, n_cols, n_rows, n_bands, datatype, n_driver_description);

// Add the georectification
    set_geotransform(baseImage);

    return status;
  }
#endif // GDAL

#ifdef THREEDIM
  bool Image::create(const string& file_name, const int& n_cols, const int& n_rows, const int& n_slices,
                     const int& n_bands, const RHSEGDType& d_type)
#else
  bool Image::create(const string& file_name, const int& n_cols, const int& n_rows,
                     const int& n_bands, const RHSEGDType& d_type)
#endif
  {
    gdal_flag = false;
    image_file = file_name;

    info_flag = false;
    ncols = n_cols;
    nrows = n_rows;
    nbands = n_bands;
    dtype = d_type;

    data_fs.open(image_file.c_str(), fstream::out | fstream::trunc | fstream::binary);
    if (!data_fs)
    {
      cout << "ERROR(image): Error opening image file " << image_file << " for writing" << endl;
      return false;
    }

    geotransform_flag = false;

    initialize(false);

    return true;
  }

#ifdef GDAL
  void Image::create(const string& file_name, const int& n_cols, const int& n_rows,
                     const int& n_bands, const GDALDataType& datatype,
                     const double& Xoffset, const double& Yoffset, 
                     const double& Xgsd, const double& Ygsd, 
                     const string& n_driver_description)
  {
    create(file_name, n_cols, n_rows, n_bands, datatype, n_driver_description);

    X_offset = Xoffset;
    Y_offset = Yoffset;
    X_gsd = Xgsd;
    Y_gsd = Ygsd;
    geotransform_flag = true;

    imageGeoTransform[0] = X_offset;
    imageGeoTransform[1] = X_gsd;
    imageGeoTransform[2] = 0;
    imageGeoTransform[3] = Y_offset;
    imageGeoTransform[4] = 0;
    imageGeoTransform[5] = Y_gsd;
    if (imageDataset->SetGeoTransform( imageGeoTransform ) != CE_None)
      geotransform_flag = false;

    return;
  }


 // Creates image file with no georectification
  bool Image::create(const string& file_name, const int& n_cols, const int& n_rows,
                     const int& n_bands, const GDALDataType& datatype, 
                     const string& n_driver_description)
  {
    gdal_flag = false;

    info_flag = false;
    ncols = n_cols;
    nrows = n_rows;
    nbands = n_bands;
    data_type = datatype;
    switch(data_type)
    {
      case GDT_Byte:    dtype = UInt8;
                        break;
      case GDT_UInt16:  dtype = UInt16;
                        break;
      case GDT_Int16:   dtype = Unknown;
                        break;
      case GDT_UInt32:  dtype = Unknown;
                        break;
      case GDT_Int32:   dtype = Unknown;
                        break;
      case GDT_Float32: dtype = Float32;
                        break;
      case GDT_Float64: dtype = Unknown;
                        break;
      default:          cout << "ERROR(image): Image " << file_name;
                        cout << " is of unknown or unsupported image data type" << endl;
                        return false;
    }

    char **papszOptions = NULL;
    driver_description = n_driver_description;
    driver = GetGDALDriverManager()->GetDriverByName(driver_description.c_str());
    if (driver == NULL)
    {
      cout << "ERROR(image): Could not find driver with name " << driver_description << endl;
      return false;
    }
    image_file = file_name;
    imageDataset = driver->Create(image_file.c_str(), ncols, nrows, nbands, data_type, papszOptions);
    if (imageDataset == NULL)
    {
      cout << "ERROR(image): Could not create " << image_file << endl;
      return false;
    }
    gdal_flag = true;

    geotransform_flag = false;

    initialize(false);

    return true;
  }

 // Use the GDAL method CreateCopy() to create an image of the same size and geographic projection of another image (baseImage).
  bool Image::create_copy(const string& file_name, const Image& baseImage)
  {
    bool status;

    if (baseImage.driver == NULL)
    {
      cout << "ERROR(Image:create_copy): baseImage driver is invalid (NULL)" << endl;
      return false;
    }

    status = create_copy(file_name, baseImage, baseImage.get_driver_description());

    return status;
  }

  bool Image::create_copy(const string& file_name, const Image& baseImage, const string& n_driver_description)
  {
    bool status;
    char **metaData;

    driver_description = n_driver_description;
    driver = GetGDALDriverManager()->GetDriverByName(driver_description.c_str());


    metaData = driver->GetMetadata();
    if( CSLFetchBoolean( metaData, GDAL_DCAP_CREATECOPY, FALSE ) )
    {
//      printf( "Driver %s supports CreateCopy() method.\n", baseImage.driver_description.c_str() );
      image_file = file_name;
      imageDataset = driver->CreateCopy(image_file.c_str(),baseImage.get_imageDataset(), FALSE, NULL, NULL, NULL);
      if (imageDataset == NULL)
      {
        cout << "ERROR(image): Could not create " << image_file << endl;
        return false;
      }
      gdal_flag = true;
    }
    else
    {
//      printf( "Driver %s does not support CreateCopy() method.\n", driver_description.c_str() );
      return false;
    }

    ncols = baseImage.get_ncols();
    nrows = baseImage.get_nrows();
    nbands = baseImage.get_nbands();
    data_type = baseImage.get_data_type();

    status = initialize(false);

    return status;
  }

  bool Image::create_mask(const string& file_name, Image& sourceImage, const string& n_driver_description)
  {
    if (!sourceImage.gdal_valid())
    {
      cout << "ERROR(image): Cannot create mask; invalid GDAL association for source image" << endl;
      return false;
    }
    if (!sourceImage.info_flag)
    {
      cout << "ERROR(image): Cannot create minimum mask; invalid image information for source image" << endl;
      return false;
    }
    if (!sourceImage.no_data_value_valid(0))
    {
      cout << "ERROR(image): Cannot create minimum mask; invalid no data value for source image" << endl;
      return false;
    }

    if (!create(file_name, sourceImage, 1, GDT_Byte, n_driver_description))
      return false;

    int col, row;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        if (sourceImage.data_valid(col,row,0))
          put_data(1.0,col,row,0);
        else
          put_data(0.0,col,row,0);
      }
    flush_data();

    return true;
  }

  void Image::registered_copy(const Image& sourceImage)
  {
  // Setup warp options
    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
    psWarpOptions->eResampleAlg = GRA_NearestNeighbour; // Options: GRA_NearestNeighbour, GRA_Bilinear, GRA_Cubic, GRA_CubicSpline, GRA_Lanczos
    psWarpOptions->hSrcDS = sourceImage.get_imageDataset();
    psWarpOptions->hDstDS = imageDataset;
    psWarpOptions->nBandCount = nbands;
    psWarpOptions->panSrcBands = (int *) CPLMalloc(sizeof(int)*nbands);
    psWarpOptions->panDstBands = (int *) CPLMalloc(sizeof(int)*nbands);
    int band;
    for (band = 0; band < nbands; band++)
    {
      psWarpOptions->panSrcBands[band] = band+1;
      psWarpOptions->panDstBands[band] = band+1;
    }

    psWarpOptions->pfnProgress = GDALTermProgress;   

    psWarpOptions->pTransformerArg =  
        GDALCreateGenImgProjTransformer( sourceImage.get_imageDataset(), 
                                         GDALGetProjectionRef(sourceImage.get_imageDataset()), 
                                         imageDataset,
                                         GDALGetProjectionRef(imageDataset), 
                                         false, 0.0, 1 );

    psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

   // Initialize and execute the warp operation. 
    GDALWarpOperation oOperation;
    oOperation.Initialize(psWarpOptions);
    oOperation.ChunkAndWarpImage( 0, 0, ncols, nrows);

    GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
    GDALDestroyWarpOptions( psWarpOptions );

    return;
  }
#endif

  void Image::set_geotransform(const double& Xoffset, const double& Yoffset, const double& Xgsd, const double& Ygsd)
  {

    X_offset = Xoffset;
    Y_offset = Yoffset;
    X_gsd = Xgsd;
    Y_gsd = Ygsd;
    geotransform_flag = true;
#ifdef GDAL
    imageGeoTransform[0] = X_offset;
    imageGeoTransform[1] = X_gsd;
    imageGeoTransform[2] = 0; 
    imageGeoTransform[3] = Y_offset;
    imageGeoTransform[4] = 0;
    imageGeoTransform[5] = Y_gsd;
    geotransform_flag = true;
    if (imageDataset->SetGeoTransform( imageGeoTransform ) != CE_None)
      geotransform_flag = false;
#endif

    return;
  }
#ifdef GDAL
  void Image::set_geotransform(const double geotransform[6])
  {

    imageGeoTransform[0] = geotransform[0];
    imageGeoTransform[1] = geotransform[1];
    imageGeoTransform[2] = geotransform[2]; 
    imageGeoTransform[3] = geotransform[3];
    imageGeoTransform[4] = geotransform[4];
    imageGeoTransform[5] = geotransform[5];
    X_offset = imageGeoTransform[0];
    X_gsd = imageGeoTransform[1];
    Y_offset = imageGeoTransform[3];
    Y_gsd = imageGeoTransform[5];
    geotransform_flag = true;

    return;
  }

  void Image::put_geotransform()
  {

    if (geotransform_flag)
    {
      if ((projection_type != "") && ((projection_type.find("Unknown") == string::npos) || (projection_type.size() > 10)))
        imageDataset->SetProjection( projection_type.c_str());

      if (imageDataset->SetGeoTransform( imageGeoTransform ) != CE_None)
        geotransform_flag = false;
    }

    return;
  }

  void Image::get_geotransform(double geotransform[6])
  {
    geotransform[0] = imageGeoTransform[0];
    geotransform[1] = imageGeoTransform[1];
    geotransform[2] = imageGeoTransform[2]; 
    geotransform[3] = imageGeoTransform[3];
    geotransform[4] = imageGeoTransform[4];
    geotransform[5] = imageGeoTransform[5];

    return;
  }

  void Image::set_geotransform(const Image& baseImage)
  {
    geotransform_flag = false;
    if (baseImage.geotransform_valid())
    {
      projection_type = baseImage.projection_type;
      if ((projection_type != "") && ((projection_type.find("Unknown") == string::npos) || (projection_type.size() > 10)))
        imageDataset->SetProjection( projection_type.c_str());
      set_geotransform(baseImage.get_X_offset(),baseImage.get_Y_offset(),baseImage.get_X_gsd(),baseImage.get_Y_gsd());
     // NOTE: geotransform_flag set to true internal to set_geotransform function
    }

    return;
  }

  void Image::set_metadata(char** metadata_in, const char* domain)
  {
    imageDataset->SetMetadata(metadata_in, domain);
    return;
  }
#endif
  void Image::set_no_data_value(const double& value)
  {
    int band;

    for (band = 0; band < nbands; band++)
    {
      no_data_value[band] = value;
      no_data_value_flag[band] = true;
    }

    return;
  }

  void Image::set_rgb_display_bands(const int& red_band, const int& green_band, 
                                    const int& blue_band)
  {
    red_display_band = red_band;
    green_display_band = green_band;
    blue_display_band = blue_band;

    return;
  }

  void Image::set_rgb_image_stretch(const int& stretch_value)
  {
    float from = 0.9;
    float to = 0.1;

    set_rgb_image_stretch(stretch_value, from, to);

    return;
  }

  void Image::set_rgb_image_stretch(const int& stretch_value, const float& from_value, const float& to_value)
  {
    rgb_image_stretch = stretch_value;
    range[0] = from_value;
    range[1] = to_value;

    return;
  }

  void Image::get_rgb_image_stretch(int& stretch_value, float& from_value, float& to_value)
  {
    stretch_value = rgb_image_stretch;
    from_value = range[0];
    to_value = range[1];

    return;
  }

#ifdef GDAL
  char** Image::get_metadata(const char* domain)
  {
    return imageDataset->GetMetadata(domain);
  }

  void Image::set_snow_color_table()
  {
    GDALColorTable colorTable(GPI_RGB);
    GDALColorEntry *colorEntry;
    colorEntry = new GDALColorEntry;
    colorEntry->c1 = 0;
    colorEntry->c2 = 255;
    colorEntry->c3 = 0;

    colorTable.SetColorEntry(25,colorEntry);

    imageDataset->GetRasterBand(1)->SetColorTable(&colorTable);
    
    return;
  }

  GDALDataType Image::get_data_type() const
  {
    if (!gdal_flag)
    {
      switch(dtype)
      {
        case UInt8:   return GDT_Byte;
                      break;
        case UInt16:  return GDT_UInt16;
                      break;
        case UInt32:  return GDT_UInt32;
                      break;
        case Float32: return GDT_Float32;
                      break;
        default:      return GDT_Unknown;
                      break;
      }
    }

    return data_type;
  }
#endif

  bool Image::clip_and_mask(const double& clip_min, const double& clip_max, const double& no_data_val)
  {

    if (data_flag)
    {
      int band, col, row;
      double value;
      for (band = 0; band < nbands; band++)
      {
        if (!no_data_value_flag[band])
        {
          no_data_value_flag[band] = true;
          no_data_value[band] = no_data_val;
        }
        min_value[band] = clip_max;
        max_value[band] = clip_min;
        for (col = 0; col < ncols; col++)
          for (row = 0; row < nrows; row++)
          {
            value = get_data(col,row,band);
            if (value < clip_min)
              put_data(no_data_value[band],col,row,band);
            else if (value > clip_max)
              put_data(no_data_value[band],col,row,band);
            else
            {
              if (value > max_value[band])
                max_value[band] = value;
              if (value < min_value[band])
                min_value[band] = value;
            }
          }
        min_value_flag[band] = true;
        max_value_flag[band] = true;
      }
      flush_data();
    }
    else
      return false;

    return true;
  }

  bool Image::threshold_and_mask(const double& threshold_val, const double& no_data_val)
  {

    if (data_flag)
    {
      int band, col, row;
      double value;
      for (band = 0; band < nbands; band++)
      {
        if (!no_data_value_flag[band])
        {
          no_data_value_flag[band] = true;
          no_data_value[band] = no_data_val;
        }
        for (col = 0; col < ncols; col++)
          for (row = 0; row < nrows; row++)
          {
            value = get_data(col,row,band);
            if (threshold_val < 0.0)
            {
              if (value > threshold_val)
                put_data(no_data_value[band],col,row,band);
              else
                put_data((no_data_value[band]+1),col,row,band);
            }
            else
            {
              if (value < threshold_val)
                put_data(no_data_value[band],col,row,band);
              else
                put_data((no_data_value[band]+1),col,row,band);
            }
          }
      }
      flush_data();
    }
    else
      return false;

    return true;
  }

 // If inputImage1 is multiband and inputImage2 is single band, use the single band of inputImage2 for all bands of inputImage1
  bool Image::math(Image& inputImage1, Image& inputImage2, const MathOperation& operation)
  {
    int output_value = 1;
    return math(inputImage1, inputImage2, operation, output_value);
  }

 // If inputImage1 is multiband and inputImage2 is single band, use the single band of inputImage2 for all bands of inputImage1
  bool Image::math(Image& inputImage1, Image& inputImage2, const MathOperation& operation, const int& output_value)
  {
    bool coincident_flag = true;
    int band, image2band, col, row;
    double data_value;

   // Make sure all images are spatially coincident
    if (inputImage1.data_valid() && inputImage2.data_valid())
    {
      if ((geotransform_flag) && (inputImage1.geotransform_flag) && (inputImage2.geotransform_flag))
      {
        if ((X_offset != inputImage1.X_offset) || (X_offset != inputImage2.X_offset))
         coincident_flag =  false;
        if ((Y_offset != inputImage1.Y_offset) || (Y_offset != inputImage2.Y_offset))
         coincident_flag =  false;
        if ((X_gsd != inputImage1.X_gsd) || (X_gsd != inputImage2.X_gsd))
         coincident_flag =  false;
        if ((Y_gsd != inputImage1.Y_gsd) || (Y_gsd != inputImage2.Y_gsd))
         coincident_flag =  false;
      }
      else
      {
        cout << "WARNING(Image::math): Spatial coincidence cannot be confirmed\n" << endl;
      }
    }
    else
    {
      if (inputImage1.data_valid())
        cout << "ERROR(Image::math): Data invalid for inputImage1\n" << endl;
      if (inputImage2.data_valid())
        cout << "ERROR(Image::math): Data invalid for inputImage2\n" << endl;
      return false;
    }

    if (coincident_flag)
    {
      for (band = 0; band < nbands; band++)
       for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          data_value = 0;
          image2band = band;
          if (inputImage2.get_nbands() == 1)
            image2band = 0;
          if (inputImage1.data_valid(col,row,band))
          {
            data_value = inputImage1.get_data(col,row,band);
            switch (operation)
            {
              case Add:       if (inputImage2.data_valid(col,row,image2band))
                                data_value += inputImage2.get_data(col,row,image2band);
                              break;
              case Subtract:  if (inputImage2.data_valid(col,row,image2band))
                                data_value -= inputImage2.get_data(col,row,image2band);
                              break;
              case Multiply:  if (inputImage2.data_valid(col,row,image2band))
                                data_value *= inputImage2.get_data(col,row,image2band);
                              break;
              case Divide:    if (inputImage2.data_valid(col,row,image2band))
                              {
                                if (inputImage2.data_valid(col,row,image2band) != 0)
                                  data_value /= inputImage2.get_data(col,row,image2band);
                                else
                                {
                                  cout << "ERROR(Image::math): Encountered divide by zero\n" << endl;
                                  return false;
                                }
                              }
                              break;
              case Equal:     if (inputImage2.data_valid(col,row,image2band))
                                data_value = (data_value == inputImage2.get_data(col,row,image2band));
                              if (data_value)
                                data_value = output_value;
                              break;
              case LessThan:  if (inputImage2.data_valid(col,row,image2band))
                                data_value = (data_value < inputImage2.get_data(col,row,image2band));
                              if (data_value)
                                data_value = output_value;
                              break;
              case MoreThan:  if (inputImage2.data_valid(col,row,image2band))
                                data_value = (data_value > inputImage2.get_data(col,row,image2band));
                              if (data_value)
                                data_value = output_value;
                              break;
              default:        cout << "ERROR(Image::math): Invalid opearation\n" << endl;
                                return false;
                              break;
            }
          }
          put_data(data_value,col,row,band);
        }
    }
    else
    {
      double UTM_X, UTM_Y;
      for (band = 0; band < nbands; band++)
       for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          UTM_X = X_offset + col*X_gsd;
          UTM_Y = Y_offset + row*Y_gsd;
          data_value = 0;
          image2band = band;
          if (inputImage2.get_nbands() == 1)
            image2band = 0;
          if (inputImage1.data_valid(UTM_X,UTM_Y,band))
          {
            data_value = inputImage1.get_data(UTM_X,UTM_Y,band);
            switch (operation)
            {
              case Add:       if (inputImage2.data_valid(UTM_X,UTM_Y,image2band))
                                data_value += inputImage2.get_data(UTM_X,UTM_Y,image2band);
                              break;
              case Subtract:  if (inputImage2.data_valid(UTM_X,UTM_Y,image2band))
                                data_value -= inputImage2.get_data(UTM_X,UTM_Y,image2band);
                              break;
              case Multiply:  if (inputImage2.data_valid(UTM_X,UTM_Y,image2band))
                                data_value *= inputImage2.get_data(UTM_X,UTM_Y,image2band);
                              break;
              case Divide:    if (inputImage2.data_valid(UTM_X,UTM_Y,image2band))
                              {
                                if (inputImage2.data_valid(UTM_X,UTM_Y,image2band) != 0)
                                  data_value /= inputImage2.get_data(UTM_X,UTM_Y,image2band);
                                else
                                {
                                  cout << "ERROR(Image::math): Encountered divide by zero\n" << endl;
                                  return false;
                                }
                              }
                              break;
              case Equal:     if (inputImage2.data_valid(col,row,image2band))
                                data_value = (data_value == inputImage2.get_data(col,row,image2band));
                              if (data_value)
                                data_value = output_value;
                              break;
              case LessThan:  if (inputImage2.data_valid(col,row,image2band))
                                data_value = (data_value < inputImage2.get_data(col,row,image2band));
                              if (data_value)
                                data_value = output_value;
                              break;
              case MoreThan:  if (inputImage2.data_valid(col,row,image2band))
                                data_value = (data_value > inputImage2.get_data(col,row,image2band));
                              if (data_value)
                                data_value = output_value;
                              break;
              default:        cout << "ERROR(Image::math): Invalid opearation\n" << endl;
                                return false;
                              break;
            }
          }
          put_data(data_value,UTM_X,UTM_Y,band);
        }
    }

    flush_data();

    return true;
  }

  bool Image::math(Image& inputImage1, float& value, const MathOperation& operation)
  {
    int output_value = 1;
    return math(inputImage1, value, operation, output_value);    
  }

  bool Image::math(Image& inputImage1, float& value, const MathOperation& operation, const int& output_value)
  {
    int band, col, row;
    double data_value;

    for (band = 0; band < nbands; band++)
    {
     for (row = 0; row < nrows; row++)
     {
      for (col = 0; col < ncols; col++)
      {
        data_value = 0;
        if (inputImage1.data_valid(col,row,band))
        {
          data_value = inputImage1.get_data(col,row,band);
          switch (operation)
          {
            case Equal:    data_value = (data_value == value);
                           if (data_value)
                             data_value = output_value;
                           break;
            case LessThan: data_value = (data_value < value);
                           if (data_value)
                             data_value = output_value;
                           break;
            case MoreThan: data_value = (data_value > value);
                           if (data_value)
                             data_value = output_value;
                           break;
            case Add:      data_value += value;
                           break;
            case Multiply: data_value *= value;
                           break;
            default:       cout << "ERROR(Image::math): Invalid opearation\n" << endl;
                           return false;
                           break;
          }
        }
        put_data(data_value,col,row,band);
      }
     }
    }

    flush_data();

    return true;
  }

  bool Image::scale_power(Image& inputImage, Image& maskImage, const float& power_value)
  {
    bool mask_flag = maskImage.data_valid();
    int band, col, row;
    double scale, offset, min_input_value, max_input_value, data_value;

    inputImage.computeMinMax(maskImage);

    for (band = 0; band < nbands; band++)
    {
     min_input_value = inputImage.getMinimum(band);
     max_input_value = inputImage.getMaximum(band);
     scale = 1.0/(max_input_value - min_input_value);
     offset = scale*min_input_value;
     for (row = 0; row < nrows; row++)
     {
      for (col = 0; col < ncols; col++)
      {
        data_value = 0;
        if (((mask_flag) && (maskImage.data_valid(col,row,0))) || (inputImage.data_valid(col,row,band)))
        {
          data_value = inputImage.get_data(col,row,band);
          data_value = scale*data_value - offset;
          data_value = pow(data_value, power_value);
        }
        put_data(data_value,col,row,band);
      }
     }
    }

    flush_data();

    return true;
  }

  bool Image::random_sample(Image& inputImage, const float& rate)
  {
    int band, col, row, index, random_index;
    int nb_selected, nb_pixels = ncols*nrows;
    double data_value;
    vector<int> random_indices;
    vector<int>::const_iterator random_indices_iter;

    srand(time(NULL));
    nb_selected = ((int) (nb_pixels*rate))/100;

    while (((int) random_indices.size()) < nb_selected)
    {
      random_index = (int) ((double(rand())/RAND_MAX)*nb_pixels);
      if (random_indices.size() > 0)
      {
        random_indices_iter = find(random_indices.begin(),random_indices.end(),random_index);
        if (random_indices_iter == random_indices.end())
          random_indices.push_back(random_index);
      }
      else
        random_indices.push_back(random_index);
    }

    for (band = 0; band < nbands; band++)
      for (index = 0; index < nb_selected; index++)
      {
        random_index = random_indices[index];
        row = random_index/ncols;
        col = random_index - row*ncols;
        data_value = 0;
        if (inputImage.data_valid(col,row,band))
          data_value = inputImage.get_data(col,row,band);
        put_data(data_value,col,row,band);
      }

    flush_data();

    return true;
  }

  void Image::computeMinMax()
  {
    Image maskImage;
    computeMinMax(false, maskImage);
    return;
  }

  void Image::computeMinMax(const bool& approx_OK_flag)
  {
    Image maskImage;
    computeMinMax(approx_OK_flag, maskImage);
    return;
  }

  void Image::computeMinMax(Image& maskImage)
  {
    computeMinMax(false, maskImage);
    return;
  }

  void Image::computeMinMax(const bool& approx_OK_flag, Image& maskImage)
  {
    bool mask_flag, gotflag = true;
    int col, row, band, mask_col, mask_row;
    double UTM_X, UTM_Y, mask_X, mask_Y;

    for (band = 0; band < nbands; band++)
    {
      gotflag = gotflag && min_value_flag[band];
      gotflag = gotflag && max_value_flag[band];
    }

    if (gotflag)
      return;
    else
    {
      mask_flag = maskImage.data_valid();
      double value;
      for (band = 0; band < nbands; band++)
      {
        min_value_flag[band] = false;
        max_value_flag[band] = false;
#ifdef GDAL
        if ((gdal_flag) && (!mask_flag))
        {
          int gotValue;
          min_value[band] = imageDataset->GetRasterBand(band+1)->GetMinimum(&gotValue);
          min_value_flag[band] = (gotValue != false);
          max_value[band] = imageDataset->GetRasterBand(band+1)->GetMaximum(&gotValue);
          max_value_flag[band] = (gotValue != false);
          if (!(min_value_flag[band] && max_value_flag[band]))
          {
            double aMinMax[2];
            GDALComputeRasterMinMax(imageDataset->GetRasterBand(band+1),approx_OK_flag,aMinMax);
            min_value[band] = aMinMax[0];
            max_value[band] = aMinMax[1];
          }
          min_value_flag[band] = true;
          max_value_flag[band] = true;
        }
        else if (data_flag)
#else

        if (data_flag)
#endif
        {
          min_value[band] = DBL_MAX;
          max_value[band] = -DBL_MAX;
          for (col = 0; col < ncols; col++)
            for (row = 0; row < nrows; row++)
            {
              if (data_valid(col,row,band))
              {
                if (maskImage.data_valid())
                {
                  if (geotransform_flag && maskImage.geotransform_valid())
                  {
                    UTM_X = X_offset + col*X_gsd;
                    UTM_Y = Y_offset + row*Y_gsd;
                    mask_X = (UTM_X - maskImage.X_offset)/maskImage.X_gsd;
                    mask_Y = (UTM_Y - maskImage.Y_offset)/maskImage.Y_gsd;
                    mask_col = (int) offset_for_rounding(mask_X);
                    mask_row = (int) offset_for_rounding(mask_Y);
                  }
                  else
                  {
                    mask_col = col;
                    mask_row = row;
                  }
                  mask_flag = maskImage.data_valid(mask_col,mask_row,0);
                }
                else
                  mask_flag = true;
                if (mask_flag)
                {
                  value = get_data(col,row,band);
                  if (value < min_value[band])
                    min_value[band] = value;
                  if (value > max_value[band])
                    max_value[band] = value;
                }
              }
            }
          min_value_flag[band] = true;
          max_value_flag[band] = true;
        }
        else
        {
          min_value[band] = -DBL_MAX;
          max_value[band] = DBL_MAX;
        }
      }
    }
    return;
  }

  bool Image::copyMinMax(Image& sourceImage)
  {
    int band;

    min_value.resize(nbands);
    max_value.resize(nbands);
    min_value_flag.resize(nbands);
    max_value_flag.resize(nbands);

    if ((nbands == sourceImage.nbands) && (nbands == (int) sourceImage.min_value_flag.size()))
    {

      for (band = 0; band < nbands; band++)
      {
        min_value_flag[band] = sourceImage.min_value_flag[band];
        if (min_value_flag[band])
          min_value[band] = sourceImage.min_value[band];
        max_value_flag[band] = sourceImage.max_value_flag[band];
        if (max_value_flag[band])
          max_value[band] = sourceImage.max_value[band];
      }
      return true;
    }

    return false;
  }

  void Image::resetMinMax()
  {
    Image maskImage;
    resetMinMax(maskImage);
    return;
  }

  void Image::resetMinMax(Image& maskImage)
  {
    int band;

    for (band = 0; band < nbands; band++)
    {
      min_value_flag[band] = false;
      max_value_flag[band] = false;
    }
    computeMinMax(maskImage);
    return;
  }

  double Image::getMinimum(const int& band)
  { 
    if (min_value_flag[band])
      return min_value[band];
    else
      computeMinMax();

    return min_value[band];
  }

  double Image::getMinimum(const int& band, Image& maskImage)
  { 
    if (min_value_flag[band])
      return min_value[band];
    else
      computeMinMax(maskImage);

    return min_value[band];
  }

  double Image::getMaximum(const int& band)
  { 
    if (max_value_flag[band])
      return max_value[band];
    else
      computeMinMax();

    return max_value[band];
  }

  double Image::getMaximum(const int& band, Image& maskImage)
  { 
    if (max_value_flag[band])
      return max_value[band];
    else
      computeMinMax(maskImage);

    return max_value[band];
  }

  void Image::computeStdDev()
  {
    Image maskImage;
    computeStdDev(maskImage);
    return;
  }

  void Image::computeStdDev(Image& maskImage)
  {
    bool mask_flag, gotflag = true;
    int col, row, band, mask_col, mask_row;
    double UTM_X, UTM_Y, mask_X, mask_Y;

    for (band = 0; band < nbands; band++)
    {
      gotflag = gotflag && std_dev_flag[band];
      gotflag = gotflag && std_dev_flag[band];
    }

    if (gotflag)
      return;
    else
    {
      mask_flag = maskImage.data_valid();

      double value, sum, sumsq;
      int npix;
      for (band = 0; band < nbands; band++)
      {
        std_dev_flag[band] = false;
        if (data_flag)
        {
          sum = 0.0;
          sumsq = 0.0;
          npix = 0;
          for (col = 0; col < ncols; col++)
            for (row = 0; row < nrows; row++)
            {
              if (data_valid(col,row,band))
              {
                if (maskImage.data_valid())
                {
                  if (geotransform_flag && maskImage.geotransform_valid())
                  {
                    UTM_X = X_offset + col*X_gsd;
                    UTM_Y = Y_offset + row*Y_gsd;
                    mask_X = (UTM_X - maskImage.X_offset)/maskImage.X_gsd;
                    mask_Y = (UTM_Y - maskImage.Y_offset)/maskImage.Y_gsd;
                    mask_col = (int) offset_for_rounding(mask_X);
                    mask_row = (int) offset_for_rounding(mask_Y);
                  }
                  else
                  {
                    mask_col = col;
                    mask_row = row;
                  }
                  mask_flag = maskImage.data_valid(mask_col,mask_row,0);
                }
                else
                  mask_flag = true;
                if (mask_flag)
                {
                  npix++;
                  value = get_data(col,row,band);
                  sum += value;
                  sumsq += value*value;
                }
              }
            }
          std_dev_value[band] = (sumsq - ((sum*sum)/npix))/(npix-1.0);
          std_dev_flag[band] = true;
        }
      }
    }
    return;
  }

  void Image::resetStdDev()
  {
    Image maskImage;
    resetStdDev(maskImage);
    return;
  }

  void Image::resetStdDev(Image& maskImage)
  {
    int band;

    for (band = 0; band < nbands; band++)
    {
      std_dev_flag[band] = false;
      std_dev_flag[band] = false;
    }
    computeStdDev(maskImage);
    return;
  }

  double Image::getStdDev(const int& band)
  { 
    if (std_dev_flag[band])
      return std_dev_value[band];
    else
      computeStdDev();

    return std_dev_value[band];
  }

  double Image::getStdDev(const int& band, Image& maskImage)
  { 
    if (std_dev_flag[band])
      return std_dev_value[band];
    else
      computeStdDev(maskImage);

    return std_dev_value[band];
  }

// Stretch image data values over full dynamic range (Byte or Unsigned Short Integer)
  bool Image::stretch(const int& band, Image& sourceImage, const int& nb_levels)
  {
    Image maskImage;
    bool status = stretch(band,sourceImage,nb_levels,maskImage);
    return status;
  }

  bool Image::stretch(const int& band, Image& sourceImage, const int& nb_levels, Image& maskImage)
  {
    bool mask_flag = maskImage.data_valid();
    bool coincident_flag = true;
    int col, row;
    double scale, offset, data_value;

   // Make sure all images are spatially coincident
    if (mask_flag)
    {
      if ((geotransform_flag) && (sourceImage.geotransform_flag) && (maskImage.geotransform_flag))
      {
        if ((X_offset != sourceImage.X_offset) || (X_offset != maskImage.X_offset))
         coincident_flag = false;
        if ((Y_offset != sourceImage.Y_offset) || (Y_offset != maskImage.Y_offset))
         coincident_flag = false;
        if ((X_gsd != sourceImage.X_gsd) || (X_gsd != maskImage.X_gsd))
         coincident_flag = false;
        if ((Y_gsd != sourceImage.Y_gsd) || (Y_gsd != maskImage.Y_gsd))
         coincident_flag = false;
      }
      else
        cout << "WARNING(Image::stretch): Spatial coincidence cannot be confirmed\n" << endl;
    }
    else
    {
      if ((geotransform_flag) && (sourceImage.geotransform_flag))
      {
        if (X_offset != sourceImage.X_offset)
         coincident_flag = false;
        if (Y_offset != sourceImage.Y_offset)
         coincident_flag = false;
        if (X_gsd != sourceImage.X_gsd)
         coincident_flag = false;
        if (Y_gsd != sourceImage.Y_gsd)
         coincident_flag = false;
      }
      else
        cout << "WARNING(Image::stretch): Spatial coincidence cannot be confirmed\n" << endl;
    }

    if (nb_levels == 0) 
    {
      scale = 1.0;
      offset = 0.0;
    }
    else
    {
      sourceImage.computeMinMax(maskImage);
      scale = ((double) (nb_levels-1))/(sourceImage.max_value[band] - sourceImage.min_value[band]);
      offset = (double) (-sourceImage.min_value[band]);
      max_value[band] = nb_levels - 1;
      max_value_flag[band] = true;
      min_value[band] = 0.0;
      min_value_flag[band] = true;
    }
    if (coincident_flag)
    {
      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          if (maskImage.data_valid())
          {
            mask_flag = maskImage.data_valid(col,row,0);
          }
          else
            mask_flag = true;
          if (mask_flag && (sourceImage.data_valid(col,row,band)))
          {
            data_value = sourceImage.get_data(col,row,band);
            data_value = scale*(data_value+offset);
            put_data(data_value,col,row,band);
          }
          else
            put_data(0.0,col,row,band);
        }
    }
    else
    {
      double UTM_X, UTM_Y;
      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          UTM_X = X_offset + col*X_gsd;
          UTM_Y = Y_offset + row*Y_gsd;
          if (maskImage.data_valid())
          {
            mask_flag = maskImage.data_valid(UTM_X,UTM_Y,0);
          }
          else
            mask_flag = true;
          if (mask_flag && (sourceImage.data_valid(UTM_X,UTM_Y,band)))
          {
            data_value = sourceImage.get_data(UTM_X,UTM_Y,band);
            data_value = scale*(data_value+offset);
            put_data(data_value,UTM_X,UTM_Y,band);
          }
          else
            put_data(0.0,UTM_X,UTM_Y,band);
        }
    }
    flush_data();

    return true;
  }

// Compute histogram equalization map for RGB display
  void Image::computeHistoEqMap(const int& band, const int& nb_levels)
  {
#ifdef GDAL
    if (gdal_flag)
      computeHistoEqMapGDAL(band,nb_levels);
    else
#endif
      computeHistoEqMapNoGDAL(band,nb_levels);
    return;
  }

  void Image::computeHistoEqMap(const int& band, const int& nb_levels, Image& maskImage)
  {
#ifdef GDAL
    if (gdal_flag)
      computeHistoEqMapGDAL(band,nb_levels,maskImage);
    else
#endif
      computeHistoEqMapNoGDAL(band,nb_levels,maskImage);
    return;
  }

#ifdef GDAL
  void Image::computeHistoEqMapGDAL(const int& band, const int& nb_levels)
  {
    bool approx_OK_flag = true;
    Image maskImage;
    computeHistoEqMapGDAL(approx_OK_flag,band,nb_levels,maskImage);
    return;
  }
// NOTE: maskImage isn't utilized properly - need to set nodatavalue in image
  void Image::computeHistoEqMapGDAL(const int& band, const int& nb_levels, Image& maskImage)
  {
    bool approx_OK_flag = true;
    computeHistoEqMapGDAL(approx_OK_flag,band,nb_levels,maskImage);
    return;
  }

// Compute histogram equalization map for RGB display - use approx_OK_flag = false if you want an exact histogram
  void Image::computeHistoEqMapGDAL(const bool& approx_OK_flag, const int& band, const int& nb_levels)
  {
    Image maskImage;
    computeHistoEqMapGDAL(approx_OK_flag,band,nb_levels,maskImage);
    return;
  }
      
  void Image::computeHistoEqMapGDAL(const bool& approx_OK_flag, const int& band, const int& nb_levels, Image& maskImage)
  {
    int lookup_nb_levels;
    int min_histo_index;
    GDALRasterBand *imageBand;
    double lower_bound, upper_bound;
    // int *image_histogram;

    computeMinMax(approx_OK_flag, maskImage);
    if ((data_type == GDT_Byte) || (data_type == GDT_UInt16) || (data_type == GDT_Int16))
      lookup_nb_levels = max_value[band] - min_value[band] + 1;
    else
      lookup_nb_levels = 32768;
    if ((data_type == GDT_Float32) || (data_type == GDT_Float64))
    {
      lower_bound = min_value[band];
      upper_bound = max_value[band];
    }
    else
    {
      lower_bound = min_value[band] - 0.5;
      upper_bound = max_value[band] + 0.5;
    }
    // image_histogram  = new int[lookup_nb_levels];
    GUIntBig image_histogram[lookup_nb_levels];
    histo_eq_map_flag[band] = false;
    if (nb_levels != 0)
    {
      imageBand = imageDataset->GetRasterBand(band+1);
//cout << "Calling imageBand->GetHistogram with lower_bound = " << lower_bound << ", upper_bound = " << upper_bound;
//cout << " and lookup_nb_levels = " << lookup_nb_levels << endl;
      if (imageBand->GetHistogram(lower_bound, upper_bound, lookup_nb_levels, image_histogram,
                                  false, approx_OK_flag, GDALDummyProgress, NULL) == CE_None)
        histo_eq_map_flag[band] = true;
    }

    if (histo_eq_map_flag[band] == true)
    {
      histo_eq_map[band].resize(nb_levels);
      min_histo_index = make_histo_eq_map(min_value[band], max_value[band], lookup_nb_levels, image_histogram, histo_eq_map[band]);

      histo_scale[band] = (getMaximum(band) - getMinimum(band))/(lookup_nb_levels-1);
      histo_offset[band] = -getMinimum(band)/histo_scale[band];
      histo_lookup[band].resize(lookup_nb_levels);
      make_histo_lookup(histo_eq_map[band],min_histo_index,histo_scale[band],histo_offset[band],histo_lookup[band]);
    }

    return;
  }
#endif
  void Image::computeHistoEqMapNoGDAL(const int& band, const int& nb_levels)
  {
    Image maskImage;
    computeHistoEqMapNoGDAL(band,nb_levels,maskImage);
    return;
  }

  void Image::computeHistoEqMapNoGDAL(const int& band, const int& nb_levels, Image& maskImage)
  {
    bool mask_flag;
    int col, row, mask_col, mask_row;
    int min_histo_index;
    double UTM_X, UTM_Y, mask_X, mask_Y;
    double data_value;
    vector<double> data_heap;

    if (nb_levels != 0)
    {

     // Make sure the image minimum and maximum are included!!
      computeMinMax(maskImage);
      data_value = getMinimum(band);
//cout << "Image minimum = " << data_value;
      data_heap.push_back(data_value);
      data_value = getMaximum(band);
//cout << " and image maximum = " << data_value << endl;
      data_heap.push_back(data_value);

      for (row = 0; row < nrows; row += 10)
        for (col = 0; col < ncols; col += 10)
        {
          if (maskImage.data_valid())
          {
            if (geotransform_flag && maskImage.geotransform_valid())
            {
              UTM_X = X_offset + col*X_gsd;
              UTM_Y = Y_offset + row*Y_gsd;
              mask_X = (UTM_X - maskImage.X_offset)/maskImage.X_gsd;
              mask_Y = (UTM_Y - maskImage.Y_offset)/maskImage.Y_gsd;
              mask_col = (int) offset_for_rounding(mask_X);
              mask_row = (int) offset_for_rounding(mask_Y);
            }
            else
            {
              mask_col = col;
              mask_row = row;
            }
            mask_flag = (maskImage.data_valid(mask_col,mask_row,0));
          }
          else
            mask_flag = true;
          if (mask_flag && (data_valid(col,row,band)))
          {
            data_value = get_data(col,row,band);
            data_heap.push_back(data_value);
          } 
        }

      histo_eq_map[band].resize(nb_levels);
      min_histo_index = make_histo_eq_map(data_heap,histo_eq_map[band]);
      data_heap.clear();

      int lookup_nb_levels = 32768;
#ifdef GDAL
      if (data_type == GDT_Byte)
#else
      if (dtype == UInt8)
#endif
        lookup_nb_levels = 256;

      histo_scale[band] = (getMaximum(band) - getMinimum(band))/(lookup_nb_levels-1);
      histo_offset[band] = -getMinimum(band)/histo_scale[band];
      histo_lookup[band].resize(lookup_nb_levels);
      make_histo_lookup(histo_eq_map[band],min_histo_index,histo_scale[band],histo_offset[band],histo_lookup[band]);

      histo_eq_map_flag[band] = true;
    }

    return;
  } 

  int Image::make_histo_eq_map(const double& min_data_value, const double& max_data_value, const int& histogram_size,
                               GUIntBig* image_histogram, vector<double>& histo_eq_map)
  {
    int number_of_pixels, histogram_index, histo_eq_map_size, histo_eq_map_index, min_histo_eq_map_index, bin_size, count;
    double data_value, data_value_increment;
//cout << "Entering make_histo_eq_map, min_data_value = " << min_data_value << ", max_data_value = ";
//cout << max_data_value << " and histogram_size = " << histogram_size << endl;
    number_of_pixels = 0;
    histo_eq_map_size = histo_eq_map.size();
    for (histo_eq_map_index = 1; histo_eq_map_index < histo_eq_map_size; histo_eq_map_index++)
      histo_eq_map[histo_eq_map_index] = min_data_value - 1.0;
    data_value = min_data_value;
    data_value_increment = (max_data_value - min_data_value)/(histogram_size-1);
//cout << "data_value_increment = " << data_value_increment << endl;
    if (min_data_value == 0.0)
    {
   // If minimum value is zero, put all of the zero values in the first bin, no matter how many
      for (histogram_index = 1; histogram_index < histogram_size; histogram_index++)
        number_of_pixels += image_histogram[histogram_index];
      bin_size = (int) offset_for_rounding(((double) (number_of_pixels))/((double)(histo_eq_map_size - 1)));
      bin_size++;
      count = 0;
    }
    else
    {
      for (histogram_index = 0; histogram_index < histogram_size; histogram_index++)
        number_of_pixels += image_histogram[histogram_index];
      bin_size = (int) offset_for_rounding(((double) (number_of_pixels))/((double)(histo_eq_map_size)));
      bin_size++;
      count = image_histogram[0];
    }
    histo_eq_map_index = (number_of_pixels-1)/bin_size;
    min_histo_eq_map_index = (histo_eq_map_size - 2) - histo_eq_map_index;
    bool reset_flag = false;
    if (min_histo_eq_map_index < 0)
    {
      min_histo_eq_map_index = 0;
      reset_flag = true;
    }
    histo_eq_map[min_histo_eq_map_index] = data_value;
//cout << "number_of_pixels = " << number_of_pixels << ", and bin_size = " << bin_size << endl;
    int last_new_index = 0;
//cout << "image_histogram[0] = " << count << " at data_value = " << data_value << endl;
    for (histogram_index = 1; histogram_index < histogram_size; histogram_index++)
    {
      data_value += data_value_increment;
      count += image_histogram[histogram_index];
//cout << "image_histogram[" << histogram_index << "] = " << image_histogram[histogram_index];
//cout << " at data_value = " << data_value << endl;
      histo_eq_map_index = min_histo_eq_map_index + (count-1)/bin_size;
      if (histo_eq_map_index >= (histo_eq_map_size-1))
      {
        reset_flag = true;
        histogram_index = histogram_size;
      }
      else if (histo_eq_map_index != last_new_index)
      {
        histo_eq_map[histo_eq_map_index] = data_value;
        last_new_index = histo_eq_map_index;
      }
    }
    if (!reset_flag)
    {
      histo_eq_map_index = histo_eq_map_size -1;
      histo_eq_map[histo_eq_map_index] = max_data_value;
    }

    data_value = histo_eq_map[min_histo_eq_map_index];
//    cout << "histo_eq_map[" << min_histo_eq_map_index << "] = " << histo_eq_map[min_histo_eq_map_index] << endl;
    for (histo_eq_map_index = (min_histo_eq_map_index + 1); histo_eq_map_index < histo_eq_map_size; histo_eq_map_index++)
    {
      if (histo_eq_map[histo_eq_map_index] < min_data_value)
        histo_eq_map[histo_eq_map_index] = data_value;
//      cout << "histo_eq_map[" << histo_eq_map_index << "] = " << histo_eq_map[histo_eq_map_index] << endl;
      data_value = histo_eq_map[histo_eq_map_index];
    }

    return min_histo_eq_map_index;
  }

  int Image::make_histo_eq_map(vector<double>& heap, vector<double>& histo_eq_map)
  {
    int index, max_heap_index, histo_eq_index, bin_size, count;
    double data_value;

    histo_eq_index = histo_eq_map.size() - 1;
//cout << "(max)histo_eq_index = " << histo_eq_index;
    max_heap_index = heap.size() - 1;
//cout << " and max_heap_index = " << max_heap_index << endl;
    make_heap(heap.begin(),heap.end());
    bin_size = (int) offset_for_rounding(((double) (max_heap_index))/((double)(histo_eq_index)));
    bin_size++;
//cout << "bin_size = " << bin_size << endl;
    count = bin_size - 1;
    for (index = max_heap_index; index >= 0; index--)
    {
      data_value = heap[0];
      pop_heap(heap.begin(),heap.end());
      heap.pop_back();
      count++;
      if ((count == bin_size) || (index == 0))
      {
//cout << "histo_eq_map[" << histo_eq_index << "] = " << data_value << endl;
        histo_eq_map[histo_eq_index--] = data_value;
        count = 0;
      }
      if (histo_eq_index < 0)
      {
        if (index > 0)
          cout << "WARNING: Found histo_eq_index = " << histo_eq_index << " with heap.size() = " << index << endl;
        break;
      }
    }
    histo_eq_index++;
//cout << "Exiting make_histo_eq_map, histo_eq_index = " << histo_eq_index << endl;
    return histo_eq_index;
  }

  void Image::copy_histo_eq_map(const int& band, const Image& sourceImage, const int& sourceBand)
  { 
    int index, nb_levels;

    nb_levels = sourceImage.histo_eq_map[sourceBand].size();
    histo_eq_map[band].resize(nb_levels);

    if (sourceImage.histo_eq_map_flag[sourceBand])
    {
      histo_eq_map_flag[band] = true;
      for (index = 0; index < nb_levels; index++)
        histo_eq_map[band][index] = sourceImage.histo_eq_map[sourceBand][index];
    }
  }

  void Image::make_histo_lookup(vector<double>& histo_eq_map, const int& min_histo_index, const double& histo_scale,
                                const double& histo_offset, vector<unsigned char>& histo_lookup)
  {
    int index, initial_index, lookup_index;
    int lookup_size = histo_lookup.size();
    int histo_eq_size = histo_eq_map.size();
    double lookup_value, diff_value;
//cout << "Entering make_histo_lookup, min_histo_index = " << min_histo_index << ", histo_scale = " << histo_scale;
//cout << " and histo_offset = " << histo_offset << endl;
    initial_index = min_histo_index;
    for (lookup_index = 0; lookup_index < lookup_size; lookup_index++)
    {
      lookup_value = histo_scale*(lookup_index - histo_offset);
      index = initial_index;
      diff_value = abs(lookup_value-histo_eq_map[index]);
      index++;
      while ((index < histo_eq_size) && (diff_value >= abs(lookup_value-histo_eq_map[index])))
      {
        diff_value = abs(lookup_value-histo_eq_map[index++]);
      }
      index--;
      histo_lookup[lookup_index] = index;
//cout << "histo_lookup[" << lookup_index << "] = " << (int) histo_lookup[lookup_index] << " and lookup_value = " << lookup_value << endl;
      initial_index = index;
    }

    return;
  }

  double Image::get_histo_lookup(const double& lookup_value, const int& band)
  { 
    int lookup_index, lookup_nb_levels;

    if (histo_eq_map_flag[band])
    {
      lookup_nb_levels = histo_lookup[band].size();
      lookup_index = (int) offset_for_rounding((lookup_value/histo_scale[band]) + histo_offset[band]);
      if (lookup_index < 0)
        lookup_index = 0;
      if (lookup_index >= lookup_nb_levels)
        lookup_index = lookup_nb_levels - 1;

      return histo_lookup[band][lookup_index];
    }
    else
      return 0.0;
  }

  void Image::copy_histo_lookup(const int& band, const Image& sourceImage, const int& sourceBand)
  { 
    int lookup_index, lookup_nb_levels;

    lookup_nb_levels = sourceImage.histo_lookup[sourceBand].size();
    histo_lookup[band].resize(lookup_nb_levels);

    histo_scale[band] = sourceImage.histo_scale[sourceBand];
    histo_offset[band] = sourceImage.histo_offset[sourceBand];

    if (sourceImage.histo_eq_map_flag[sourceBand])
    {
      histo_eq_map_flag[band] = true;
      for (lookup_index = 0; lookup_index < lookup_nb_levels; lookup_index++)
        histo_lookup[band][lookup_index] = sourceImage.histo_lookup[sourceBand][lookup_index];
    }
  }

// Perform Histogram Equalization
  bool Image::performHistoEq(const int& band, Image& sourceImage)
  {
    Image maskImage;
    bool status = performHistoEq(band,sourceImage,maskImage);
    return status;
  }

// October 18, 2010: Rewrote to allow for non-spatially coincident images
  bool Image::performHistoEq(const int& band, Image& sourceImage, Image& maskImage)
  {
    bool mask_flag, coincident_flag = true;
    int col, row;
    double data_value;

   // Make sure all images are spatially coincident
    if ((geotransform_flag) && (sourceImage.geotransform_flag) && (maskImage.geotransform_flag))
    {
      if ((X_offset != sourceImage.X_offset) || (X_offset != maskImage.X_offset))
       coincident_flag = false;
      if ((Y_offset != sourceImage.Y_offset) || (Y_offset != maskImage.Y_offset))
       coincident_flag = false;
      if ((X_gsd != sourceImage.X_gsd) || (X_gsd != maskImage.X_gsd))
       coincident_flag = false;
      if ((Y_gsd != sourceImage.Y_gsd) || (Y_gsd != maskImage.Y_gsd))
       coincident_flag = false;
    }
    else
    {
      cout << "WARNING(Image::performHistoEq): Spatial coincidence cannot be confirmed\n" << endl;
    }

    if (coincident_flag)
    {
      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          if (maskImage.data_valid())
          {
            mask_flag = maskImage.data_valid(col,row,0);
          }
          else
            mask_flag = true;
          if (mask_flag && (sourceImage.data_valid(col,row,band)))
          {
            data_value = sourceImage.get_data(col,row,band);
            if (sourceImage.histo_eq_map_flag[band])
              data_value = sourceImage.get_histo_lookup(data_value,band);
            put_data(data_value,col,row,band);
          }
        }
    }
    else
    {
      double UTM_X, UTM_Y;
      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          UTM_X = X_offset + col*X_gsd;
          UTM_Y = Y_offset + row*Y_gsd;
          if (maskImage.data_valid())
          {
            mask_flag = maskImage.data_valid(UTM_X,UTM_Y,0);
          }
          else
            mask_flag = true;
          if (mask_flag && (sourceImage.data_valid(UTM_X,UTM_Y,band)))
          {
            data_value = sourceImage.get_data(UTM_X,UTM_Y,band);
            if (sourceImage.histo_eq_map_flag[band])
              data_value = sourceImage.get_histo_lookup(data_value,band);
            put_data(data_value,UTM_X,UTM_Y,band);
          }
        }
    }

    flush_data();

    return true;
  }

  void Image::resize_colormap(const unsigned int& colormap_size)
  {
    colormap.resize(colormap_size);
    colormap_flag = true;

    return;
  }

  void Image::set_colormap_value(const unsigned int &index, const unsigned int& red_value,
                                 const unsigned int& green_value, const unsigned int& blue_value)
  {
    colormap[index].r = red_value;
    colormap[index].g = green_value;
    colormap[index].b = blue_value;
    colormap[index].sum = red_value + green_value + blue_value;

    return;
  }

  void Image::compute_colormap(const unsigned int& colormap_size)
  {
    int   index, index2;
    short unsigned int numColors, numSteps, stepSize;

    colormap.resize(colormap_size);

    numSteps = 1;
    numColors = 1;
    while (numColors < colormap_size)
    {
      numSteps++;
      numColors = numSteps*numSteps*numSteps;
    }
    stepSize = 256/numSteps;
    if (stepSize == 0)
      stepSize = 1;
    ColorMap *colormap_heap;
    colormap_heap = new ColorMap[numColors+1];
    index = 0;
    colormap_heap[index].r = 0;
    colormap_heap[index].g = 0;
    colormap_heap[index].b = 0;
    colormap_heap[index].sum = 0;
    index = 1;
    short int redIndex, greenIndex, blueIndex;
    for (redIndex = (stepSize-1); redIndex < 256; redIndex += stepSize)
    {
      for (greenIndex = (stepSize-1); greenIndex < 256; greenIndex += stepSize)
      {
        for (blueIndex = (stepSize-1); blueIndex < 256; blueIndex += stepSize)
        {
          colormap_heap[index].r = redIndex;
          colormap_heap[index].g = greenIndex;
          colormap_heap[index].b = blueIndex;
          colormap_heap[index].sum = redIndex + greenIndex + blueIndex;
          push_heap(&colormap_heap[0],&colormap_heap[index],ColorLessThan());
          index++;
          if (index == (int) (numColors + 1))
            break;
        }
        if (index == (int) (numColors + 1))
          break;
      }
      if (index == (int) (numColors + 1))
        break;
    }

    int start_index = numColors;
    index = colormap_size-1;
    for (index2 = start_index; index2 >= 0; index2--)
    {
      pop_heap(&colormap_heap[0],&colormap_heap[index2],ColorLessThan());
      colormap[index].r = colormap_heap[index2].r;
      colormap[index].g = colormap_heap[index2].g;
      colormap[index].b = colormap_heap[index2].b;
      colormap[index].sum = colormap_heap[index2].sum;
      index--;
      if (index == 0)
        break;
    }
    colormap[index].r = 0;
    colormap[index].g = 0;
    colormap[index].b = 0;
    colormap[index].sum = 0;

    colormap_flag = true;

    return;
  }

  unsigned char Image::get_red_colormap(const unsigned int& data_value)
  {
    unsigned int  factor, remainder;

    if (data_value > MAX_COLORMAP_SIZE)
    {
      factor = data_value/MAX_COLORMAP_SIZE;
      factor = factor*MAX_COLORMAP_SIZE;
      remainder = data_value - factor;
    }
    else
      remainder = data_value;
  
    return colormap[remainder].r;
  }

  unsigned char Image::get_green_colormap(const unsigned int& data_value)
  {
    unsigned int  factor, remainder;

    if (data_value > MAX_COLORMAP_SIZE)
    {
      factor = data_value/MAX_COLORMAP_SIZE;
      factor = factor*MAX_COLORMAP_SIZE;
      remainder = data_value - factor;
    }
    else
      remainder = data_value;
  
    return colormap[remainder].g;
  }

  unsigned char Image::get_blue_colormap(const unsigned int& data_value)
  {
    unsigned int  factor, remainder;

    if (data_value > MAX_COLORMAP_SIZE)
    {
      factor = data_value/MAX_COLORMAP_SIZE;
      factor = factor*MAX_COLORMAP_SIZE;
      remainder = data_value - factor;
    }
    else
      remainder = data_value;
  
    return colormap[remainder].b;
  }

  void Image::copy_colormap(const Image& sourceImage)
  {
    unsigned int index, colormap_size = sourceImage.colormap.size();

    colormap.resize(colormap_size);
    colormap_flag = true;

    for (index = 0; index < colormap_size; index++)
    {
      colormap[index].r = sourceImage.colormap[index].r;
      colormap[index].g = sourceImage.colormap[index].g;
      colormap[index].b = sourceImage.colormap[index].b;
      colormap[index].sum = sourceImage.colormap[index].sum;
    }

#ifdef GDAL
    GDALColorTable colorTable(GPI_RGB);
    GDALColorEntry *colorEntry;
    colorEntry = new GDALColorEntry;
   
    for (index = 0; index < colormap_size; index++)
    {
      colorEntry->c1 = colormap[index].r;
      colorEntry->c2 = colormap[index].g;
      colorEntry->c3 = colormap[index].b;
      colorEntry->c4 = 255;
      colorTable.SetColorEntry(index,colorEntry);
    }
    imageDataset->GetRasterBand(1)->SetColorTable(&colorTable);
#endif

    return;
  }

  void Image::print_colormap()
  {
    int index, colormap_size = colormap.size();

    cout << "Colormap:" << endl;
    for (index = 0; index < colormap_size; index++)
    {
      cout << "At index " << index << ", r = " << (int) colormap[index].r << ", g = " << (int) colormap[index].g;
      cout << ", b = " << (int) colormap[index].b << " and sum = " << (int) colormap[index].sum << endl;
    }

    return;
  }

  void Image::upsample_copy(Image& sourceImage, const double& continuity_ratio)
  {
    int col, source_col, row, source_row, band;
    double UTM_X, UTM_Y, source_X, source_Y;

    for (band = 0; band < nbands; band++)
      if (sourceImage.no_data_value_flag[band])
      {
        no_data_value[band] = sourceImage.no_data_value[band];
        no_data_value_flag[band] = true;
      }
#ifdef GDAL
    put_no_data_value();
#endif

  // Populate even rows and columns
    for (row = 0; row < nrows; row += 2)
      for (col = 0; col < ncols; col += 2)
      {
        UTM_X = X_offset + col*X_gsd;
        UTM_Y = Y_offset + row*Y_gsd;
        source_X = (UTM_X - sourceImage.X_offset)/sourceImage.X_gsd;
        source_Y = (UTM_Y - sourceImage.Y_offset)/sourceImage.Y_gsd;
        source_col = (int) offset_for_rounding(source_X);
        source_row = (int) offset_for_rounding(source_Y);
        for (band = 0; band < nbands; band++)
        {
          put_data(sourceImage.get_data(source_col,source_row,band),col,row,band);
        }
      }
    flush_data();

    interpolate(continuity_ratio);

    return;
  }

  void Image::scale_offset(const float& scaled_min, const float& scaled_std_dev)
  {
    Image maskImage;

    scale_offset(scaled_min, scaled_std_dev, maskImage);

    return;
  }

  void Image::scale_offset(const float& scaled_min, const float& scaled_std_dev, Image& maskImage)
  {
    int col, row, band, npixels;
    float orig_value, orig_min, orig_mean, orig_std_dev;
    float sum_values, sumsq_values, scale, offset, scaled_value;

    for (band = 0; band < nbands; band++)
    {
      npixels = 0;
      orig_min = FLT_MAX;
      sum_values = 0.0;
      sumsq_values = 0.0;
      for (row = 0; row < nrows; row ++)
        for (col = 0; col < ncols; col ++)
        {
          if (maskImage.data_valid(col,row,0))
          {
            npixels++;
            orig_value = get_data(col,row,band);
            if (orig_min > orig_value)
              orig_min = orig_value;
            sum_values += orig_value;
            sumsq_values += orig_value*orig_value;
          }
        }
      cout << endl << "For Original Data:" << endl;
      orig_mean = sum_values/((double) npixels);
      orig_std_dev = (sum_values*sum_values)/((double) npixels);
      orig_std_dev = sumsq_values - orig_std_dev;
      orig_std_dev = orig_std_dev/(((double) npixels)-1.0);
      orig_std_dev = sqrt(orig_std_dev);
      cout << "For band " << band << ", minimum = " << orig_min;
      cout << ", mean = " << orig_mean;
      cout << ", and standard deviation = " << orig_std_dev << endl;
      scale = (1.0/orig_std_dev);
      offset = orig_min;
      cout << "scale = " << scale << " and offset = " << offset << endl;

      flush_data();

      for (row = 0; row < nrows; row ++)
        for (col = 0; col < ncols; col ++)
        {
          if (maskImage.data_valid(col,row,0))
          {
            orig_value = get_data(col,row,band);
            scaled_value = scale*(orig_value - offset);
            put_data(scaled_value,col,row,band);
          }
        }
      flush_data();

      npixels = 0;
      orig_min = FLT_MAX;
      sum_values = 0.0;
      sumsq_values = 0.0;
      for (row = 0; row < nrows; row ++)
        for (col = 0; col < ncols; col ++)
        {
          if (maskImage.data_valid(col,row,0))
          {
            npixels++;
            orig_value = get_data(col,row,band);
            if (orig_min > orig_value)
              orig_min = orig_value;
            sum_values += orig_value;
            sumsq_values += orig_value*orig_value;
          }
        }
      cout << endl << "For Scaled Data:" << endl;
      orig_mean = sum_values/((double) npixels);
      orig_std_dev = (sum_values*sum_values)/((double) npixels);
      orig_std_dev = sumsq_values - orig_std_dev;
      orig_std_dev = orig_std_dev/(((double) npixels)-1.0);
      orig_std_dev = sqrt(orig_std_dev);
      cout << "For band " << band << ", minimum = " << orig_min;
      cout << ", mean = " << orig_mean;
      cout << ", and standard deviation = " << orig_std_dev << endl;
    }
    resetMinMax();

    return;
  }

  void Image::scale_offset(const int& new_min, const int& new_max, const int& band, Image& maskImage)
  {
    int col, row;
    double old_min, old_max, scale, offset, scaled_value;

    resetMinMax();
    old_min = getMinimum(band,maskImage);
    old_max = getMaximum(band,maskImage);
    scale = (new_max-new_min)/(old_max-old_min);
    offset = scale*old_min - new_min;

    no_data_value[band] = new_min - 1;
    no_data_value_flag[band] = true;

    for (row = 0; row < nrows; row ++)
      for (col = 0; col < ncols; col ++)
      {
        if (maskImage.data_valid(col,row,0))
        {
          scaled_value = get_data(col,row,band);
          scaled_value = scale*scaled_value - offset;
          put_data(scaled_value,col,row,band);
        }
        else
          put_data(no_data_value[band],col,row,band);
      }
    flush_data();
    resetMinMax();

    return;
  }

  void Image::scale_offset_SI(Image& SI_Image, const float SI_scale, const float SI_offset, const int& SI_band,
                              const int& min_valid, const int& max_valid)
  {
    int col, row;
    double scaled_value;

    if (!no_data_value_flag[SI_band])
    {
      no_data_value[SI_band] = -1;
      no_data_value_flag[SI_band] = true;
    }

    for (row = 0; row < nrows; row ++)
      for (col = 0; col < ncols; col ++)
      {
        scaled_value = SI_Image.get_data(col,row,SI_band);
        if ((scaled_value >= min_valid) && (scaled_value <= max_valid))
          scaled_value = (scaled_value - SI_offset)*SI_scale;
        else
          scaled_value = no_data_value[SI_band];
        put_data(scaled_value,col,row,0);
      }
    flush_data();

    return;
  }

  void Image::chi_square_ratio(Image& posChiSquareImage, Image& negChiSquareImage, const double& chi_square_threshold,
                               Image& dofImage, const int& dof_threshold)
  {
    int col, row, band;
    double pos_chi_square, neg_chi_square, dof, ratio;

    for (band = 0; band < nbands; band++)
     for (row = 0; row < nrows; row ++)
      for (col = 0; col < ncols; col ++)
      {
        pos_chi_square = posChiSquareImage.get_data(col,row,band);
        neg_chi_square = negChiSquareImage.get_data(col,row,band);
        dof = dofImage.get_data(col,row,band);
        ratio = 0.0;
        if ((dof >= dof_threshold) &&
            (pos_chi_square >= chi_square_threshold) &&
            (neg_chi_square >= chi_square_threshold))
        {
          ratio = pos_chi_square/neg_chi_square;
        } 
        put_data(ratio,col,row,band);
      }
    flush_data();

    return;
  }

#ifdef MODIS
  void Image::copy_swath_lat_long_data(Image& swathLatitudeImage, Image& swathLongitudeImage)
  {
    int col, row;
    double data_value;
    
    no_data_value[0] = no_data_value[1] = LAT_LONG_FILL;
    no_data_value_flag[0] = no_data_value_flag[1] = true;
    put_no_data_value();

    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        data_value = swathLatitudeImage.get_data(col,row,0);
        if (((int) data_value) == LAT_LONG_FILL)
          put_data(no_data_value[0],col,row,0);
        else
          put_data(data_value,col,row,0);
        data_value = swathLongitudeImage.get_data(col,row,0);
        if (((int) data_value) == LAT_LONG_FILL)
          put_data(no_data_value[1],col,row,1);
        else
          put_data(data_value,col,row,1);
      }
    flush_data();

    return;
  }

  void Image::copy_proj_lat_long_data(Image& projLatitudeImage, Image& projLongitudeImage)
  {
    int col, row;
    double data_value;
    
    no_data_value[0] = no_data_value[1] = LAT_LONG_FILL;
    no_data_value_flag[0] = no_data_value_flag[1] = true;
    put_no_data_value();

    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        data_value = projLatitudeImage.get_data(col,row,0);
        data_value /= LATITUDE_SCALE;
        if ((data_value == 0.0) || (((int) data_value) == LAT_LONG_FILL))
          put_data(no_data_value[0],col,row,0);
        else
          put_data(data_value,col,row,0);
        data_value = projLongitudeImage.get_data(col,row,0);
        data_value /= LONGITUDE_SCALE;
        if ((data_value == 0.0) || (((int) data_value) == LAT_LONG_FILL))
          put_data(no_data_value[1],col,row,1);
        else
          put_data(data_value,col,row,1);
      }
    flush_data();

    return;
  }

  void Image::NNProjColRow(Image& swathLatLongImage, Image& projLatLongImage)
  {
    bool continue_flag;
    int col, row, swath_col, swath_row, check_col, check_row, best_col, best_row;
    int swath_ncols, swath_nrows, delta, iterations;
    double latitude_value, longitude_value;
    double lat_diff, long_diff, min_lat_diff, min_long_diff;
    double lat_long_diff_sum, max_lat_long_diff_sum = 0;

    swath_ncols = swathLatLongImage.get_ncols();
    swath_nrows = swathLatLongImage.get_nrows();

    swath_row = 0;
    for (row = 0; row < nrows; row++)
    {
      swath_col = 0;
      for (col = 0; col < ncols; col++)
      {
        if (projLatLongImage.data_valid(col,row,0))
        {
          latitude_value = projLatLongImage.get_data(col,row,0);
          longitude_value = projLatLongImage.get_data(col,row,1);
         // Search in swathLatLongImage for location of closest match
          min_lat_diff = fabs(latitude_value - swathLatLongImage.get_data(swath_col,swath_row,0));
          min_long_diff = fabs(longitude_value - swathLatLongImage.get_data(swath_col,swath_row,1));
          lat_long_diff_sum = min_lat_diff + min_long_diff;
          delta = (int) (10.0*lat_long_diff_sum);
          if (delta < 10)
            delta = 10;
          if (delta > 100);
            delta = 100;
          iterations = 0;
          continue_flag = false;
          if ((min_lat_diff != 0.0) || (min_long_diff != 0.0))
            continue_flag = true;
          while (continue_flag)
          {
            iterations++;
            continue_flag = false;
            best_row = swath_row; best_col = swath_col;
            if (min_lat_diff > min_long_diff)
            {
              // Check north and south
              for (check_row = (swath_row - delta); check_row < (swath_row + delta); check_row++)
              { 
                if (swathLatLongImage.data_valid(swath_col,check_row,0))
                {
                  lat_diff = fabs(latitude_value - swathLatLongImage.get_data(swath_col,check_row,0));
                  long_diff = fabs(longitude_value - swathLatLongImage.get_data(swath_col,check_row,1));
                  if ((lat_diff < min_lat_diff) && (long_diff < min_lat_diff))
                  {
                    min_lat_diff = lat_diff;
                    min_long_diff = long_diff;
                    best_row = check_row;
                    continue_flag = true;
                  }
                }
              }
            }
            else
            {
              // Check east and west
              for (check_col = (swath_col - delta); check_col < (swath_col + delta); check_col++)
              { 
                if (swathLatLongImage.data_valid(check_col,swath_row,1))
                {
                  lat_diff = fabs(latitude_value - swathLatLongImage.get_data(check_col,swath_row,0));
                  long_diff = fabs(longitude_value - swathLatLongImage.get_data(check_col,swath_row,1));
                  if ((long_diff < min_long_diff) && (lat_diff < min_long_diff))
                  {
                    min_lat_diff = lat_diff;
                    min_long_diff = long_diff;
                    best_col = check_col;
                    continue_flag = true;
                  }
                }
              }
            }
            if (!continue_flag) // Force a check the other direction!
            {
              if (min_lat_diff < min_long_diff)
              {
                // Check north and south
                for (check_row = (swath_row - delta); check_row < (swath_row + delta); check_row++)
                { 
                  if (swathLatLongImage.data_valid(swath_col,check_row,0))
                  {
                    lat_diff = fabs(latitude_value - swathLatLongImage.get_data(swath_col,check_row,0));
                    long_diff = fabs(longitude_value - swathLatLongImage.get_data(swath_col,check_row,1));
                    if ((lat_diff < min_lat_diff) && (long_diff < min_lat_diff))
                    {
                      min_lat_diff = lat_diff;
                      min_long_diff = long_diff;
                      best_row = check_row;
                      continue_flag = true;
                    }
                  }
                }
              }
              else
              {
                // Check east and west
                for (check_col = (swath_col - delta); check_col < (swath_col + delta); check_col++)
                { 
                  if (swathLatLongImage.data_valid(check_col,swath_row,1))
                  {
                    lat_diff = fabs(latitude_value - swathLatLongImage.get_data(check_col,swath_row,0));
                    long_diff = fabs(longitude_value - swathLatLongImage.get_data(check_col,swath_row,1));
                    if ((long_diff < min_long_diff) && (lat_diff < min_long_diff))
                    {
                      min_lat_diff = lat_diff;
                      min_long_diff = long_diff;
                      best_col = check_col;
                      continue_flag = true;
                    }
                  }
                }
              }  
            }
            swath_row = best_row; swath_col = best_col;
            min_lat_diff = fabs(latitude_value - swathLatLongImage.get_data(swath_col,swath_row,0));
            min_long_diff = fabs(longitude_value - swathLatLongImage.get_data(swath_col,swath_row,1));
            lat_long_diff_sum = min_lat_diff + min_long_diff;
            delta = (int) (10.0*lat_long_diff_sum);
            if (delta < 10)
              delta = 10;
            if (delta > 100);
              delta = 100;
            if (iterations > 50)
              continue_flag = false;
          }
          if (lat_long_diff_sum > 0.05)
          {
            cout << "WARNING: Final lat_long_diff_sum = " << lat_long_diff_sum << " at (swath_col,swath_row) = (";
            cout << swath_col << "," << swath_row << ")" << endl;
            cout << "and (col,row) = (" << col << "," << row << ")" << ". Iterations = " << iterations << endl;
            cout << "   min_lat_diff = " << min_lat_diff << " and min_long_diff = " << min_long_diff << endl;
          }
          if (lat_long_diff_sum > max_lat_long_diff_sum)
            max_lat_long_diff_sum = lat_long_diff_sum;
          put_data(swath_col,col,row,0);
          put_data(swath_row,col,row,1);
        }
        else
        {
          if (no_data_value_flag[0])
            put_data(no_data_value[0],col,row,0);
          else
            put_data(0,col,row,0);
          if (no_data_value_flag[1])
            put_data(no_data_value[1],col,row,1);
          else
            put_data(0,col,row,1);
        }
      }
    }
    flush_data();

cout << "Quality Check: In NNProjColRow, max_lat_long_diff_sum = " << max_lat_long_diff_sum << endl;

    return;
  }

  void Image::NNProj(Image& swathDataImage, Image& projColRowImage)
  {
    int band, col, row, swath_col, swath_row;

    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        if (projColRowImage.data_valid(col,row,0))
        {
          swath_col = (int) projColRowImage.get_data(col,row,0);
          swath_row = (int) projColRowImage.get_data(col,row,1);
          for (band = 0; band < nbands; band++)
            put_data(swathDataImage.get_data(swath_col,swath_row,band),col,row,band);
        }
        else
        {
          for (band = 0; band < nbands; band++)
            put_data(get_no_data_value(band),col,row,band);
        }
      }
    }
    flush_data();

    return;
  }

  void Image::modis_scale_and_correct(Image& inputImage, const int& input_band, const float& scale, const float& offset,
                                      Image& solarZenithImage, const int& output_band)
  {
    int col, row, solar_col, solar_row;
    double UTM_X, UTM_Y, solar_X, solar_Y;
    double data_value, Solar_Zenith_Angle, Cos_Solar_Zenith_Angle;
    no_data_value[output_band] = -1;
    no_data_value_flag[output_band] = true;

    for (row = 0; row < nrows; row++)
     for (col = 0; col < ncols; col++)
     {
       data_value = inputImage.get_data(col,row,input_band);
       if (data_value > MAX_USABLE_L1B)
         put_data(no_data_value[output_band],col,row,output_band);
       else
       {
      // Compute the cosine of the Solar Zenith Angle
         if (geotransform_flag && solarZenithImage.geotransform_valid())
         {
           UTM_X = X_offset + col*X_gsd;
           UTM_Y = Y_offset + row*Y_gsd;
           solar_X = (UTM_X - solarZenithImage.X_offset)/solarZenithImage.X_gsd;
           solar_Y = (UTM_Y - solarZenithImage.Y_offset)/solarZenithImage.Y_gsd;
           solar_col = (int) offset_for_rounding(solar_X);
           solar_row = (int) offset_for_rounding(solar_Y);
         }
         else
         {
           solar_col = col;
           solar_row = row;
         }
         Solar_Zenith_Angle = solarZenithImage.get_data(solar_col,solar_row,0)*SOLAR_ZENITH_SCALE;
         Cos_Solar_Zenith_Angle = cos(Solar_Zenith_Angle/RADIAN_CONV);
         data_value = (data_value - offset)*scale;
         data_value = data_value/Cos_Solar_Zenith_Angle;
         put_data(data_value,col,row,output_band);
       }
     } //for (col = 0; col < ncols; col++)
       //for (row = 0; row < nrows; row++)
    flush_data();

    return;
  }

  void Image::compute_surface_temperature(Image& inputImage, vector<float>& scale, vector<float>& offset,  
                                          const int& band_31, const int& band_32)
  {
    double cw31 = 1.103e-5;
    double cw32 = 1.202e-5;
    double cw31_5 = (cw31*cw31*cw31*cw31*cw31);
    double cw32_5 = (cw32*cw32*cw32*cw32*cw32);
    
    double c1 = 1.1910439e-16;	/* 2*h*(c**2) = W m**(-2)	*/
    double c2 = 1.4387686e-2;	/* (h*c)/k = m K		*/
   
    double aa[3], bb[3], cc[3], dd[3];
/*** Northern Hemisphere ***/
    aa[0]=-1.5711228087; bb[0]=1.0054774067; cc[0]=1.8532794923; dd[0]=-0.7905176303; /* < 240   */
    aa[1]=-2.3726968515; bb[1]=1.0086040702; cc[1]=1.6948238801; dd[1]=-0.2052523236; /* 240-260 */
    aa[2]=-4.2953046345; bb[2]=1.0150179031; cc[2]=1.9495254583; dd[2]=0.1971325790; /* > 260   */

/*** Southern Hemisphere ***/
//   aa[0]=-0.1594802497; bb[0]=0.9999256454; cc[0]=1.3903881106; dd[0]=-0.4135749071; /* < 240   */
//   aa[1]=-3.3294560023, bb[1]=1.0129459037; cc[1]=1.2145725772; dd[1]=0.1310171301; /* 240-260 */
//   aa[2]=-5.2073604160; bb[2]=1.0194285947; cc[2]=1.5102495616; dd[2]=0.2603553496; /* > 260   */

    double secantThetaMinusOne[ST_NCOLS] = {
    0.743447, 0.739925, 0.736421, 0.732935, 0.729466, 0.726014, 0.722580, 0.719163,
    0.715763, 0.712379, 0.709012, 0.705662, 0.702329, 0.699012, 0.695711, 0.692426,
    0.689157, 0.685905, 0.682668, 0.679447, 0.676242, 0.673052, 0.669878, 0.666719,
    0.663576, 0.660447, 0.657334, 0.654236, 0.651152, 0.648083, 0.645030, 0.641990,
    0.638965, 0.635955, 0.632959, 0.629977, 0.627009, 0.624055, 0.621115, 0.618189,
    0.615277, 0.612378, 0.609494, 0.606622, 0.603764, 0.600920, 0.598089, 0.595271,
    0.592466, 0.589674, 0.586895, 0.584129, 0.581375, 0.578635, 0.575907, 0.573192,
    0.570489, 0.567798, 0.565120, 0.562455, 0.559801, 0.557159, 0.554530, 0.551913,
    0.549307, 0.546713, 0.544132, 0.541561, 0.539003, 0.536456, 0.533920, 0.531396,
    0.528883, 0.526382, 0.523892, 0.521413, 0.518945, 0.516488, 0.514042, 0.511607,
    0.509182, 0.506769, 0.504366, 0.501974, 0.499593, 0.497222, 0.494862, 0.492512,
    0.490172, 0.487843, 0.485524, 0.483215, 0.480917, 0.478628, 0.476349, 0.474081,
    0.471822, 0.469573, 0.467334, 0.465105, 0.462885, 0.460675, 0.458475, 0.456284,
    0.454103, 0.451931, 0.449769, 0.447616, 0.445472, 0.443337, 0.441212, 0.439096,
    0.436988, 0.434890, 0.432801, 0.430721, 0.428650, 0.426587, 0.424534, 0.422489,
    0.420452, 0.418425, 0.416406, 0.414396, 0.412394, 0.410401, 0.408416, 0.406440,
    0.404472, 0.402512, 0.400560, 0.398617, 0.396682, 0.394755, 0.392836, 0.390926,
    0.389023, 0.387128, 0.385241, 0.383363, 0.381492, 0.379628, 0.377773, 0.375925,
    0.374085, 0.372253, 0.370428, 0.368611, 0.366802, 0.365000, 0.363205, 0.361418,
    0.359639, 0.357866, 0.356101, 0.354344, 0.352593, 0.350850, 0.349114, 0.347386,
    0.345664, 0.343949, 0.342242, 0.340541, 0.338848, 0.337161, 0.335482, 0.333809,
    0.332143, 0.330484, 0.328832, 0.327187, 0.325548, 0.323916, 0.322290, 0.320672,
    0.319060, 0.317454, 0.315855, 0.314263, 0.312677, 0.311097, 0.309524, 0.307957,
    0.306397, 0.304843, 0.303295, 0.301754, 0.300219, 0.298690, 0.297167, 0.295650,
    0.294140, 0.292636, 0.291137, 0.289645, 0.288159, 0.286679, 0.285205, 0.283737,
    0.282274, 0.280818, 0.279367, 0.277923, 0.276484, 0.275051, 0.273624, 0.272202,
    0.270786, 0.269376, 0.267972, 0.266573, 0.265180, 0.263792, 0.262410, 0.261034,
    0.259663, 0.258298, 0.256938, 0.255583, 0.254234, 0.252891, 0.251552, 0.250220,
    0.248892, 0.247570, 0.246253, 0.244941, 0.243635, 0.242334, 0.241038, 0.239747,
    0.238462, 0.237181, 0.235906, 0.234636, 0.233371, 0.232111, 0.230856, 0.229606,
    0.228361, 0.227121, 0.225886, 0.224656, 0.223431, 0.222210, 0.220995, 0.219785,
    0.218579, 0.217378, 0.216182, 0.214991, 0.213804, 0.212623, 0.211446, 0.210274,
    0.209106, 0.207943, 0.206785, 0.205631, 0.204482, 0.203338, 0.202198, 0.201063,
    0.199932, 0.198806, 0.197685, 0.196568, 0.195455, 0.194347, 0.193243, 0.192144,
    0.191049, 0.189959, 0.188872, 0.187791, 0.186713, 0.185640, 0.184572, 0.183507,
    0.182447, 0.181391, 0.180340, 0.179292, 0.178249, 0.177210, 0.176176, 0.175145,
    0.174119, 0.173097, 0.172078, 0.171064, 0.170055, 0.169049, 0.168047, 0.167049,
    0.166056, 0.165066, 0.164081, 0.163099, 0.162122, 0.161148, 0.160178, 0.159213,
    0.158251, 0.157293, 0.156339, 0.155389, 0.154443, 0.153501, 0.152562, 0.151628,
    0.150697, 0.149770, 0.148847, 0.147927, 0.147012, 0.146100, 0.145192, 0.144287,
    0.143387, 0.142490, 0.141597, 0.140707, 0.139821, 0.138939, 0.138061, 0.137186,
    0.136314, 0.135447, 0.134583, 0.133722, 0.132865, 0.132012, 0.131162, 0.130316,
    0.129473, 0.128634, 0.127798, 0.126966, 0.126138, 0.125313, 0.124491, 0.123672,
    0.122858, 0.122046, 0.121238, 0.120434, 0.119633, 0.118835, 0.118041, 0.117250,
    0.116462, 0.115678, 0.114897, 0.114119, 0.113345, 0.112574, 0.111806, 0.111042,
    0.110281, 0.109523, 0.108768, 0.108017, 0.107269, 0.106524, 0.105783, 0.105044,
    0.104309, 0.103577, 0.102848, 0.102123, 0.101400, 0.100681, 0.099965, 0.099252,
    0.098542, 0.097835, 0.097131, 0.096431, 0.095733, 0.095039, 0.094348, 0.093659,
    0.092974, 0.092292, 0.091613, 0.090937, 0.090264, 0.089594, 0.088927, 0.088263,
    0.087602, 0.086944, 0.086289, 0.085637, 0.084988, 0.084342, 0.083699, 0.083058,
    0.082421, 0.081787, 0.081155, 0.080527, 0.079901, 0.079278, 0.078658, 0.078041,
    0.077427, 0.076816, 0.076208, 0.075602, 0.074999, 0.074399, 0.073802, 0.073208,
    0.072617, 0.072028, 0.071442, 0.070859, 0.070279, 0.069701, 0.069127, 0.068555,
    0.067986, 0.067419, 0.066855, 0.066294, 0.065736, 0.065181, 0.064628, 0.064078,
    0.063531, 0.062986, 0.062444, 0.061905, 0.061368, 0.060834, 0.060303, 0.059774,
    0.059248, 0.058725, 0.058204, 0.057686, 0.057171, 0.056658, 0.056148, 0.055641,
    0.055136, 0.054633, 0.054134, 0.053637, 0.053142, 0.052650, 0.052161, 0.051674,
    0.051190, 0.050708, 0.050229, 0.049752, 0.049278, 0.048807, 0.048337, 0.047871,
    0.047407, 0.046946, 0.046487, 0.046030, 0.045576, 0.045125, 0.044676, 0.044230,
    0.043786, 0.043344, 0.042905, 0.042469, 0.042034, 0.041603, 0.041173, 0.040747,
    0.040322, 0.039901, 0.039481, 0.039064, 0.038649, 0.038237, 0.037827, 0.037420,
    0.037015, 0.036613, 0.036212, 0.035815, 0.035419, 0.035026, 0.034636, 0.034247,
    0.033862, 0.033478, 0.033097, 0.032718, 0.032342, 0.031968, 0.031596, 0.031226,
    0.030859, 0.030494, 0.030132, 0.029772, 0.029414, 0.029059, 0.028706, 0.028355,
    0.028006, 0.027660, 0.027316, 0.026974, 0.026635, 0.026298, 0.025963, 0.025631,
    0.025301, 0.024972, 0.024647, 0.024323, 0.024002, 0.023683, 0.023367, 0.023052,
    0.022740, 0.022430, 0.022123, 0.021817, 0.021514, 0.021213, 0.020914, 0.020618,
    0.020324, 0.020032, 0.019742, 0.019454, 0.019169, 0.018885, 0.018605, 0.018326,
    0.018049, 0.017775, 0.017503, 0.017233, 0.016965, 0.016699, 0.016436, 0.016174,
    0.015915, 0.015658, 0.015404, 0.015151, 0.014901, 0.014652, 0.014406, 0.014162,
    0.013920, 0.013681, 0.013443, 0.013208, 0.012975, 0.012744, 0.012515, 0.012288,
    0.012063, 0.011841, 0.011621, 0.011402, 0.011186, 0.010972, 0.010760, 0.010550,
    0.010343, 0.010137, 0.009934, 0.009733, 0.009534, 0.009336, 0.009141, 0.008949,
    0.008758, 0.008569, 0.008383, 0.008198, 0.008016, 0.007836, 0.007657, 0.007481,
    0.007307, 0.007135, 0.006965, 0.006798, 0.006632, 0.006468, 0.006307, 0.006147,
    0.005990, 0.005834, 0.005681, 0.005530, 0.005381, 0.005234, 0.005089, 0.004946,
    0.004805, 0.004666, 0.004530, 0.004395, 0.004262, 0.004132, 0.004003, 0.003877,
    0.003752, 0.003630, 0.003510, 0.003391, 0.003275, 0.003161, 0.003049, 0.002939,
    0.002830, 0.002724, 0.002620, 0.002518, 0.002418, 0.002321, 0.002225, 0.002131,
    0.002039, 0.001949, 0.001862, 0.001776, 0.001692, 0.001611, 0.001531, 0.001453,
    0.001378, 0.001304, 0.001233, 0.001163, 0.001096, 0.001030, 0.000967, 0.000905,
    0.000846, 0.000789, 0.000733, 0.000680, 0.000629, 0.000579, 0.000532, 0.000487,
    0.000443, 0.000402, 0.000363, 0.000326, 0.000291, 0.000257, 0.000226, 0.000197,
    0.000170, 0.000145, 0.000122, 0.000100, 0.000081, 0.000064, 0.000049, 0.000036,
    0.000025, 0.000016, 0.000009, 0.000004, 0.000001, 0.000001, 0.000004, 0.000009,
    0.000016, 0.000025, 0.000036, 0.000049, 0.000064, 0.000081, 0.000100, 0.000122,
    0.000145, 0.000170, 0.000197, 0.000226, 0.000257, 0.000291, 0.000326, 0.000363,
    0.000402, 0.000443, 0.000487, 0.000532, 0.000579, 0.000629, 0.000680, 0.000733,
    0.000789, 0.000846, 0.000905, 0.000967, 0.001030, 0.001096, 0.001163, 0.001233,
    0.001304, 0.001378, 0.001453, 0.001531, 0.001611, 0.001692, 0.001776, 0.001862,
    0.001949, 0.002039, 0.002131, 0.002225, 0.002321, 0.002418, 0.002518, 0.002620,
    0.002724, 0.002830, 0.002939, 0.003049, 0.003161, 0.003275, 0.003391, 0.003510,
    0.003630, 0.003752, 0.003877, 0.004003, 0.004132, 0.004262, 0.004395, 0.004530,
    0.004666, 0.004805, 0.004946, 0.005089, 0.005234, 0.005381, 0.005530, 0.005681,
    0.005834, 0.005990, 0.006147, 0.006307, 0.006468, 0.006632, 0.006798, 0.006965,
    0.007135, 0.007307, 0.007481, 0.007657, 0.007836, 0.008016, 0.008198, 0.008383,
    0.008569, 0.008758, 0.008949, 0.009141, 0.009336, 0.009534, 0.009733, 0.009934,
    0.010137, 0.010343, 0.010550, 0.010760, 0.010972, 0.011186, 0.011402, 0.011621,
    0.011841, 0.012063, 0.012288, 0.012515, 0.012744, 0.012975, 0.013208, 0.013443,
    0.013681, 0.013920, 0.014162, 0.014406, 0.014652, 0.014901, 0.015151, 0.015404,
    0.015658, 0.015915, 0.016174, 0.016436, 0.016699, 0.016965, 0.017233, 0.017503,
    0.017775, 0.018049, 0.018326, 0.018605, 0.018885, 0.019169, 0.019454, 0.019742,
    0.020032, 0.020324, 0.020618, 0.020914, 0.021213, 0.021514, 0.021817, 0.022123,
    0.022430, 0.022740, 0.023052, 0.023367, 0.023683, 0.024002, 0.024323, 0.024647,
    0.024972, 0.025301, 0.025631, 0.025963, 0.026298, 0.026635, 0.026974, 0.027316,
    0.027660, 0.028006, 0.028355, 0.028706, 0.029059, 0.029414, 0.029772, 0.030132,
    0.030494, 0.030859, 0.031226, 0.031596, 0.031968, 0.032342, 0.032718, 0.033097,
    0.033478, 0.033862, 0.034247, 0.034636, 0.035026, 0.035419, 0.035815, 0.036212,
    0.036613, 0.037015, 0.037420, 0.037827, 0.038237, 0.038649, 0.039064, 0.039481,
    0.039901, 0.040322, 0.040747, 0.041173, 0.041603, 0.042034, 0.042469, 0.042905,
    0.043344, 0.043786, 0.044230, 0.044676, 0.045125, 0.045576, 0.046030, 0.046487,
    0.046946, 0.047407, 0.047871, 0.048337, 0.048807, 0.049278, 0.049752, 0.050229,
    0.050708, 0.051190, 0.051674, 0.052161, 0.052650, 0.053142, 0.053637, 0.054134,
    0.054633, 0.055136, 0.055641, 0.056148, 0.056658, 0.057171, 0.057686, 0.058204,
    0.058725, 0.059248, 0.059774, 0.060303, 0.060834, 0.061368, 0.061905, 0.062444,
    0.062986, 0.063531, 0.064078, 0.064628, 0.065181, 0.065736, 0.066294, 0.066855,
    0.067419, 0.067986, 0.068555, 0.069127, 0.069701, 0.070279, 0.070859, 0.071442,
    0.072028, 0.072617, 0.073208, 0.073802, 0.074399, 0.074999, 0.075602, 0.076208,
    0.076816, 0.077427, 0.078041, 0.078658, 0.079278, 0.079901, 0.080527, 0.081155,
    0.081787, 0.082421, 0.083058, 0.083699, 0.084342, 0.084988, 0.085637, 0.086289,
    0.086944, 0.087602, 0.088263, 0.088927, 0.089594, 0.090264, 0.090937, 0.091613,
    0.092292, 0.092974, 0.093659, 0.094348, 0.095039, 0.095733, 0.096431, 0.097131,
    0.097835, 0.098542, 0.099252, 0.099965, 0.100681, 0.101400, 0.102123, 0.102848,
    0.103577, 0.104309, 0.105044, 0.105783, 0.106524, 0.107269, 0.108017, 0.108768,
    0.109523, 0.110281, 0.111042, 0.111806, 0.112574, 0.113345, 0.114119, 0.114897,
    0.115678, 0.116462, 0.117250, 0.118041, 0.118835, 0.119633, 0.120434, 0.121238,
    0.122046, 0.122858, 0.123672, 0.124491, 0.125313, 0.126138, 0.126966, 0.127798,
    0.128634, 0.129473, 0.130316, 0.131162, 0.132012, 0.132865, 0.133722, 0.134583,
    0.135447, 0.136314, 0.137186, 0.138061, 0.138939, 0.139821, 0.140707, 0.141597,
    0.142490, 0.143387, 0.144287, 0.145192, 0.146100, 0.147012, 0.147927, 0.148847,
    0.149770, 0.150697, 0.151628, 0.152562, 0.153501, 0.154443, 0.155389, 0.156339,
    0.157293, 0.158251, 0.159213, 0.160178, 0.161148, 0.162122, 0.163099, 0.164081,
    0.165066, 0.166056, 0.167049, 0.168047, 0.169049, 0.170055, 0.171064, 0.172078,
    0.173097, 0.174119, 0.175145, 0.176176, 0.177210, 0.178249, 0.179292, 0.180340,
    0.181391, 0.182447, 0.183507, 0.184572, 0.185640, 0.186713, 0.187791, 0.188872,
    0.189959, 0.191049, 0.192144, 0.193243, 0.194347, 0.195455, 0.196568, 0.197685,
    0.198806, 0.199932, 0.201063, 0.202198, 0.203338, 0.204482, 0.205631, 0.206785,
    0.207943, 0.209106, 0.210274, 0.211446, 0.212623, 0.213804, 0.214991, 0.216182,
    0.217378, 0.218579, 0.219785, 0.220995, 0.222210, 0.223431, 0.224656, 0.225886,
    0.227121, 0.228361, 0.229606, 0.230856, 0.232111, 0.233371, 0.234636, 0.235906,
    0.237181, 0.238462, 0.239747, 0.241038, 0.242334, 0.243635, 0.244941, 0.246253,
    0.247570, 0.248892, 0.250220, 0.251552, 0.252891, 0.254234, 0.255583, 0.256938,
    0.258298, 0.259663, 0.261034, 0.262410, 0.263792, 0.265180, 0.266573, 0.267972,
    0.269376, 0.270786, 0.272202, 0.273624, 0.275051, 0.276484, 0.277923, 0.279367,
    0.280818, 0.282274, 0.283737, 0.285205, 0.286679, 0.288159, 0.289645, 0.291137,
    0.292636, 0.294140, 0.295650, 0.297167, 0.298690, 0.300219, 0.301754, 0.303295,
    0.304843, 0.306397, 0.307957, 0.309524, 0.311097, 0.312677, 0.314263, 0.315855,
    0.317454, 0.319060, 0.320672, 0.322290, 0.323916, 0.325548, 0.327187, 0.328832,
    0.330484, 0.332143, 0.333809, 0.335482, 0.337161, 0.338848, 0.340541, 0.342242,
    0.343949, 0.345664, 0.347386, 0.349114, 0.350850, 0.352593, 0.354344, 0.356101,
    0.357866, 0.359639, 0.361418, 0.363205, 0.365000, 0.366802, 0.368611, 0.370428,
    0.372253, 0.374085, 0.375925, 0.377773, 0.379628, 0.381492, 0.383363, 0.385241,
    0.387128, 0.389023, 0.390926, 0.392836, 0.394755, 0.396682, 0.398617, 0.400560,
    0.402512, 0.404472, 0.406440, 0.408416, 0.410401, 0.412394, 0.414396, 0.416406,
    0.418425, 0.420452, 0.422489, 0.424534, 0.426587, 0.428650, 0.430721, 0.432801,
    0.434890, 0.436988, 0.439096, 0.441212, 0.443337, 0.445472, 0.447616, 0.449769,
    0.451931, 0.454103, 0.456284, 0.458475, 0.460675, 0.462885, 0.465105, 0.467334,
    0.469573, 0.471822, 0.474081, 0.476349, 0.478628, 0.480917, 0.483215, 0.485524,
    0.487843, 0.490172, 0.492512, 0.494862, 0.497222, 0.499593, 0.501974, 0.504366,
    0.506769, 0.509182, 0.511607, 0.514042, 0.516488, 0.518945, 0.521413, 0.523892,
    0.526382, 0.528883, 0.531396, 0.533920, 0.536456, 0.539003, 0.541561, 0.544132,
    0.546713, 0.549307, 0.551913, 0.554530, 0.557159, 0.559801, 0.562455, 0.565120,
    0.567798, 0.570489, 0.573192, 0.575907, 0.578635, 0.581375, 0.584129, 0.586895,
    0.589674, 0.592466, 0.595271, 0.598089, 0.600920, 0.603764, 0.606622, 0.609494,
    0.612378, 0.615277, 0.618189, 0.621115, 0.624055, 0.627009, 0.629977, 0.632959,
    0.635955, 0.638965, 0.641990, 0.645030, 0.648083, 0.651152, 0.654236, 0.657334,
    0.660447, 0.663576, 0.666719, 0.669878, 0.673052, 0.676242, 0.679447, 0.682668,
    0.685905, 0.689157, 0.692426, 0.695711, 0.699012, 0.702329, 0.705662, 0.709012,
    0.712379, 0.715763, 0.719163, 0.722580, 0.726014, 0.729466, 0.732935, 0.736421,
    0.739925, 0.743447 };

    int row, col;
    double factor1;
    float radiance_band_31, BT_band_31;
    float radiance_band_32, BT_band_32;
    float surface_temperature;

    for (row = 0; row < nrows; row++)
     for (col = 0; col < ncols; col++)
     {
  // Compute radiance and Brightness Temperature for MODIS band 31
      radiance_band_31 = inputImage.get_data(col,row,band_31);
      if (radiance_band_31 <= MAX_USABLE_L1B)
      {
       radiance_band_31 = (radiance_band_31 - offset[band_31]);
       radiance_band_31 = radiance_band_31 * scale[band_31];
/*
       factor1 = c1/((cw31_5*1e6)*((double)radiance_band_31));
       factor2 = c2/cw31;
       factor1 = log(factor1 + 1.0);
       BT_band_31 = factor2/factor1;
*/
       factor1 = cw31*log((c1/((cw31_5*1e6)*((double)radiance_band_31))) + 1.0);
       BT_band_31 = c2/factor1;
      }
      else
      {
       BT_band_31 = -1.0;		/* Invalid value */
      }

  // Compute radiance and Brightness Temperature for MODIS band 32
      radiance_band_32 = inputImage.get_data(col,row,band_32);
      if (radiance_band_32 <= MAX_USABLE_L1B)
      {
       radiance_band_32 = (radiance_band_32 - offset[band_32]);
       radiance_band_32 = radiance_band_32 * scale[band_32];
/*
       factor1 = c1/(cw32_5*radiance_band_32*1e6);
       factor2 = c2/cw32;
       factor1 = log(factor1 + 1.0);
       BT_band_32 = factor2/factor1;
*/
       factor1 = cw32*log((c1/((cw32_5*1e6)*((double)radiance_band_32))) + 1.0);
       BT_band_32 = c2/factor1;
      }
      else
      {
       BT_band_32 = -1.0;		/* Invalid value */
      }

  // Estimate Surface Temperature
      if (BT_band_31 < 240)
       surface_temperature = aa[0] + bb[0]*BT_band_31 + cc[0]*(BT_band_31-BT_band_32) +
	                     dd[0]*((BT_band_31-BT_band_32)*secantThetaMinusOne[col]);
      else if (BT_band_31 <= 260 )
       surface_temperature = aa[1] + bb[1]*BT_band_31 + cc[1]*(BT_band_31-BT_band_32) +
	                     dd[1]*((BT_band_31-BT_band_32)*secantThetaMinusOne[col]);
      else
       surface_temperature = aa[2] + bb[2]*BT_band_31 + cc[2]*(BT_band_31-BT_band_32) +
	                     dd[2]*((BT_band_31-BT_band_32)*secantThetaMinusOne[col]);

      put_data(surface_temperature,col,row,0);
    }
    no_data_value[0] = -1.0;
    no_data_value[0] = true;
    flush_data();
   
    return;
  }

 // Thin cirrus cloud mask set to 196 if thin cirrus cloud detected by Solar or IR test, and cloud not detected by another cloud test.
 // Thin cirrus cloud mask set to 128 if thin cirrus cloud detected by Solar or IR test, and high cloud detected by another cloud test.
 // Thin cirrus cloud mask set to 96 if thin cirrus cloud not detected by Solar or IR test, and high cloud detected by another cloud test.
 // Thin cirrus cloud mask set to 64 if thin cirrus cloud detected by Solar or IR test, and non-high cloud detected by another cloud test.
 // Thin cirrus cloud mask set ot 32 if thin cirrus cloud not detected, but non-high cloud detected by another cloud test.
 // Set to 0 otherwise, unless the byte 0, bit 0 cloud mask flag is set. In that case it is set to the no_data_value (255).
  void Image::compute_thin_cirrus_cloud_mask(Image& cloudMaskImage, Image& cloudMaskQAImage, Image& snowCoverImage)
  {
    bool thin_cirrus_solar_detect, thin_cirrus_IR_detect, high_cloud_detect, other_cloud_detect, cloud_flag;
    int snow_cover_value, cloud_mask_value, cloud_mask_qa_value, thin_cirrus_flag;
    int row, col, hkm_row, hkm_col;
    
    no_data_value.resize(nbands); // nbands = 1
    no_data_value[0] = 255;

    for (row = 0; row < nrows; row ++)
      for (col = 0; col < ncols; col ++)
      {
        thin_cirrus_solar_detect = thin_cirrus_IR_detect = high_cloud_detect = other_cloud_detect = false;
        cloud_mask_value = (int) cloudMaskImage.get_data(col,row,0);
        if ((2*(cloud_mask_value/2)) == cloud_mask_value)
          thin_cirrus_flag = (int) no_data_value[0];
        else
        {
         cloud_flag = false;
         for (hkm_row = 2*row; hkm_row < 2*(row+1); hkm_row++)
           for (hkm_col = 2*col; hkm_col < 2*(col+1); hkm_col++)
           { 
             snow_cover_value = (int) snowCoverImage.get_data(hkm_col,hkm_row,0);
             if (snow_cover_value == 50)
               cloud_flag = true;
           }
         if (cloud_flag)
         {
          cloud_mask_value = (int) cloudMaskImage.get_data(col,row,1);
          cloud_mask_qa_value = (int) cloudMaskQAImage.get_data(col,row,1);
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 1
            thin_cirrus_solar_detect = true;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 3
            thin_cirrus_IR_detect = true;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 5
            other_cloud_detect = true;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 6
            high_cloud_detect = true;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 7
            high_cloud_detect = true;
          cloud_mask_value = (int) cloudMaskImage.get_data(col,row,2);
          cloud_mask_qa_value = (int) cloudMaskQAImage.get_data(col,row,2);
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 0
            high_cloud_detect = true;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 1
            high_cloud_detect = true;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 2
            other_cloud_detect = true;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 3
            other_cloud_detect = true;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 4
            other_cloud_detect = true;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 5
            other_cloud_detect = true;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 6
            other_cloud_detect = true;
          cloud_mask_value /= 2;
          cloud_mask_qa_value /= 2;
          if (((2*(cloud_mask_value/2)) == cloud_mask_value) && ((2*(cloud_mask_qa_value/2)) != cloud_mask_qa_value)) // bit 7
            other_cloud_detect = true;
          thin_cirrus_flag = 0;
          if (other_cloud_detect)
          {
            thin_cirrus_flag = 32;
            if ((thin_cirrus_IR_detect) || (thin_cirrus_solar_detect))
              thin_cirrus_flag = 64;
          }
          else if (high_cloud_detect)
          {
            thin_cirrus_flag = 96;
            if ((thin_cirrus_IR_detect) || (thin_cirrus_solar_detect))
              thin_cirrus_flag = 128;
          }
          else if ((thin_cirrus_IR_detect) || (thin_cirrus_solar_detect))
            thin_cirrus_flag = 196;
         }
         else
          thin_cirrus_flag = 0;
        } // if (cloud_mask_value != 0)
        put_data(thin_cirrus_flag,col,row,0);
      }
    flush_data();

    return;
  }
#endif

  void Image::compute_ndsi(Image& inputImage, const int& green_band, const int& MIR_band)
  {
    Image maskImage;
    compute_ndsi(inputImage,green_band,MIR_band,maskImage);
    return;
  }

  void Image::compute_ndsi(Image& inputImage, const int& green_band, const int& MIR_band, Image& maskImage)
  {
    bool mask_flag;
    int col, row, mask_col, mask_row;
    double UTM_X, UTM_Y, mask_X, mask_Y;
    double green_value, MIR_value, ndsi_value;

    no_data_value[0] = -2.0;
    no_data_value_flag[0] = true;
    min_value[0] = 1.0;
    min_value_flag[0] = true;
    max_value[0] = -1.0;
    max_value_flag[0] = true;

    for (row = 0; row < nrows; row ++)
      for (col = 0; col < ncols; col ++)
      {
        ndsi_value = green_value = MIR_value = no_data_value[0];
        if (maskImage.data_valid())
        {
          if (geotransform_flag && maskImage.geotransform_valid())
          {
            UTM_X = X_offset + col*X_gsd;
            UTM_Y = Y_offset + row*Y_gsd;
            mask_X = (UTM_X - maskImage.X_offset)/maskImage.X_gsd;
            mask_Y = (UTM_Y - maskImage.Y_offset)/maskImage.Y_gsd;
            mask_col = (int) offset_for_rounding(mask_X);
            mask_row = (int) offset_for_rounding(mask_Y);
          }
          else
          {
            mask_col = col;
            mask_row = row;
          }
          mask_flag = (maskImage.data_valid(mask_col,mask_row,0));
        }
        else
          mask_flag = true;
        if (mask_flag && (inputImage.data_valid(col,row,green_band))&& (inputImage.data_valid(col,row,MIR_band)))
        {
          green_value = inputImage.get_data(col,row,green_band);
          MIR_value = inputImage.get_data(col,row,MIR_band);
          if ((green_value+MIR_value) != 0.0)
          {
            ndsi_value = (green_value - MIR_value) / (green_value + MIR_value);
            if (ndsi_value < min_value[0])
              min_value[0] = ndsi_value;
            if (ndsi_value > max_value[0])
              max_value[0] = ndsi_value;
          }
        }
        put_data(ndsi_value,col,row,0);
      }
    flush_data();

    return;
  }

  void Image::compute_ndvi(Image& inputImage, const int& NIR_band, const int& red_band)
  {
    Image maskImage;
    compute_ndvi(inputImage,NIR_band,red_band,maskImage);
    return;
  }

  void Image::compute_ndvi(Image& inputImage, const int& NIR_band, const int& red_band, Image& maskImage)
  {
    bool mask_flag;
    int col, row, mask_col, mask_row;
    double UTM_X, UTM_Y, mask_X, mask_Y;
    double NIR_value, red_value, ndvi_value;

    no_data_value[0] = -2;
    no_data_value_flag[0] = true;
    min_value[0] = 1.0;
    min_value_flag[0] = true;
    max_value[0] = -1.0;
    max_value_flag[0] = true;

    for (row = 0; row < nrows; row ++)
      for (col = 0; col < ncols; col ++)
      {
        ndvi_value = NIR_value = red_value = no_data_value[0];
        if (maskImage.data_valid())
        {
          if (geotransform_flag && maskImage.geotransform_valid())
          {
            UTM_X = X_offset + col*X_gsd;
            UTM_Y = Y_offset + row*Y_gsd;
            mask_X = (UTM_X - maskImage.X_offset)/maskImage.X_gsd;
            mask_Y = (UTM_Y - maskImage.Y_offset)/maskImage.Y_gsd;
            mask_col = (int) offset_for_rounding(mask_X);
            mask_row = (int) offset_for_rounding(mask_Y);
          }
          else
          {
            mask_col = col;
            mask_row = row;
          }
          mask_flag = (maskImage.data_valid(mask_col,mask_row,0));
        }
        else
          mask_flag = true;
        if (mask_flag && (inputImage.data_valid(col,row,NIR_band))&& (inputImage.data_valid(col,row,red_band)))
        {
          NIR_value = inputImage.get_data(col,row,NIR_band);
          red_value = inputImage.get_data(col,row,red_band);
          if ((NIR_value+red_value) != 0.0)
            ndvi_value = (NIR_value - red_value) / (NIR_value + red_value);
          else
            ndvi_value = 0.0;
          if (ndvi_value < min_value[0])
            min_value[0] = ndvi_value;
          if (ndvi_value > max_value[0])
            max_value[0] = ndvi_value;
        }
        put_data(ndvi_value,col,row,0);
      }
    flush_data();

    return;
  }

  void Image::compute_ndvi(Image& NIRImage, Image& redImage)
  {
    Image maskImage;
    compute_ndvi(NIRImage,redImage,maskImage);
    return;
  }

  void Image::compute_ndvi(Image& NIRImage, Image& redImage, Image& maskImage)
  {
    bool mask_flag;
    int col, row, mask_col, mask_row;
    double UTM_X, UTM_Y, mask_X, mask_Y;
    double NIR_value, red_value, ndvi_value;

    no_data_value[0] = -2;
    no_data_value_flag[0] = true;
    min_value[0] = 1.0;
    min_value_flag[0] = true;
    max_value[0] = -1.0;
    max_value_flag[0] = true;

    for (row = 0; row < nrows; row ++)
      for (col = 0; col < ncols; col ++)
      {
        ndvi_value = NIR_value = red_value = no_data_value[0];
        if (maskImage.data_valid())
        {
          if (geotransform_flag && maskImage.geotransform_valid())
          {
            UTM_X = X_offset + col*X_gsd;
            UTM_Y = Y_offset + row*Y_gsd;
            mask_X = (UTM_X - maskImage.X_offset)/maskImage.X_gsd;
            mask_Y = (UTM_Y - maskImage.Y_offset)/maskImage.Y_gsd;
            mask_col = (int) offset_for_rounding(mask_X);
            mask_row = (int) offset_for_rounding(mask_Y);
          }
          else
          {
            mask_col = col;
            mask_row = row;
          }
          mask_flag = (maskImage.data_valid(mask_col,mask_row,0));
        }
        else
          mask_flag = true;
        if (mask_flag && (NIRImage.data_valid(col,row,0))&& (redImage.data_valid(col,row,0)))
        {
          NIR_value = NIRImage.get_data(col,row,0);
          red_value = redImage.get_data(col,row,0);
          if ((NIR_value+red_value) != 0.0)
            ndvi_value = (NIR_value - red_value) / (NIR_value + red_value);
          else
            ndvi_value = 0.0;
          if (ndvi_value < min_value[0])
            min_value[0] = ndvi_value;
          if (ndvi_value > max_value[0])
            max_value[0] = ndvi_value;
        }
        put_data(ndvi_value,col,row,0);
      }
    flush_data();

    return;
  }

  void Image::compute_difference(Image& firstImage, Image& secondImage)
  {
    int col, row;
    double diff_value;

    for (row = 0; row < nrows; row ++)
      for (col = 0; col < ncols; col ++)
      {
        diff_value = 0.0;
        if ((firstImage.data_valid(col,row,0)) && (secondImage.data_valid(col,row,0)))
          diff_value = firstImage.get_data(col,row,0) - secondImage.get_data(col,row,0);
        put_data(diff_value,col,row,0);
      }
    flush_data();

    return;
  }

  void Image::compare(Image& firstImage, Image& secondImage)
  {
    compare(firstImage, secondImage, 0.0, 0.0);
    return;
  }

  void Image::compare(Image& firstImage, Image& secondImage, const double& UTM_X_shift, const double& UTM_Y_shift)
  {
    int band, col, row;
    double UTM_X, UTM_Y;

    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        UTM_X = firstImage.get_UTM_X(col);
        UTM_Y = firstImage.get_UTM_Y(row);
        UTM_X += UTM_X_shift;
        UTM_Y += UTM_Y_shift;
        for (band = 0; band < nbands; band++)
        {
          if (firstImage.data_valid(col,row,band) && secondImage.data_valid(UTM_X,UTM_Y,band))
          {
            if (firstImage.get_data(col,row,band) == secondImage.get_data(UTM_X,UTM_Y,band))
              put_data(0,col,row,0);
            else
              put_data(255,col,row,0);
          }
          else if (no_data_value_valid(band))
            put_data(get_no_data_value(band),col,row,band);
          else
            put_data(0,col,row,band);
        }
      }
    flush_data();

    return;
  }

  void Image::compare(Image& compareImage, const string& compare_type, const double& compare_value)
  {
    int band, col, row, compare_nbands, value;
    double UTM_X, UTM_Y;

    if ((compare_type != "eq") && (compare_type != "ne"))
    {
      cout << "ERROR: In Image::compare, unknown compare_type = " << compare_type << endl;
      return;
    }

    compare_nbands = compareImage.get_nbands();
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        UTM_X = get_UTM_X(col);
        UTM_Y = get_UTM_Y(row);
        value = 0;
        if (compare_type == "eq")
          value = 255;
        for (band = 0; band < compare_nbands; band++)
        {
          if (compareImage.data_valid(UTM_X,UTM_Y,band))
          {
            if (compare_type == "eq")
            {
              if (compareImage.get_data(UTM_X,UTM_Y,band) != compare_value)
                value = 0;
            }
            else if (compare_type == "ne")
            {
              if (compareImage.get_data(UTM_X,UTM_Y,band) != compare_value)
                value = 255;
            }
          }
        }
        put_data(value,col,row,0);
      }
    flush_data();

    return;
  }

  bool Image::edge(Image& inputImage, Image& maskImage, const int& bias_value, const EdgeOperation& edge_operation, const double& threshold, 
                   const int& output_type, Image& outputMaskImage)
  {
    short int option = 1;
    return edge(inputImage, maskImage, bias_value, edge_operation, threshold, output_type, outputMaskImage, option);
  }

  bool Image::edge(Image& inputImage, Image& maskImage, const int& bias_value, const EdgeOperation& edge_operation, const double& threshold, 
                   const int& output_type, Image& outputMaskImage, const short int& option)
  {
    bool data_valid_flag;
    int col, row, band, input_nbands;
    double window[3][3];
    double edge_value, sum_edge_value, max_edge_value, min_edge_value;

    if (output_type != 1)
    {
      if ((ncols != inputImage.get_ncols()) || (nrows != inputImage.get_nrows()))
        return false;
    }
    else
    {
      if ((ncols != inputImage.get_ncols()) || (nrows != inputImage.get_nrows()) || (nbands != inputImage.get_nbands()))
        return false;
    }
    if (maskImage.data_valid())
      if ((ncols != maskImage.get_ncols()) || (nrows != maskImage.get_nrows()))
        return false;
    if (outputMaskImage.data_valid())
      if ((ncols != outputMaskImage.get_ncols()) || (nrows != outputMaskImage.get_nrows()))
        return false;

    input_nbands = inputImage.get_nbands();

    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        sum_edge_value = 0.0;
        min_edge_value = FLT_MAX;
        max_edge_value = -FLT_MAX;
        data_valid_flag = true;
        if (maskImage.data_valid())
        {
          data_valid_flag = data_valid_flag && 
                            maskImage.data_valid(col-1,row-1,0) &&
                            maskImage.data_valid(col,row-1,0) &&
                            maskImage.data_valid(col+1,row-1,0) &&
                            maskImage.data_valid(col-1,row,0) &&
                            maskImage.data_valid(col,row,0) &&
                            maskImage.data_valid(col+1,row,0) &&
                            maskImage.data_valid(col-1,row+1,0) &&
                            maskImage.data_valid(col,row+1,0) &&
                            maskImage.data_valid(col+1,row+1,0);
        }
        if (data_valid_flag)
        {
         for (band = 0; band < input_nbands; band++)
         {
          if (!maskImage.data_valid())
          {
            data_valid_flag = data_valid_flag && 
                              inputImage.data_valid(col-1,row-1,band) &&
                              inputImage.data_valid(col,row-1,band) &&
                              inputImage.data_valid(col+1,row-1,band) &&
                              inputImage.data_valid(col-1,row,band) &&
                              inputImage.data_valid(col,row,band) &&
                              inputImage.data_valid(col+1,row,band) &&
                              inputImage.data_valid(col-1,row+1,band) &&
                              inputImage.data_valid(col,row+1,band) &&
                              inputImage.data_valid(col+1,row+1,band);
          }

          if (data_valid_flag)
          {
            window[0][0] = inputImage.get_data(col-1,row-1,band) + bias_value;
            window[1][0] = inputImage.get_data(col,row-1,band) + bias_value;
            window[2][0] = inputImage.get_data(col+1,row-1,band) + bias_value;
            window[0][1] = inputImage.get_data(col-1,row,band) + bias_value;
            window[1][1] = inputImage.get_data(col,row,band) + bias_value;
            window[2][1] = inputImage.get_data(col+1,row,band) + bias_value;
            window[0][2] = inputImage.get_data(col-1,row+1,band) + bias_value;
            window[1][2] = inputImage.get_data(col,row+1,band) + bias_value;
            window[2][2] = inputImage.get_data(col+1,row+1,band) + bias_value;

            switch (edge_operation)
            {
              case Prewitt:   edge_value = prewitt(window);
                              break;
              case Sobel:     edge_value = sobel(window);
                              break;
              case Scharr:    edge_value = scharr(window);
                              break;
              case Frei_Chen: edge_value = frei_chen(window,option);
                              break;
              case Std_Dev:   edge_value = std_dev(window);
                              break;
              default:        edge_value = 0.0;
                              break;
            }

            if (output_type == 1)
            {
              if (edge_value < threshold)
                edge_value = 0.0;
              put_data(edge_value, col, row, band);
            }
            else
            {
              sum_edge_value += edge_value;
              if (edge_value < min_edge_value)
                min_edge_value = edge_value;
              if (edge_value > max_edge_value)
                max_edge_value = edge_value;
            }
          } // if (data_valid_flag)
         } // for (band = 0; band < nbands; band++)
        } // if (data_valid_flag)
        if (data_valid_flag)
        {
          switch (output_type)
          {
            case 2:  edge_value /= input_nbands;
                     break;
            case 3:  edge_value = max_edge_value;
                     break;
            case 4:  edge_value = min_edge_value;
                     break;
            default: edge_value = 0.0;
                     break;
          }
          if (output_type != 1)
          {
            if (edge_value < threshold)
              edge_value = 0.0;
            put_data(edge_value, col, row, 0);
          }
          outputMaskImage.put_data(1.0, col, row, 0);
        } // if (data_valid_flag)
        else
          outputMaskImage.put_data(0.0, col, row, 0);
      } // for (col = 1; col < (ncols-1); col++)
    } // for (row = 0; row < nrows; row++)
    flush_data();
    outputMaskImage.flush_data();
    outputMaskImage.set_no_data_value(0.0);

    return true;
  }

  double Image::prewitt(const double window[3][3])
  {
    double gx, gy;
    double edge_value;

    gx = window[2][0] - window[0][0];
    gx += window[2][1] - window[0][1];
    gx += window[2][2] - window[0][2];
    gy = window[0][0] + window[1][0] + window[2][0];
    gy -= window[0][2] + window[1][2] + window[2][2];

    edge_value = sqrt(gx*gx + gy*gy);

    return edge_value;
  }

  double Image::sobel(const double window[3][3])
  {
    double gx, gy;
    double edge_value;

    gx = window[2][0] - window[0][0];
    gx += 2*(window[2][1] - window[0][1]);
    gx += window[2][2] - window[0][2];
    gy = window[0][0] + 2*window[1][0] + window[2][0];
    gy -= window[0][2] + 2*window[1][2] + window[2][2];

    edge_value = sqrt(gx*gx + gy*gy);

    return edge_value;
  }

  double Image::scharr(const double window[3][3])
  {
    double gx, gy;
    double edge_value;

    gx = 3*(window[2][0] - window[0][0]);
    gx += 10*(window[2][1] - window[0][1]);
    gx += 3*(window[2][2] - window[0][2]);
    gy = 3*window[0][2] + 10*window[1][2] + 3*window[2][2];
    gy -= 3*window[0][0] + 10*window[1][0] + 3*window[2][0];

    edge_value = sqrt(gx*gx + gy*gy);

    return edge_value;
  }

  double Image::frei_chen(const double window[3][3], const short int& option)
  {
    double g1, g2, g3, g4, g5, g6, g7, g8, g9, M, S, edge_value;
    double sqrt2 = sqrt(2.0);

    g1 = window[0][0] + sqrt2*window[1][0] + window[2][0];
    g1 -= window[0][2] + sqrt2*window[1][2] + window[2][2];
    g1 /= 2*sqrt2;
    g2 = window[0][0] - window[2][0];
    g2 += sqrt2*(window[0][1] - window[2][1]);
    g2 += window[0][2] - window[2][2];
    g2 /= 2*sqrt2;
    g3 = sqrt2*window[2][0] - window[1][0];
    g3 += window[0][1] - window[2][1];
    g3 += window[1][2] - sqrt2*window[0][2];
    g3 /= 2*sqrt2;
    g4 = sqrt2*window[0][0] - window[1][0];
    g4 += window[2][1] - window[0][1];
    g4 += window[1][2] - sqrt2*window[2][2];
    g4 /= 2*sqrt2;
    g5 = window[1][0];
    g5 -= window[0][1] + window[2][1];
    g5 += window[1][2];
    g5 /= 2;
    g6 = window[2][0] - window[0][0];
    g6 += window[0][2] - window[2][2];
    g6 /= 2;
    g7 = window[0][0] - 2*window[1][0] + window[2][0];
    g7 -= 2*window[0][1] - 4*window[1][1] + 2*window[2][1];
    g7 += window[0][2] - 2*window[1][2] + window[2][2];
    g7 /= 6;
    g8 = window[0][1] + 4*window[1][1] + window[2][1];
    g8 -= 2*window[0][0] - window[1][0] + 2*window[2][0];
    g8 -= 2*window[0][2] - window[1][2] + 2*window[2][2];
    g8 /= 6;
    g9 = window[0][0] + window[1][0] + window[2][0];
    g9 += window[0][1] + window[1][1] + window[2][1];
    g9 += window[0][2] + window[1][2] + window[2][2];
    g9 /= 3;

  // edge option - default
    M = g1*g1 + g2*g2 + g3*g3 + g4*g4;
    S = M + g5*g5 + g6*g6 + g7*g7 + g8*g8 + g9*g9;

    if (option == 2)
    {
     // line-edge option
      M = g1*g1 + g2*g2 + g3*g3 + g4*g4 + g5*g5 + g6*g6 + g7*g7 + g8*g8;
      S = M + g9*g9;
    }

    edge_value = sqrt(M/S);

    return edge_value;
  }

  double Image::std_dev(const double window[3][3])
  {
    int col, row;
    double dnpix, dsum, dsumsq, std_dev_value;

    dnpix = 9.0;
    dsum = 0.0;
    dsumsq = 0.0;
    for (row = 0; row < 3; row++)
    {
      for (col = 0; col < 3; col++)
      {
        dsum += window[col][row];
        dsumsq += window[col][row]*window[col][row];
      }
    }

    std_dev_value = (dsumsq - ((dsum*dsum)/dnpix))/(dnpix-1.0);
               
    if (std_dev_value > 0.0)
      std_dev_value = sqrt(std_dev_value);
    else
      std_dev_value = 0.0;

    return std_dev_value;
  }

#ifdef THREEDIM
  bool Image::data_valid(const int& col, const int& row, const int& slice, const int& band)
  {
    double value;

    if (data_flag)
    {
      if ((col < 0) || (col >= ncols) || (row < 0) || (row >= nrows) || 
          (slice < 0) || (slice >= nslices) || (band < 0) || (band >= nbands))
        return false;

      value = get_data(col,row,slice,band);

      if (no_data_value_flag[band])
        return (value != no_data_value[band]);
      else if (value == value) // Checking for "nan" value
        return true;
      else
        return false;
    }
    else
      return false;
  }
#endif

  bool Image::data_valid(const int& col, const int& row, const int& band)
  {
    double value;

    if (data_flag)
    {
      if ((col < 0) || (col >= ncols) || (row < 0) || (row >= nrows) || (band < 0) || (band >= nbands))
        return false;

      value = get_data(col,row,band);

      if (no_data_value_flag[band])
        return (value != no_data_value[band]);
      else if (value == value) // Checking for "nan" value
        return true;
      else
        return false;
    }
    else
      return false;
  }

  bool Image::data_valid(const double& UTM_X, const double& UTM_Y, const int& band)
  {
    double X,Y;
    int col, row;

    X = (UTM_X - X_offset)/X_gsd;
    Y = (UTM_Y - Y_offset)/Y_gsd;
    col = (int) offset_for_rounding(X);
    row = (int) offset_for_rounding(Y);

    return data_valid(col,row,band);
  }

  double Image::offset_for_rounding(const double& value)
  {
     if (value >= -0.5)
       return (value + 0.5);
     else
       return (value - 0.5);
  }

#ifdef THREEDIM
  double Image::get_data(const int& col, const int& row, const int& slice, const int& band)
  {
    double value = 0.0;

    if (no_data_value_flag[band])
      value = no_data_value[band];

    if ((col < 0) || (col >= ncols) || (row < 0) || (row >= nrows) || 
        (slice < 0) || (slice >= nslices) || (band < 0) || (band >= nbands))
      return value;
#ifndef GDAL
    if (write_flag)
    {
       cout << "WARNING(get_data): Tried to get a data value from a WRITE ONLY image" << endl;
       return value;
    }
#endif
    if (io_nrows_offset < 0)
    {
      io_nrows_offset = 0;
      read_data();
    }

   // Obtain requested value from data block currently held in memory.
    int index = col + row*ncols + slice*nrows*ncols + band*nslices*nrows*ncols;
    switch(dtype)
    {
      case UInt8:   value = (double) byte_data[index];
                    break;
      case UInt16:  value = (double) ushort_data[index];
                    break;
      case UInt32:  value = (double) uint_data[index];
                    break;
      case Float32: value = (double) float_data[index];
                    break;
      default:      value = 0.0;
                    break;
    }

    return value;
  }
#endif

  double Image::get_data(const int& col, const int& row, const int& band)
  {
    double value = 0.0;

    if (no_data_value_flag[band])
      value = no_data_value[band];

    if ((col < 0) || (col >= ncols) || (row < 0) || (row >= nrows) || (band < 0) || (band >= nbands))
      return value;

#ifndef GDAL
    if (write_flag)
    {
       cout << "WARNING(get_data): Tried to get a data value from a WRITE ONLY image" << endl;
       return value;
    }
#endif

    if (io_nrows_step != nrows)
    {
    // Need to check whether or not current read position is contained in the data block currently held in memory.
      if ((io_nrows_offset < 0) || (row < io_nrows_offset) || (row >= (io_nrows_offset + io_nrows_step + io_nrows_overlap)))
      {
      // The current read position is not contained in the data block currently in memory. Need to read new block.
        io_nrows_offset = 0;
        while (row >= io_nrows_offset)
          io_nrows_offset += io_nrows_step;
        io_nrows_offset -= io_nrows_step;
        io_nrows = io_nrows_offset + io_nrows_overlap + io_nrows_step;
        if (io_nrows > nrows)
          io_nrows = nrows - io_nrows_offset;
        else
          io_nrows = io_nrows_step + io_nrows_overlap;
        read_data(); 
      }
    }
    else if (io_nrows_offset < 0)
    {
      io_nrows_offset = 0;
      io_nrows_overlap = 0;
      io_nrows = io_nrows_step;
      read_data();
    }

   // Obtain requested value from data block currently held in memory.
    int index = col + (row - io_nrows_offset)*ncols + band*io_nrows*ncols;
#ifdef GDAL
    if (gdal_flag)
    {
      switch(data_type)
      {
        case GDT_Byte:    value = (double) byte_data[index];
                          break;
        case GDT_UInt16:  value = (double) ushort_data[index];
                          break;
        case GDT_Int16:   value = (double) short_data[index];
                          break;
        case GDT_UInt32:  value = (double) uint_data[index];
                          break;
        case GDT_Int32:   value = (double) int_data[index];
                          break;
        case GDT_Float32: value = (double) float_data[index];
                          break;
        case GDT_Float64: value = double_data[index];
                          break;
        default:          value = 0.0;
                          break;
      }
    }
    else
    {
#endif
      switch(dtype)
      {
        case UInt8:   value = (double) byte_data[index];
                      break;
        case UInt16:  value = (double) ushort_data[index];
                      break;
        case UInt32:  value = (double) uint_data[index];
                      break;
        case Float32: value = (double) float_data[index];
                      break;
        default:      value = 0.0;
                      break;
      }
#ifdef GDAL
    }
#endif

    return value;
  }

  double Image::get_data(const double& UTM_X, const double& UTM_Y, const int& band)
  {
    double X,Y;
    int col, row;

    X = (UTM_X - X_offset)/X_gsd;
    Y = (UTM_Y - Y_offset)/Y_gsd;
    col = (int) offset_for_rounding(X);
    row = (int) offset_for_rounding(Y);

    return get_data(col,row,band);
  }

  int Image::get_col(double& UTM_X)
  { 
    double col;

    col = (UTM_X - X_offset)/X_gsd;
 
    return (int) (col + 0.5);
  }
   
  int Image::get_row(double& UTM_Y)
  { 
    double row;

    row = (UTM_Y - Y_offset)/Y_gsd;
 
    return (int) (row + 0.5);
  }

#ifdef THREEDIM
  void Image::put_data(const double& value, const int& col, const int& row, const int& slice, const int& band)
  {
    if ((col < 0) || (col >= ncols) || (row < 0) || (row >= nrows) || 
        (slice < 0) || (slice >= nslices) || (band < 0) || (band >= nbands))
      return;

    if (read_flag)
    {
       cout << "WARNING(put_data): Tried to put a data value to a READ ONLY image" << endl;
       return;
    }

    io_nrows_offset = 0;

  // Copy the provided data value to the proper location in the current data block
    int index = col + row*ncols + slice*nrows*ncols + band*nslices*nrows*ncols;
    switch(dtype)
    {
        case UInt8:   byte_data[index] = (unsigned char) offset_for_rounding(value);
                      break;
        case UInt16:  ushort_data[index] = (unsigned short int) offset_for_rounding(value);
                      break;
        case UInt32:  uint_data[index] = (unsigned int) offset_for_rounding(value);
                      break;
        case Float32: float_data[index] = (float) value;
                      break;
        default:      break;
    }

    return;
  }
#endif // THREEDIM

  void Image::put_data(const double& value, const int& col, const int& row, const int& band)
  {
    if ((col < 0) || (col >= ncols) || (row < 0) || (row >= nrows) || (band < 0) || (band >= nbands))
    {
//      cout << "WARNING(put_data): Tried to write to location outside of image: col = " << col << " and row = " << row << endl;
      return;
    }

    if (read_flag)
    {
       cout << "WARNING(put_data): Tried to put a data value to a READ ONLY image" << endl;
       return;
    }

    if (io_nrows_step != nrows)
    {
    // Need to check whether or not current read position is contained in the data block currently held in memory.
      if ((io_nrows_offset < 0) || (row < io_nrows_offset) || (row >= (io_nrows_offset + io_nrows_step)))
      {
      // The current write position is not contained in the data block currently in memory.
#ifdef GDAL
      // Need to write currently held block of data before reading data from new block
        write_data(); // NOTE: write_data checks for io_nrows_offset < 0 condition
#endif
        int new_io_nrows_offset = 0;
        while (row >= new_io_nrows_offset)
          new_io_nrows_offset += io_nrows_step;
        new_io_nrows_offset -= io_nrows_step;
#ifdef GDAL
        io_nrows_offset = new_io_nrows_offset;
        io_nrows = io_nrows_offset + io_nrows_overlap + io_nrows_step;
        if (io_nrows > nrows)
          io_nrows = nrows - io_nrows_offset;
        else
          io_nrows = io_nrows_step + io_nrows_overlap;
        read_data();
#else
     // In this case, implicitly in write_only mode, and must write data in sequential order with no overlapping
        if ((new_io_nrows_offset != io_nrows_offset) || (band != io_band))
        {
          if (((io_nrows_offset < 0) && (new_io_nrows_offset == 0) && (band == 0)) || 
              ((band == io_band) && (new_io_nrows_offset == (io_nrows_offset+io_nrows_step))))
          {
            write_data();
            clear_data();
            io_band = band;
            io_nrows_offset = new_io_nrows_offset;
            io_nrows = io_nrows_offset + io_nrows_step;
            if (io_nrows > nrows)
              io_nrows = nrows - io_nrows_offset;
            else
              io_nrows = io_nrows_step;
          }
          else if ((new_io_nrows_offset == 0) && (band == (io_band+1)))
          {
            write_data();
            clear_data();
            io_nrows_offset = 0;
            io_nrows = io_nrows_offset + io_nrows_step;
            if (io_nrows > nrows)
              io_nrows = nrows - io_nrows_offset;
            else
              io_nrows = io_nrows_step;
            io_band = band;
          }
          else
          {
            cout << "WARNING(put_data): Tried to put a data value out of sequential write order" << endl;
            cout << "io_band = " << io_band << ", io_nrows_offset = " << io_nrows_offset;
            cout << " and new_io_nrows_offset = " << new_io_nrows_offset << endl;
            cout << "image_file = " << image_file << ", value = " << value << ", col = " << col;
            cout << ", row = " << row << ", and band = " << band << endl;
            return;
          }
        }
#endif      
      }
    }
    else if (io_nrows_offset < 0)
    {
      io_nrows_offset = 0;
      io_nrows_overlap = 0;
      io_nrows = io_nrows_step;
#ifdef GDAL
      read_data();
#endif
    }

  // Copy the provided data value to the proper location in the current data block
    int index;
#ifdef GDAL
    if (gdal_flag)
    {
      index = col + (row-io_nrows_offset)*ncols + band*io_nrows*ncols;
      switch(data_type)
      {
        case GDT_Byte:    byte_data[index] = (unsigned char) offset_for_rounding(value);
                          break;
        case GDT_UInt16:  ushort_data[index] = (unsigned short int) offset_for_rounding(value);
                          break;
        case GDT_Int16:   short_data[index] = (short int) offset_for_rounding(value);
                          break;
        case GDT_UInt32:  uint_data[index] = (unsigned int) offset_for_rounding(value);
                          break;
        case GDT_Int32:   int_data[index] = (int) offset_for_rounding(value);
                          break;
        case GDT_Float32: float_data[index] = (float) value;
                          break;
        case GDT_Float64: double_data[index] = value;
                          break;
        default:          break;
      }
    }
    else
    {
#endif
      if (nrows == io_nrows_step)
        index = col + row*ncols + band*nrows*ncols;
      else
        index = col + (row-io_nrows_offset)*ncols;
      switch(dtype)
      {
        case UInt8:   byte_data[index] = (unsigned char) offset_for_rounding(value);
                      break;
        case UInt16:  ushort_data[index] = (unsigned short int) offset_for_rounding(value);
                      break;
        case UInt32:  uint_data[index] = (unsigned int) offset_for_rounding(value);
                      break;
        case Float32: float_data[index] = (float) value;
                      break;
        default:      break;
      }
#ifdef GDAL
    }
#endif

    return;
  }

  void Image::put_data_line(const double& value, const int& col1, const int& col2, 
                            const int& row1, const int& row2, const int& band)
  {
    int col, row;

    if ((col1 == col2) && (row1 == row2))
    {
      put_data(value,col1,row1,band);
      return;
    }

    if (col1 == col2)
    {
      if (row2 > row1)
      {
        for (row = row1; row <= row2; row++)
          put_data(value,col1,row,band);
      }
      else
      {
        for (row = row2; row <= row1; row++)
          put_data(value,col1,row,band);
      }
    }

    if (row1 == row2)
    {
      if (col2 > col1)
      {
        for (col = col1; col <= col2; col++)
          put_data(value,col,row1,band);
      }
      else
      {
        for (col = col2; col <= col1; col++)
          put_data(value,col,row1,band);
      }
    }

    double x, y, x1, x2, y1, y2;
    x1 = col1;
    x2 = col2;
    y1 = row1;
    y2 = row2;
    if (abs(col1-col2) <= abs(row1-row2))
    {
      if (row2 > row1)
      {
        for (row = row1; row <= row2; row++)
        {
          y = row;
          x = x1 + (x2-x1)*(y-y1)/(y2-y1);
          col = (int) offset_for_rounding(x);
          put_data(value,col,row,band);
        }
      }
      else
      {
        for (row = row2; row <= row1; row++)
        {
          y = row;
          x = x1 + (x2-x1)*(y-y1)/(y2-y1);
          col = (int) offset_for_rounding(x);
          put_data(value,col,row,band);
        }
      }
    }
    else
    {
      if (col2 > col1)
      {
        for (col = col1; col <= col2; col++)
        {
          x = col;
          y = y1 + (y2-y1)*(x-x1)/(x2-x1);
          row = (int) offset_for_rounding(y);
          put_data(value,col,row,band);
        }
      }
      else
      {
        for (col = col2; col <= col1; col++)
        {
          x = col;
          y = y1 + (y2-y1)*(x-x1)/(x2-x1);
          row = (int) offset_for_rounding(y);
          put_data(value,col,row,band);
        }
      }
    }

    return;
  }

  void Image::put_data(const double& value, const double& UTM_X, const double& UTM_Y, const int& band)
  {
    double X,Y;
    int col, row;

    X = (UTM_X - X_offset)/X_gsd;
    Y = (UTM_Y - Y_offset)/Y_gsd;
    col = (int) offset_for_rounding(X);
    row = (int) offset_for_rounding(Y);

    put_data(value,col,row,band);
  
    return;
  }

  void Image::put_data_line(const double& value, const double& UTM_X1, const double& UTM_X2, 
                            const double& UTM_Y1, const double& UTM_Y2, const int& band)
  {
    double X1, X2, Y1, Y2;
    int col1, col2, row1, row2;

    X1 = (UTM_X1 - X_offset)/X_gsd;
    Y1 = (UTM_Y1 - Y_offset)/Y_gsd;
    X2 = (UTM_X2 - X_offset)/X_gsd;
    Y2 = (UTM_Y2 - Y_offset)/Y_gsd;
    col1 = (int) offset_for_rounding(X1);
    row1 = (int) offset_for_rounding(Y1);
    col2 = (int) offset_for_rounding(X2);
    row2 = (int) offset_for_rounding(Y2);

    put_data_line(value,col1,col2,row1,row2,band);
  
    return;
  }

  void Image::put_data_values(const double& value)
  {
    int band, row, col;

    for (band = 0; band < nbands; band++)
      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
          put_data(value,col,row,band);
    flush_data();
 
    return;
  }

  void Image::putMinimum(const double& value, const int& band)
  {
    min_value[band] = value;
    min_value_flag[band] = true;

    return;
  }

  void Image::putMaximum(const double& value, const int& band)
  {
    max_value[band] = value;
    max_value_flag[band] = true;

    return;
  }

#ifdef GDAL
  void Image::put_no_data_value()
  {
    int band;

    for (band = 0; band < nbands; band++)
    {
      if (no_data_value_flag[band])
        imageDataset->GetRasterBand(band+1)->SetNoDataValue(no_data_value[band]);
    }
    
    return;
  }

  bool Image::update_mask(Image& sourceImage)
  {
    if (!sourceImage.gdal_valid())
    {
      cout << "ERROR(image): Cannot update mask; invalid GDAL association for source image" << endl;
      return false;
    }
    if (!sourceImage.info_flag)
    {
      cout << "ERROR(image): Cannot update minimum mask; invalid image information for source image" << endl;
      return false;
    }
    if (!sourceImage.no_data_value_valid(0))
    {
      cout << "ERROR(image): Cannot update minimum mask; invalid no data value for source image" << endl;
      return false;
    }
    if (!sourceImage.data_flag)
      sourceImage.read_data();

    int col, row, source_col, source_row;
    double UTM_X, UTM_Y, source_X, source_Y;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        if (geotransform_flag && sourceImage.geotransform_valid())
        {
          UTM_X = X_offset + col*X_gsd;
          UTM_Y = Y_offset + row*Y_gsd;
          source_X = (UTM_X - sourceImage.X_offset)/sourceImage.X_gsd;
          source_Y = (UTM_Y - sourceImage.Y_offset)/sourceImage.Y_gsd;
          source_col = (int) offset_for_rounding(source_X);
          source_row = (int) offset_for_rounding(source_Y);
        }
        else
        {
          source_col = col;
          source_row = row;
        }
        if (!sourceImage.data_valid(source_col,source_row,0))
          put_data(0.0,col,row,0);
      }
    flush_data();

    return true;
  }
#endif

  void Image::increment(const int& col, const int& row, const int& band)
  {
    double value = get_data(col,row,band);
    put_data(++value,col,row,band);

    return;
  }

 // Copy properly registered data
  void Image::registered_data_copy(const int& band, Image& sourceImage, const int& source_band)
  {
    int   process_flag = 1; // direct data registered copy
    Image null_image;

    registered_feature_copy(process_flag, band, sourceImage, source_band, null_image);

    return;
  }

  void Image::registered_data_copy(const int& band, Image& sourceImage, const int& source_band, Image& maskImage)
  {
    int   process_flag = 1; // direct data registered copy

    registered_feature_copy(process_flag, band, sourceImage, source_band, maskImage);

    return;
  }

 // Copy properly registered tasseled cap data
  void Image::registered_tc_copy(const int& band, Image& sourceImage, const int& tc_band, Image& maskImage)
  {
    int   process_flag = 2; // tasseled cap computation and registered copy

    registered_feature_copy(process_flag, band, sourceImage, tc_band, maskImage);

    return;
  }

 // Copy properly registered ndvi feature data
  void Image::registered_ndvi_copy(const int& band, Image& sourceImage, Image& maskImage)
  {
    int   process_flag = 3; // ndvi computation and registered copy
    int   source_band = 0;

    registered_feature_copy(process_flag, band, sourceImage, source_band, maskImage);

    return;
  }

 // Copy properly registered and clipped radar data
  void Image::registered_clipped_radar_copy(const int& band, Image& sourceImage, const int& source_band, Image& maskImage)
  {
    int   process_flag = 4; // radar clipped and 1.2 and registered copy

    registered_feature_copy(process_flag, band, sourceImage, source_band, maskImage);

    return;
  }

  void Image::registered_feature_copy(const int& process_flag, const int& band, 
                                      Image& sourceImage, const int& source_band, Image& maskImage)
  {
    int col, row, source_col, source_row, mask_col, mask_row;
    double UTM_X, UTM_Y, source_X, source_Y, mask_X, mask_Y;
    double blue_value, green_value, red_value, nir_value, mask_value, feature_value;
    
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        if (geotransform_flag && sourceImage.geotransform_flag)
        {
          UTM_X = X_offset + col*X_gsd;
          UTM_Y = Y_offset + row*Y_gsd;
          source_X = (UTM_X - sourceImage.X_offset)/sourceImage.X_gsd;
          source_Y = (UTM_Y - sourceImage.Y_offset)/sourceImage.Y_gsd;
          source_col = (int) offset_for_rounding(source_X);
          source_row = (int) offset_for_rounding(source_Y);
        }
        else
        {
          source_col = col;
          source_row = row;
        }
        if (sourceImage.data_valid(source_col,source_row,source_band))
        {
          if (maskImage.data_valid())
          {
            if (geotransform_flag && maskImage.geotransform_flag)
            {
              UTM_X = X_offset + col*X_gsd;
              UTM_Y = Y_offset + row*Y_gsd;
              mask_X = (UTM_X - maskImage.X_offset)/maskImage.X_gsd;
              mask_Y = (UTM_Y - maskImage.Y_offset)/maskImage.Y_gsd;
              mask_col = (int) offset_for_rounding(mask_X);
              mask_row = (int) offset_for_rounding(mask_Y);
            }
            else
            {
              mask_col = col;
              mask_row = row;
            }
            mask_value = maskImage.get_data(mask_col,mask_row,0);
          }
          else
            mask_value = true;
          if (mask_value)
          {
            switch (process_flag)
            {
              case 1:  feature_value = sourceImage.get_data(source_col,source_row,source_band);
                       break;
              case 2:  blue_value = sourceImage.get_data(source_col,source_row,0);
                       green_value = sourceImage.get_data(source_col,source_row,1);
                       red_value = sourceImage.get_data(source_col,source_row,2);
                       nir_value = sourceImage.get_data(source_col,source_row,3);
                       switch(source_band)
                       {
                         case 0:  feature_value = 0.326*blue_value + 0.509*green_value + 0.560*red_value + 0.567*nir_value;
                                  break;
                         case 1:  feature_value = 0.819*nir_value - 0.311*blue_value - 0.356*green_value - 0.325*red_value;
                                  break;
                         case 2:  feature_value = 0.722*red_value - 0.612*blue_value - 0.312*green_value - 0.081*nir_value;
                                  break;
                         case 3:  feature_value = 0.719*green_value - 0.650*blue_value - 0.243*red_value - 0.031*nir_value;
                                  break;
                         default: feature_value = 0.0;
                                  break;
                       }
                       break;
              case 3:  red_value = sourceImage.get_data(source_col,source_row,2);
                       nir_value = sourceImage.get_data(source_col,source_row,3);
                       feature_value = (nir_value-red_value)/(nir_value+red_value);
                       break;
              case 4:  feature_value = sourceImage.get_data(source_col,source_row,source_band);
                       if (feature_value > 1.2)
                         feature_value = 1.2;
                       break;
              default: feature_value = sourceImage.get_data(source_col,source_row,source_band);
                       break;
            }
          } // if (mask_value)
          else
            feature_value = 0.0;
          put_data(feature_value,col,row,band);
        } // if (sourceImage.data_valid(source_col,source_row,source_band))
      } // for (row = 0; row < nrows; row++)
        //   for (col = 0; col < ncols; col++)
    flush_data();

    return;
  }

  void Image::registered_mask_copy(Image& sourceImage)
  {
    int col, row, source_col, source_row, source_ncols, source_nrows;
    double UTM_X, UTM_Y, source_X_offset, source_Y_offset, source_X_gsd, source_Y_gsd;
    double value;

    source_ncols = sourceImage.get_ncols();
    source_nrows = sourceImage.get_nrows();
    source_X_offset = sourceImage.get_X_offset();
    source_Y_offset = sourceImage.get_Y_offset();
    source_X_gsd = sourceImage.get_X_gsd();
    source_Y_gsd = sourceImage.get_Y_gsd();
    for (source_row = 0; source_row < source_nrows; source_row++)
      for (source_col = 0; source_col < source_ncols; source_col++)
      {
        if (geotransform_flag && sourceImage.geotransform_flag)
        {
          UTM_X = source_X_offset + source_col*source_X_gsd;
          UTM_Y = source_Y_offset + source_row*source_Y_gsd;
          col = (int) offset_for_rounding((UTM_X - X_offset)/X_gsd);
          row = (int) offset_for_rounding((UTM_Y - Y_offset)/Y_gsd);
        }
        else
        {
          col = source_col;
          row = source_row;
        }
        if (sourceImage.data_valid(source_col,source_row,0))
        {
          value = sourceImage.get_data(source_col,source_row,0);
          if (value > 0)
            put_data(value,col,row,0);
        } // if (sourceImage.data_valid(source_col,source_row,0))
      } // for (source_row = 0; source_row < source_nrows; source_row++)
        //   for (source_col = 0; source_col < source_ncols; source_col++)
    flush_data();

    return;
  }
/*
 // Copy properly registered data through and a forward row and column projection image
  void Image::forward_data_copy(const int& band, Image& forwardProjRowColImage, Image& sourceImage, const int& source_band)
  {
    int col, row, proj_col, proj_row, source_col, source_row;
    double UTM_X, UTM_Y, proj_X, proj_Y;
    double feature_value;

    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        UTM_X = X_offset + col*X_gsd;
        UTM_Y = Y_offset + row*Y_gsd;
        proj_X = (UTM_X - forwardProjRowColImage.X_offset)/forwardProjRowColImage.X_gsd;
        proj_Y = (UTM_Y - forwardProjRowColImage.Y_offset)/forwardProjRowColImage.Y_gsd;
        proj_col = (int) offset_for_rounding(proj_X);
        proj_row = (int) offset_for_rounding(proj_Y);
        source_col = (int) offset_for_rounding(forwardProjRowColImage.get_data(proj_col,proj_row,0));
        source_row = (int) offset_for_rounding(forwardProjRowColImage.get_data(proj_col,proj_row,1));
        feature_value = sourceImage.get_data(source_col,source_row,source_band);
        put_data(feature_value,col,row,band);
      }
    flush_data();

    return;
  }

 // Copy properly registered data through and a reverse row and column projection image
  void Image::reverse_data_copy(const int& band, Image& reverseProjRowColImage, Image& forwardProjRowColImage,
                                Image& sourceImage, const int& source_band)
  {
    int col, row, proj_col, proj_row, source_col, source_row;
    double UTM_X, UTM_Y, source_X, source_Y;
    double feature_value;
    
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        proj_col = (int) reverseProjRowColImage.get_data(col,row,0);
        proj_row = (int) reverseProjRowColImage.get_data(col,row,1);
        UTM_X = forwardProjRowColImage.X_offset + proj_col*forwardProjRowColImage.X_gsd;
        UTM_Y = forwardProjRowColImage.Y_offset + proj_row*forwardProjRowColImage.Y_gsd;
        source_X = (UTM_X - sourceImage.X_offset)/sourceImage.X_gsd;
        source_Y = (UTM_Y - sourceImage.Y_offset)/sourceImage.Y_gsd;
        source_col = (int) offset_for_rounding(source_X);
        source_row = (int) offset_for_rounding(source_Y);
        if (sourceImage.data_valid(source_col,source_row,source_band))
        {
          feature_value = sourceImage.get_data(source_col,source_row,source_band);
          put_data(feature_value,col,row,band);
        }
      }
    flush_data();

    return;
  }
*/
#ifdef SHAPEFILE
  void Image::overlay_copy(const string& shape_base_file_name, const int& red, const int& green, const int& blue)
  {
    SHPHandle hSHP;		 // Shape File Handle
    int nb_entities;             // Number of shape entities/structures.
    int shapeType;               // Shapetype
    double minBound[4];          // X, Y, Z and M minimum values
    double maxBound[4];          // X, Y, Z and M maximum values

    if (!gdal_flag)
    {
      X_offset = 0.0;
      Y_offset = 0.0;
    }

    hSHP = SHPOpen(shape_base_file_name.c_str(),"rb");
    SHPGetInfo(hSHP, &nb_entities, &shapeType, &minBound[0], &maxBound[0]);

    int entity, vertex, nb_vertices;
    int X1, X2, Y1, Y2;
    SHPObject *shpObject;

    if (!check_bounds(minBound[0],minBound[1],maxBound[0],maxBound[1]))
    {
      cout << "WARNING: Attempted to overlay shapefile which is completely out of image bounds" << endl;
      cout << "Overlay not performed." << endl;
      return;
    }

    if (shapeType == SHPT_POINT)
    {
      for (entity = 0; entity < nb_entities; entity++)
      {
        shpObject = SHPReadObject(hSHP, entity);
        X1 = (int) (shpObject->padfX[0] + 0.5 - X_offset);
        if (X1 >= ncols)
          X1 = ncols-1;
        Y1 = (int) (shpObject->padfY[0] + 0.5 - Y_offset);
        if (Y1 >= nrows)
          Y1 = nrows-1;
        put_data(red,X1,Y1,0);
        put_data(green,X1,Y1,1);
        put_data(blue,X1,Y1,2);
      }
    }
    else if ((shapeType == SHPT_ARC) || (shapeType == SHPT_POLYGON))
    {
      for (entity = 0; entity < nb_entities; entity++)
      {
        shpObject = SHPReadObject(hSHP, entity);
        nb_vertices = shpObject->nVertices;
        X1 = (int) (shpObject->padfX[0] + 0.5 - X_offset);
        if (X1 >= ncols)
          X1 = ncols-1;
        Y1 = (int) (shpObject->padfY[0] + 0.5 - Y_offset);
        if (Y1 >= nrows)
          Y1 = nrows-1;
        for (vertex = 1; vertex < nb_vertices; vertex++)
        {
          X2 = (int) (shpObject->padfX[vertex] + 0.5 - X_offset);
          if (X2 >= ncols)
            X2 = ncols-1;
          Y2 = (int) (shpObject->padfY[vertex] + 0.5 - Y_offset);
          if (Y2 >= nrows)
            Y2 = nrows-1;
          put_data_line(red,X1,X2,Y1,Y2,0);
          put_data_line(green,X1,X2,Y1,Y2,1);
          put_data_line(blue,X1,X2,Y1,Y2,2);
          X1 = X2;
          Y1 = Y2;
        }
        if (shapeType == SHPT_POLYGON)
        {
          X2 = (int) (shpObject->padfX[0] + 0.5 - X_offset);
          if (X2 >= ncols)
            X2 = ncols-1;
          Y2 = (int) (shpObject->padfY[0] + 0.5 - Y_offset);
          if (Y2 >= nrows)
            Y2 = nrows-1;
          put_data_line(red,X1,X2,Y1,Y2,0);
          put_data_line(green,X1,X2,Y1,Y2,1);
          put_data_line(blue,X1,X2,Y1,Y2,2);
        }
      }
    }

    return;
  }

  void Image::overlay_copy(Shape shapeFile, const int& red, const int& green, const int& blue)
  {
    int entity, nb_entities;
    int vertex, nb_vertices;
    double X1, X2, Y1, Y2;

    if (!check_bounds(shapeFile))
    {
      cout << "WARNING: Attempted to overlay shapefile which is completely out of image bounds" << endl;
      cout << "Overlay not performed." << endl;
      return;
    }

    if (shapeFile.get_shapeType() == SHPT_POINT)
    {
      nb_entities = shapeFile.get_nb_entities();
      for (entity = 0; entity < nb_entities; entity++)
      {
        nb_vertices = shapeFile.get_nb_vertices(entity);
        for (vertex = 0; vertex < nb_vertices; vertex++)
        {
          X1 = shapeFile.get_X(entity,vertex);
          Y1 = shapeFile.get_Y(entity,vertex);
          put_data(red,X1,Y1,0);
          put_data(green,X1,Y1,1);
          put_data(blue,X1,Y1,2);
        }
      }
    }
    else if (shapeFile.get_shapeType() == SHPT_ARC)
    {
      nb_entities = shapeFile.get_nb_entities();
      for (entity = 0; entity < nb_entities; entity++)
      {
        nb_vertices = shapeFile.get_nb_vertices(entity);
        for (vertex = 1; vertex < nb_vertices; vertex++)
        {
          X1 = shapeFile.get_X(entity,(vertex-1));
          Y1 = shapeFile.get_Y(entity,(vertex-1));
          X2 = shapeFile.get_X(entity,vertex);
          Y2 = shapeFile.get_Y(entity,vertex);
          put_data_line(red,X1,X2,Y1,Y2,0);
          put_data_line(green,X1,X2,Y1,Y2,1);
          put_data_line(blue,X1,X2,Y1,Y2,2);
        }
      }
    }

    return;
  }

  void Image::shape_to_image(Shape shapeFile)
  {
    bool plotted_flag;
    int entity, nb_entities, nb_sites;
    int vertex, nb_vertices;
    int col1, col2, row1, row2;
    double UTM_X1, UTM_X2, UTM_Y1, UTM_Y2, X, Y;

    if (!check_bounds2(shapeFile))
    {
      cout << "WARNING: Attempted to plot shapefile boundaries that are completely out of image bounds" << endl;
      cout << "Plotting not performed." << endl;
      return;
    }

    if (shapeFile.get_shapeType() == SHPT_POINT)
    {
      nb_sites = 0;
      nb_entities = shapeFile.get_nb_entities();
      for (entity = 0; entity < nb_entities; entity++)
      {
        plotted_flag = false;
        nb_vertices = shapeFile.get_nb_vertices(entity);
        for (vertex = 0; vertex < nb_vertices; vertex++)
        {
          UTM_X1 = shapeFile.get_X(entity,vertex);
          UTM_Y1 = shapeFile.get_Y(entity,vertex);
          X = (UTM_X1 - X_offset)/X_gsd;
          Y = (UTM_Y1 - Y_offset)/Y_gsd;
          col1 = (int) offset_for_rounding(X);
          row1 = (int) offset_for_rounding(Y);
          if ((col1 < 0) || (col1 >= ncols) || (row1 < 0) || (row1 >= nrows))
            break;
          if (!plotted_flag)
          {
            nb_sites++;
            plotted_flag = true;
          }
          put_data(nb_sites,col1,row1,0);
        }
      }
    }
    else if (shapeFile.get_shapeType() == SHPT_ARC)
    {
      nb_sites = 0;
      nb_entities = shapeFile.get_nb_entities();
      for (entity = 0; entity < nb_entities; entity++)
      {
        plotted_flag = false;
        nb_vertices = shapeFile.get_nb_vertices(entity);
        for (vertex = 1; vertex < nb_vertices; vertex++)
        {
          UTM_X1 = shapeFile.get_X(entity,(vertex-1));
          UTM_Y1 = shapeFile.get_Y(entity,(vertex-1));
          X = (UTM_X1 - X_offset)/X_gsd;
          Y = (UTM_Y1 - Y_offset)/Y_gsd;
          col1 = (int) offset_for_rounding(X);
          row1 = (int) offset_for_rounding(Y);
          UTM_X2 = shapeFile.get_X(entity,vertex);
          UTM_Y2 = shapeFile.get_Y(entity,vertex);
          X = (UTM_X2 - X_offset)/X_gsd;
          Y = (UTM_Y2 - Y_offset)/Y_gsd;
          col2 = (int) offset_for_rounding(X);
          row2 = (int) offset_for_rounding(Y);
          if (((col1 < 0) || (col1 >= ncols) || (row1 < 0) || (row1 >= nrows)) &&
              ((col2 < 0) || (col2 >= ncols) || (row2 < 0) || (row2 >= nrows)))
            break;
          if (!plotted_flag)
          {
            nb_sites++;
            plotted_flag = true;
          }
          put_data_line(entity,col1,col2,row1,row2,0);
        }
      }
    }
    else if ((shapeFile.get_shapeType() == SHPT_POLYGON) || (shapeFile.get_shapeType() == SHPT_POLYGONZ))
    {
bool print_flag;
      bool up_flag, down_flag;
      int part, nb_parts;
      double X_minBound, Y_minBound, X_maxBound, Y_maxBound;
      int col_min, row_min, col_max, row_max, temp_index, temp_ncols, temp_nrows;
      vector<dPoint> dpath;
      vector<Point> path;
      vector<int> col_nodes, minimal_col_nodes, prev_col_nodes;
      vector<bool> temp_polygon, temp_fill;
      int temp_size, col_nodes_size, prev_col_nodes_size, col_index, cusp_index, cusp_diff, sum_diff;
      nb_sites = 0;
      nb_entities = shapeFile.get_nb_entities();
      for (entity = 0; entity < nb_entities; entity++)
      {
        shapeFile.get_Bounds(entity, X_minBound, Y_minBound, X_maxBound, Y_maxBound);
        X = (X_minBound - X_offset)/X_gsd;
        Y = (Y_minBound - Y_offset)/Y_gsd;
        col_min = (int) offset_for_rounding(X);
        row_min = (int) offset_for_rounding(Y);
        X = (X_maxBound - X_offset)/X_gsd;
        Y = (Y_maxBound - Y_offset)/Y_gsd;
        col_max = (int) offset_for_rounding(X);
        row_max = (int) offset_for_rounding(Y);
        if (check_bounds2(col_min, row_min, col_max, row_max))
        {
          nb_sites++;
          cout << "Site " << nb_sites << " is shape file entity " << entity << endl;
cout << "col_min = " << col_min << ", col_max = " << col_max << ", row_min = " << row_min << " and row_max = " << row_max << endl;

          temp_ncols = col_max - col_min + 1;
          temp_nrows = row_max - row_min + 1;
          temp_size = temp_ncols*temp_nrows;
          temp_polygon.clear();
          temp_fill.clear();
          for (temp_index = 0; temp_index < temp_size; temp_index++)
          {
            temp_polygon.push_back(false);
            temp_fill.push_back(false);
          } 

        // Plot polygon boundary
          nb_parts = shapeFile.get_nParts(entity);
          for (part = 0; part < nb_parts; part++)
          {
            shapeFile.get_Vertices(entity, part, dpath);
            convert_dpath_to_path(dpath,path);
            nb_vertices = path.size();
            if (nb_vertices == 1)
            {
              col1 = path[0].get_col() - col_min;
              row1 = path[0].get_row() - row_min;
              temp_index = col1 + row1*temp_ncols;
              temp_polygon[temp_index] = true;
            }
            else
            {
              for (vertex = 1; vertex < nb_vertices; vertex++)
              {
                col1 = path[vertex-1].get_col() - col_min;
                row1 = path[vertex-1].get_row() - row_min;
                col2 = path[vertex].get_col() - col_min;
                row2 = path[vertex].get_row() - row_min;
                plot_bool_line(col1,col2,row1,row2,temp_ncols,temp_polygon);
              }
            }
          }

        // Now fill in the polygon, initially recording fills in temp_fill
          for (row1 = 1; row1 < (temp_nrows-1); row1++)
          {
print_flag = false;
/*
if ((row1 + row_min) > 19125)
  print_flag = true;
if ((row1 + row_min) > 19135)
  print_flag = false;
*/
if (print_flag)
  cout << endl << "Processing row " << (row1 + row_min) << endl;
           // Find columns that are on the polygon boundary
            col_nodes.clear();
            for (col1 = 0; col1 < temp_ncols; col1++)
            {
              temp_index = col1 + row1*temp_ncols;
              if (temp_polygon[temp_index])
                col_nodes.push_back(col1);
            }

           // Minimize col_nodes list (keep col_node furthest to right for adjacent col_nodes)
            minimal_col_nodes.clear();
            col_nodes_size = col_nodes.size();
if ((print_flag)&& (col_nodes_size > 0))
  cout << "Column locations on the polygon boundary are: " << (col_nodes[0] + col_min) << " ";
            for (vertex = 1; vertex < col_nodes_size; vertex++)
            {
              col1 = col_nodes[vertex-1];
              col2 = col_nodes[vertex];
if (print_flag)
  cout << (col2 + col_min) << " ";
              if ((col1+1) != col2)
                minimal_col_nodes.push_back(col1);
            }
if (print_flag)
  cout << endl;
            minimal_col_nodes.push_back(col2);

          // Edit out column nodes where the polygon does not cross the row.
            col_nodes.clear();
            col_nodes_size = minimal_col_nodes.size();
if ((print_flag) && (col_nodes_size > 0))
  cout << "Minimal column locations on the polygon boundary are: ";
            if (col_nodes_size < 2)
            {
              col_nodes_size = 0;
              col_nodes.clear();
if (print_flag)
  cout << "X";
            }
            for (vertex = 0; vertex < col_nodes_size; vertex++)
            {
              up_flag = down_flag = false;
              col1 = minimal_col_nodes[vertex];
if (print_flag)
  cout << (col1 + col_min) << " ";
              for (col2 = col1; col2 >= 0; col2--)
              {
                temp_index = col2 + row1*temp_ncols;
                if (!(temp_polygon[temp_index]))
                  break;
                temp_index = (col2-1) + (row1-1)*temp_ncols;
                if ((temp_polygon[temp_index]) || (temp_polygon[temp_index+1]) || (temp_polygon[temp_index+2]))
                  up_flag = true;
              }
              if (up_flag)
              {
                for (col2 = col1; col2 >= 0; col2--)
                {
                  temp_index = col2 + row1*temp_ncols;
                  if (!(temp_polygon[temp_index]))
                    break;
                  temp_index = (col2-1) + (row1+1)*temp_ncols;
                  if ((temp_polygon[temp_index]) || (temp_polygon[temp_index+1]) || (temp_polygon[temp_index+2]))
                    down_flag = true;
                }
              }
              else
              {
                for (col2 = col1; col2 >= 0; col2--)
                {
                  temp_index = col2 + row1*temp_ncols;
                  if (!(temp_polygon[temp_index]))
                    break;
                  temp_index = (col2-1) + (row1+1)*temp_ncols;
                  if ((temp_polygon[temp_index]) || (temp_polygon[temp_index+1]) || (temp_polygon[temp_index+2]))
                    down_flag = true;
                }
                if (down_flag)
                {
                  for (col2 = col1; col2 >= 0; col2--)
                  {
                    temp_index = col2 + row1*temp_ncols;
                    if (!(temp_polygon[temp_index]))
                      break;
                    temp_index = (col2-1) + (row1-1)*temp_ncols;
                    if ((temp_polygon[temp_index]) || (temp_polygon[temp_index+1]) || (temp_polygon[temp_index+2]))
                      up_flag = true;
                  }
                }
              }
              if ((up_flag) && (down_flag))
                col_nodes.push_back(col1);
else if (print_flag)
  cout << "X";
            } // for (vertex = 0; vertex < col_nodes_size; vertex)
if (print_flag)
  cout << endl;
          // Edit out extra column node when there is an odd number of nodes
            minimal_col_nodes.clear();
            col_nodes_size = col_nodes.size();
            if (((col_nodes_size/2)*2) != (col_nodes_size))
            {
             //  Unusual case - need to consult prev_col_nodes - most likely a boundary cusp
cout << endl << "Detected an unusual case for entity " << entity << " at row " << (row1+row_min) << endl;
cout << "col_nodes: ";
for (vertex = 0; vertex < col_nodes_size; vertex++)
  cout << (col_nodes[vertex]+col_min) << " ";
cout << endl;
              prev_col_nodes_size = prev_col_nodes.size();
cout << "prev_col_nodes: ";
for (vertex = 0; vertex < prev_col_nodes_size; vertex++)
  cout << (prev_col_nodes[vertex]+col_min) << " ";
cout << endl;
            // Find the cusp_index as the index of the node that, when removed,
            // creates the best match between col_nodes and prev_col_nodes
              if (prev_col_nodes_size > col_nodes_size)
              {
                cusp_index = 0;
                cusp_diff = 0;
                for (vertex = 0; vertex < col_nodes_size; vertex++)
                  cusp_diff += abs(col_nodes[vertex] - prev_col_nodes[vertex+1]);
cout << "For cusp_index " << cusp_index << ", cusp_diff = " << cusp_diff << endl;
                for (col_index = 1; col_index < prev_col_nodes_size; col_index++)
                {
                  sum_diff = 0;
                  for (vertex = 0; vertex < col_nodes_size; vertex++)
                  {
                    if (vertex < col_index)
                      sum_diff += abs(col_nodes[vertex] - prev_col_nodes[vertex]);
                    else if (vertex >= col_index)
                      sum_diff += abs(col_nodes[vertex] - prev_col_nodes[vertex+1]);
                  }
cout << "For cusp_index " << col_index << ", cusp_diff = " << sum_diff << endl;
                  if (sum_diff < cusp_diff)
                  {
                    cusp_index = col_index;
                    cusp_diff = sum_diff;
                  }
                }
cout << "Mimimum cusp_diff = " << cusp_diff << " at cusp_index = " << cusp_index << endl;
               // Need to find closest column index in col_nodes to prev_col_nodes[cusp_index] and remove it
                col_index = 0;
                sum_diff = abs(col_nodes[0] - prev_col_nodes[cusp_index]);
                for (vertex = 1; vertex < col_nodes_size; vertex++)
                {
                  if ((col_nodes[vertex] - prev_col_nodes[cusp_index]) < sum_diff)
                  {
                    sum_diff = abs(col_nodes[vertex] - prev_col_nodes[cusp_index]);
                    col_index = vertex;
                  }
                }
                cusp_index = col_index;
cout << "Removed col_node " << (col_nodes[cusp_index] + col_min) << " from list" << endl;
cout << "New list is: ";
                for (vertex = 0; vertex < col_nodes_size; vertex++)
                {
                  if (vertex != cusp_index)
{
                    minimal_col_nodes.push_back(col_nodes[vertex]);
cout << (col_nodes[vertex] + col_min) << " ";
}
                }
cout << endl;
              }
              else
              {
                cusp_index = 0;
                cusp_diff = 0;
                for (vertex = 0; vertex < prev_col_nodes_size; vertex++)
                  cusp_diff += abs(col_nodes[vertex+1] - prev_col_nodes[vertex]);
cout << "For cusp_index " << cusp_index << ", cusp_diff = " << cusp_diff << endl;
                for (col_index = 1; col_index < prev_col_nodes_size; col_index++)
                {
                  sum_diff = 0;
                  for (vertex = 0; vertex < prev_col_nodes_size; vertex++)
                  {
                    if (vertex < col_index)
                      sum_diff += abs(col_nodes[vertex] - prev_col_nodes[vertex]);
                    else if (vertex >= col_index)
                      sum_diff += abs(col_nodes[vertex+1] - prev_col_nodes[vertex]);
                  }
cout << "For cusp_index " << col_index << ", cusp_diff = " << sum_diff << endl;
                  if (sum_diff < cusp_diff)
                  {
                    cusp_index = col_index;
                    cusp_diff = sum_diff;
                  }
                }
cout << "Mimimum cusp_diff = " << cusp_diff << " at cusp_index = " << cusp_index << endl;
cout << "Removed col_node " << (col_nodes[cusp_index] + col_min) << " from list" << endl;
cout << "New list is: ";
                for (vertex = 0; vertex < col_nodes_size; vertex++)
                {
                  if (vertex != cusp_index)
{
                    minimal_col_nodes.push_back(col_nodes[vertex]);
cout << (col_nodes[vertex] + col_min) << " ";
}
                }
              }
            }
            else
            {
              for (vertex = 0; vertex < col_nodes_size; vertex++)
                minimal_col_nodes.push_back(col_nodes[vertex]);
            }

           // Fill in polygon between pairs of column nodes
            prev_col_nodes.clear();
            col_nodes_size = minimal_col_nodes.size();
            for (vertex = 0; vertex < col_nodes_size; vertex+=2)
            {
              col1 = minimal_col_nodes[vertex];
              prev_col_nodes.push_back(col1);
              if ((vertex+1) == col_nodes_size)
              {
                cout << "WARNING(shape_to_image): Found odd number of column nodes when filling in interior of polygon" << endl;
                break;
              }
              col2 = minimal_col_nodes[vertex+1];
              prev_col_nodes.push_back(col2);
if (print_flag)
  cout << "Filling in polygon between col1 = " << (col1+col_min) << " and col2 = " << (col2+col_min) << endl;
              for (col_index = col1; col_index <= col2; col_index++)
              {
                temp_index = col_index + row1*temp_ncols;
                temp_fill[temp_index] = true;
              }
            }
          } // for (row1 = 1; row1 < (temp_nrows-1); row1++)

         // Complete the fill process by transferring the fill locations from the temporary sub_temp
          for (row1 = 0; row1 < temp_nrows; row1++)
          {
            for (col1 = 0; col1 < temp_ncols; col1++)
            {
              temp_index = col1 + row1*temp_ncols;
              if ((temp_polygon[temp_index]) || (temp_fill[temp_index]))
                put_data(nb_sites,(col1+col_min),(row1+row_min),0);
            }
          }
          flush_data();
        } // if (check_bounds2(col_min, row_min, col_max, row_max))
      } // for (entity = 0; entity < nb_entities; entity++)
    }

    return;
  }

  bool Image::check_bounds(const double& X_minBound, const double& Y_minBound, const double& X_maxBound, const double& Y_maxBound)
  {
    bool return_value = true;
    int X_min, Y_min, X_max, Y_max;
    
    X_min = (int) (X_minBound + 0.5 - X_offset);
    Y_min = (int) (Y_minBound + 0.5 - Y_offset);
    X_max = (int) (X_maxBound - 0.5 - X_offset);
    Y_max = (int) (Y_maxBound - 0.5 - Y_offset);

    if (!data_valid(X_min,Y_min,0))
    {
      cout << "WARNING: Shapefile location X_minBound = " << X_min << ", Y_minBound = " << Y_min;
      cout << " outside of image spatial coverage:" << endl;
      return_value = false;
    }

    if (!data_valid(X_min,Y_max,0))
    {
      cout << "WARNING: Shapefile location X_minBound = " << X_min << ", Y_maxBound = " << Y_max;
      cout << " outside of image spatial coverage:" << endl;
      return_value = false;
    }

    if (!data_valid(X_max,Y_min,0))
    {
      cout << "WARNING: Shapefile location X_maxBound = " << X_max << ", Y_minBound = " << Y_min;
      cout << " outside of image spatial coverage:" << endl;
      return_value = false;
    }

    if (!data_valid(X_max,Y_max,0))
    {
      cout << "WARNING: Shapefile location X_maxBound = " << X_max << ", Y_maxBound = " << Y_max;
      cout << " outside of image spatial coverage:" << endl;
      return_value = false;
    }

    return return_value;
  }

  bool Image::check_bounds(Shape shapeFile)
  {
    bool return_value = true;
    int X_min, Y_min, X_max, Y_max;
    
    X_min = (int) (shapeFile.get_X_minBound() + 0.5);
    Y_min = (int) (shapeFile.get_Y_minBound() + 0.5);
    X_max = (int) (shapeFile.get_X_maxBound() - 0.5);
    Y_max = (int) (shapeFile.get_Y_maxBound() - 0.5);

    if (!data_valid(X_min,Y_min,0))
    {
      cout << "WARNING: Shapefile location X_minBound = " << X_min << ", Y_minBound = " << Y_min;
      cout << " outside of image spatial coverage:" << endl;
      return_value = false;
    }

    if (!data_valid(X_min,Y_max,0))
    {
      cout << "WARNING: Shapefile location X_minBound = " << X_min << ", Y_maxBound = " << Y_max;
      cout << " outside of image spatial coverage:" << endl;
      return_value = false;
    }

    if (!data_valid(X_max,Y_min,0))
    {
      cout << "WARNING: Shapefile location X_maxBound = " << X_max << ", Y_minBound = " << Y_min;
      cout << " outside of image spatial coverage:" << endl;
      return_value = false;
    }

    if (!data_valid(X_max,Y_max,0))
    {
      cout << "WARNING: Shapefile location X_maxBound = " << X_max << ", Y_maxBound = " << Y_max;
      cout << " outside of image spatial coverage:" << endl;
      return_value = false;
    }

    return return_value;
  }

  bool Image::check_bounds2(const double& X_minBound, const double& Y_minBound, const double& X_maxBound, const double& Y_maxBound)
  {
    double X, Y;
    int col_min, row_min, col_max, row_max, temp_int;
    
    X = (X_minBound - X_offset)/X_gsd;
    Y = (Y_minBound - Y_offset)/Y_gsd;
    col_min = (int) offset_for_rounding(X);
    row_min = (int) offset_for_rounding(Y);
    X = (X_maxBound - X_offset)/X_gsd;
    Y = (Y_maxBound - Y_offset)/Y_gsd;
    col_max = (int) offset_for_rounding(X);
    row_max = (int) offset_for_rounding(Y);

    if (col_min > col_max)
    {
      temp_int = col_min;
      col_min = col_max;
      col_max = temp_int;
    }
    if (row_min > row_max)
    {
      temp_int = row_min;
      row_min = row_max;
      row_max = temp_int;
    }

    if ((col_max < 0) || (row_max < 0))
      return false;
    if ((col_min >= ncols) || (row_min >= nrows))
      return false;

    return true;
  }

  bool Image::check_bounds2(int& col_min, int& row_min, int& col_max, int& row_max)
  {
    int temp_int;

    if (col_min > col_max)
    {
      temp_int = col_min;
      col_min = col_max;
      col_max = temp_int;
    }
    if (row_min > row_max)
    {
      temp_int = row_min;
      row_min = row_max;
      row_max = temp_int;
    }

    if ((col_max < 0) || (row_max < 0))
      return false;
    if ((col_min >= ncols) || (row_min >= nrows))
      return false;

    return true;
  }

  bool Image::check_bounds2(Shape shapeFile)
  {
    double X, Y;
    int col_min, row_min, col_max, row_max, temp_int;
    
    X = (shapeFile.get_X_minBound() - X_offset)/X_gsd;
    Y = (shapeFile.get_Y_minBound() - Y_offset)/Y_gsd;
    col_min = (int) offset_for_rounding(X);
    row_min = (int) offset_for_rounding(Y);
    X = (shapeFile.get_X_maxBound() - X_offset)/X_gsd;
    Y = (shapeFile.get_Y_maxBound() - Y_offset)/Y_gsd;
    col_max = (int) offset_for_rounding(X);
    row_max = (int) offset_for_rounding(Y);
    if (col_min > col_max)
    {
      temp_int = col_min;
      col_min = col_max;
      col_max = temp_int;
    }
    if (row_min > row_max)
    {
      temp_int = row_min;
      row_min = row_max;
      row_max = temp_int;
    }

    if ((col_max < 0) || (row_max < 0))
      return false;
    if ((col_min >= ncols) || (row_min >= nrows))
      return false;

    return true;
  }

  void Image::convert_dpath_to_path(vector<dPoint>& dpath, vector<Point>& path)
  {
    double UTM_X, UTM_Y, X, Y;
    int col, row, vertex, dpath_size = dpath.size();

    path.clear();
    for (vertex = 0; vertex < dpath_size; vertex++)
    {
      UTM_X = dpath[vertex].get_X();
      UTM_Y = dpath[vertex].get_Y();
      X = (UTM_X - X_offset)/X_gsd;
      Y = (UTM_Y - Y_offset)/Y_gsd;
      col = (int) offset_for_rounding(X);
      row = (int) offset_for_rounding(Y);
      Point new_point(col, row);
      if ((vertex == 0) || (!(new_point == path[vertex-1])))
        path.push_back(new_point);
    }

    return;
  }
#endif

#ifndef GDAL
  void Image::clear_data()
  {
    int index, io_size = ncols*io_nrows_step;
    switch(dtype)
    { 
      case UInt8:   for (index = 0; index < io_size; index++)
                      byte_data[index] = 0;
                    break;
      case UInt16:  for (index = 0; index < io_size; index++)
                      ushort_data[index] = 0;
                    break;
      case UInt32:  for (index = 0; index < io_size; index++)
                      uint_data[index] = 0;
                    break;
      case Float32: for (index = 0; index < io_size; index++)
                      float_data[index] = 0.0;
                    break;
      default:      cout << "RHSEG style data type is invalid (i2)" << endl;
                    return;
    }

    return;
  }

 // Close a write_only image object and reopen it as a read_only image object
  void Image::close_and_reopen()
  {
    data_fs.close();
    read_flag = true;
#ifndef GDAL
    write_flag = false;
#endif
#ifdef THREEDIM
   // For 3-D, assume simple case where all data is contained in memory
    io_nrows_overlap = 0;
    io_nrows = io_nrows_step = nrows;
#else
    io_nrows_step = nrows;
#ifdef MAX_IO_SIZE
    int num_io_blocks = 1;
    while ((ncols*io_nrows_step) > (MAX_IO_SIZE/nbands))
    {
      io_nrows_step /= 2;
      num_io_blocks *= 2;
    }
    while (io_nrows_step*num_io_blocks < nrows)
      io_nrows_step++;
#ifdef IO_NROWS_OVERLAP
    io_nrows_overlap = IO_NROWS_OVERLAP;
#endif
#endif
    if (io_nrows_step == nrows)
    {
     // Simple case where all data is contained in memory
      io_nrows_overlap = 0;
      io_nrows = io_nrows_step;
    }
#endif // THREEDIM
    int index, io_size;
#ifdef THREEDIM
    io_size = ncols*nrows*nslices*nbands;
#else
    io_size = ncols*(io_nrows_step+io_nrows_overlap)*nbands;
#endif
    switch(dtype)
    { 
      case UInt8:   byte_data = new unsigned char[io_size];  
                    for (index = 0; index < io_size; index++)
                      byte_data[index] = 0;
                    break;
      case UInt16:  ushort_data = new unsigned short int[io_size];
                    for (index = 0; index < io_size; index++)
                      ushort_data[index] = 0;
                    break;
      case UInt32:  uint_data = new unsigned int[io_size];
                    for (index = 0; index < io_size; index++)
                      uint_data[index] = 0;
                    break;
      case Float32: float_data = new float[io_size];
                    for (index = 0; index < io_size; index++)
                      float_data[index] = 0.0;
                    break;
      default:      cout << "RHSEG style data type is invalid (i3)" << endl;
                    return;
    }
    data_fs.open(image_file.c_str(), fstream::in | fstream::out | fstream::binary); // Need to include "out" for seekg to work
    if (!data_fs)
    {
      cout << "ERROR(image): Error opening image file " << image_file << " for reading" << endl;
      return;
    }
    io_nrows_offset = 0;
    io_nrows = (io_nrows_step+io_nrows_overlap);
    read_data();

    return;
  }
#endif

  void Image::flush_data()
  {
    if (io_nrows_offset < 0)
     io_nrows_offset = 0;
    write_data();

    return;
  }

  void Image::close()
  {
#ifdef GDAL
//    delete imageDataset; // old style close
    if (imageDataset != NULL)
      GDALClose( (GDALDatasetH) imageDataset) ;
#else
    data_fs.close();
#endif
    if (byte_data != NULL)
      delete [] byte_data;
    if (ushort_data != NULL)
      delete [] ushort_data;
    if (uint_data != NULL)
      delete [] uint_data;
    if (float_data != NULL)
      delete [] float_data;
#ifdef GDAL
    if (short_data != NULL)
      delete [] short_data;
    if (int_data != NULL)
      delete [] int_data;
    if (double_data != NULL)
      delete [] double_data;
#endif
    construct();

    return;
  }

 // Print parameters
 void Image::print_info()
 {
   int band;

  // Print image information
#ifdef GDAL
   if (gdal_flag)
   {
     cout << endl << "Formatted image data file = " << image_file << endl;
     cout << "Image I/O driver type description = " << driver_description << endl;
   }
   else
   {
     cout << endl << "Raw format image data file = " << image_file << endl;
     cout << "GDAL information invalid" << endl;
   }
#else
   cout << endl << "Raw format image data file = " << image_file << endl;
#endif
   if (info_flag)
   {
     cout << "Image number of columns = " <<  ncols << endl;
     cout << "Image number of rows = " <<  nrows << endl;
#ifdef THREEDIM
     cout << "Image number of slices = " <<  nslices << endl;
#endif
     cout << "Image number of bands = " <<  nbands << endl;
     cout << "Number of rows step increment for image I/O = " << io_nrows_step << endl;
#ifdef GDAL
     if (gdal_flag)
     {
       switch(data_type)
       {
         case GDT_Byte:    cout << "Data type is UNSIGNED CHAR (8 bit)" << endl;
                           break;
         case GDT_UInt16:  cout << "Data type is UNSIGNED SHORT INT (16 bit)" << endl;
                           break;
         case GDT_Int16:   cout << "Data type is SIGNED SHORT INT (16 bit)" << endl;
                           break;
         case GDT_UInt32:  cout << "Data type is UNSIGNED INT (32 bit)" << endl;
                           break;
         case GDT_Int32:   cout << "Data type is SIGNED INT (32 bit)" << endl;
                           break;
         case GDT_Float32: cout << "Data type is FLOAT (32 bit)" << endl;
                           break;
         case GDT_Float64: cout << "Data type is DOUBLE (64 bit)" << endl;
                           break;
         default:          cout << "Unknown or unsupported image data type" << endl;
                           break;
       }
     }
     else
     {
#endif
       switch(dtype)
       {
         case UInt8:   cout << "RHSEG style data type is \"UInt8\" (8-bit UNSIGNED CHAR)" << endl;
                       break;
         case UInt16:  cout << "RHSEG style data type is \"UInt16\" (16-bit UNSIGNED SHORT INT)" << endl;
                       break;
         case UInt32:  cout << "RHSEG style data type is \"UInt32\" (32-bit UNSIGNED SHORT INT)" << endl;
                       break;
         case Float32: cout << "RHSEG style data type is \"Float32\" (32-bit FLOAT)" << endl;
                       break;
         default:      cout << "RHSEG style data type is invalid (i4)" << endl;
                       break;
       }
#ifdef GDAL
     }
#endif
     for (band = 0; band < nbands; band++)
     {
       if (min_value_flag[band])
         cout << " For band " << band << ", data minimum = " << min_value[band] << endl;
       if (max_value_flag[band])
         cout << " For band " << band << ", data maximum = " << max_value[band] << endl;
       if (std_dev_flag[band])
         cout << " For band " << band << ", data standard deviation = " << std_dev_value[band] << endl;
       if (no_data_value_flag[band])
         cout << " For band " << band << ", no data value = " << no_data_value[band] << endl;
     }
   }
   else
     cout << "Image information invalid" << endl;
#ifdef GDAL
   if (projection_type != "")
     cout << "Projection type = " << projection_type << endl;
#endif
   if (geotransform_flag)
   {
     cout.precision(12);
     cout << " X offset = " << X_offset << ", Y offset = " << Y_offset;
     cout << ", X gsd = " << X_gsd << ", and Y gsd = " << Y_gsd << endl;
     cout.precision(6);
   }
   else
     cout << "Geo transform data invalid" << endl;
   if (data_flag)
     cout << "Image data valid" << endl;
   else
     cout << "Image data invalid" << endl;

   if (red_display_band >= 0)
   {
     cout << "Red display band = " << red_display_band << ", green display band = " << green_display_band;
     cout << " and blue display band = " << blue_display_band << endl;
   }
   if (read_flag)
     cout << "Opened in READ ONLY mode" << endl;
#ifndef GDAL
   if (write_flag)
     cout << "Opened in WRITE ONLY mode" << endl;
#endif
/*
   if ((!histo_eq_map_flag.empty()) && (histo_eq_map_flag[0]))
   {
     cout << "The histogram equalization map is valid" << endl;
     int index, size;
     cout << endl << "Red Histogram Equalization Map:" << endl;
     size = histo_eq_map[red_display_band].size();
     for (index = 0; index < size; index++)
     {
       cout << "  histo_eq_map[" << index << "] = " << histo_eq_map[red_display_band][index] << endl;
     }
     cout << endl;
     cout << endl << "Green Histogram Equalization Map:" << endl;
     size = histo_eq_map[green_display_band].size();
     for (index = 0; index < size; index++)
     {
       cout << "  histo_eq_map[" << index << "] = " << histo_eq_map[green_display_band][index] << endl;
     }
     cout << endl;
     cout << endl << "Blue Histogram Equalization Map:" << endl;
     size = histo_eq_map[blue_display_band].size();
     for (index = 0; index < size; index++)
     {
       cout << "  histo_eq_map[" << index << "] = " << histo_eq_map[blue_display_band][index] << endl;
     }
     cout << endl;
   }
*/
   return;
 }

// Friend functions
  void compute_ndvi(Image& NDVIImage, Image& NIRImage, Image& redImage)
  {
    Image maskImage;
    compute_ndvi(NDVIImage, NIRImage, redImage, maskImage);
    return;
  }

  void compute_ndvi(Image& NDVIImage, Image& NIRImage, Image& redImage, Image& maskImage)
  {
    bool mask_flag;
    int col, row, mask_col, mask_row;
    double UTM_X, UTM_Y, mask_X, mask_Y;
    double NIR_value, red_value, ndvi_value;

    NDVIImage.no_data_value[0] = -2;
    NDVIImage.no_data_value_flag[0] = true;
    NDVIImage.min_value[0] = 1.0;
    NDVIImage.min_value_flag[0] = true;
    NDVIImage.max_value[0] = -1.0;
    NDVIImage.max_value_flag[0] = true;

    for (row = 0; row < NDVIImage.nrows; row ++)
      for (col = 0; col < NDVIImage.ncols; col ++)
      {
        ndvi_value = NIR_value = red_value = NDVIImage.no_data_value[0];
        if (maskImage.data_valid())
        {
          if (NDVIImage.geotransform_flag && maskImage.geotransform_valid())
          {
            UTM_X = NDVIImage.X_offset + col*NDVIImage.X_gsd;
            UTM_Y = NDVIImage.Y_offset + row*NDVIImage.Y_gsd;
            mask_X = (UTM_X - maskImage.X_offset)/maskImage.X_gsd;
            mask_Y = (UTM_Y - maskImage.Y_offset)/maskImage.Y_gsd;
            mask_col = (int) NDVIImage.offset_for_rounding(mask_X);
            mask_row = (int) NDVIImage.offset_for_rounding(mask_Y);
          }
          else
          {
            mask_col = col;
            mask_row = row;
          }
          mask_flag = (maskImage.data_valid(mask_col,mask_row,0));
        }
        else
          mask_flag = true;
        if (mask_flag && (NIRImage.data_valid(col,row,0))&& (redImage.data_valid(col,row,0)))
        {
          NIR_value = NIRImage.get_data(col,row,0);
          red_value = redImage.get_data(col,row,0);
          if ((NIR_value+red_value) != 0.0)
            ndvi_value = (NIR_value - red_value) / (NIR_value + red_value);
          else
            ndvi_value = 0.0;
          if (ndvi_value < NDVIImage.min_value[0])
            NDVIImage.min_value[0] = ndvi_value;
          if (ndvi_value > NDVIImage.max_value[0])
            NDVIImage.max_value[0] = ndvi_value;
        }
        NDVIImage.put_data(ndvi_value,col,row,0);
      }
    NDVIImage.flush_data();

    return;
  }

// Private functions
  void Image::construct()
  {
    gdal_flag = false;
    image_file = "";
#ifdef GDAL
    imageDataset = NULL;
    driver = NULL;
    driver_description = "";
#endif

    info_flag = false;
    nslices = 1;
    dtype = Unknown;
#ifdef GDAL
    data_type = GDT_Unknown;
#endif
    io_nrows_offset = -1;
    io_nrows_overlap = 0;
    read_flag = false;
#ifndef GDAL
    write_flag = false;
    io_band = -1;
#endif

    geotransform_flag = false;
#ifdef GDAL
    projection_type = "";
#endif

    data_flag = false;
    byte_data = NULL;
    ushort_data = NULL;
    uint_data = NULL;
    float_data = NULL;
#ifdef GDAL
    short_data = NULL;
    int_data = NULL;
    double_data = NULL;
#endif

    red_display_band = green_display_band = blue_display_band = -1;
    rgb_image_stretch = 2;
    range[0] = 0.1;
    range[1] = 0.9;

    return;
  }

 // Initialize the image data structure
  bool Image::initialize(const bool& r_flag)
  {
    read_flag = r_flag;
#ifndef GDAL
    write_flag = !read_flag;
    if (write_flag)
      io_band = 0;
#endif

    io_nrows_step = nrows;
#ifdef MAX_IO_SIZE
    int num_io_blocks = 1;
#ifndef GDAL
    if (write_flag)
    {
      while ((ncols*io_nrows_step) > MAX_IO_SIZE)
      {
        io_nrows_step /= 2;
        num_io_blocks *= 2;
      }
    }
    else
    {
#endif
      while ((ncols*io_nrows_step) > (MAX_IO_SIZE/nbands))
      {
        io_nrows_step /= 2;
        num_io_blocks *= 2;
      }
#ifndef GDAL
    }
#endif
    while (io_nrows_step*num_io_blocks < nrows)
      io_nrows_step++;
#ifdef IO_NROWS_OVERLAP
    io_nrows_overlap = IO_NROWS_OVERLAP;
#ifndef GDAL
    if (write_flag)
      io_nrows_overlap = 0; // No overlapping in write only mode. 
#endif // !GDAL
#endif // IO_NROWS_OVERLAP
#endif // MAX_IO_SIZE
    if (io_nrows_step == nrows)
    {
     // Simple case where all data is contained in memory
      io_nrows_overlap = 0;
      io_nrows = io_nrows_step;
    }
#ifndef GDAL
#ifdef THREEDIM
   // For 3-D, assume simple case where all data is contained in memory (overriding previous settings)
    io_nrows_overlap = 0;
    io_nrows = io_nrows_step = nrows;
#endif
#endif

    no_data_value.resize(nbands);
    no_data_value_flag.resize(nbands);
    min_value.resize(nbands);
    max_value.resize(nbands);
    min_value_flag.resize(nbands);
    max_value_flag.resize(nbands);
    std_dev_value.resize(nbands);
    std_dev_flag.resize(nbands);
    int band;
#ifdef GDAL
    int gotValue;
#endif
    for (band = 0; band < nbands; band++)
    {
#ifdef GDAL
      if (gdal_flag)
      {
        no_data_value[band] = imageDataset->GetRasterBand(band+1)->GetNoDataValue(&gotValue);
        no_data_value_flag[band] = (gotValue != false);
        min_value[band] = imageDataset->GetRasterBand(band+1)->GetMinimum(&gotValue);
        min_value_flag[band] = (gotValue != false);
        max_value[band] = imageDataset->GetRasterBand(band+1)->GetMaximum(&gotValue);
        max_value_flag[band] = (gotValue != false);
      }
      else
      {
#else
        no_data_value_flag[band] = false;
        min_value_flag[band] = false;
        max_value_flag[band] = false;
#endif
#ifdef GDAL
      }
#endif
      std_dev_flag[band] = false;
    }
    info_flag = true;

    allocate_data();
    data_flag = true;

    for (band = 0; band < nbands; band++)
      histo_eq_map_flag.push_back(false);
    histo_scale.resize(nbands);
    histo_offset.resize(nbands);
    histo_eq_map.resize(nbands);
    histo_lookup.resize(nbands);

    return true;
  }

  void Image::allocate_data()
  {
    int index, io_size;

#ifdef THREEDIM // THREEDIM can only be defined if GDAL is not defined
    io_size = ncols*nrows*nslices*nbands;
#else
    io_size = ncols*(io_nrows_step+io_nrows_overlap)*nbands;
#ifndef GDAL
    if (write_flag)
    {
      io_size = ncols*io_nrows_step;
      if (io_nrows_step == nrows)
        io_size = ncols*nrows*nbands;
    }
#endif
#endif // THREEDIM
    double value = 0.0;
    if (no_data_value_flag[0])
      value = no_data_value[0];
    
#ifdef GDAL
    if (gdal_flag)
    {
      switch(data_type)
      {
        case GDT_Byte:    byte_data = new unsigned char[io_size];
                          for (index = 0; index < io_size; index++)
                            byte_data[index] = (unsigned char) value;
                          break;
        case GDT_UInt16:  ushort_data = new unsigned short int[io_size];
                          for (index = 0; index < io_size; index++)
                            ushort_data[index] = (unsigned short int) value;
                          break;
        case GDT_Int16:   short_data = new short int[io_size];
                          for (index = 0; index < io_size; index++)
                            short_data[index] = (short int) value;
                          break;
        case GDT_UInt32:  uint_data = new unsigned int[io_size];
                          for (index = 0; index < io_size; index++)
                            uint_data[index] = (unsigned int) value;
                          break;
        case GDT_Int32:   int_data = new int[io_size];
                          for (index = 0; index < io_size; index++)
                            int_data[index] = (int) value;
                          break;
        case GDT_Float32: float_data = new float[io_size];
                          for (index = 0; index < io_size; index++)
                            float_data[index] = (float) value;
                          break;
        case GDT_Float64: double_data = new double[io_size];
                          for (index = 0; index < io_size; index++)
                            double_data[index] = value;
                          break;
        default:          cout << "Unknown or unsupported image data type" << endl;
                          return;
      }
    }
    else
    {
#endif
      switch(dtype)
      { 
        case UInt8:   byte_data = new unsigned char[io_size];  
                      for (index = 0; index < io_size; index++)
                        byte_data[index] = (unsigned char) value;
                      break;
        case UInt16:  ushort_data = new unsigned short int[io_size];
                      for (index = 0; index < io_size; index++)
                        ushort_data[index] = (unsigned short int) value;
                      break;
        case UInt32:  uint_data = new unsigned int[io_size];
                      for (index = 0; index < io_size; index++)
                        uint_data[index] = (unsigned int) value;
                      break;
        case Float32: float_data = new float[io_size];
                      for (index = 0; index < io_size; index++)
                        float_data[index] = (float) value;
                      break;
        default:      cout << "RHSEG style data type is invalid (i1)" << endl;
                      return;
      }
#ifdef GDAL
    }
#endif

    return;
  }

  void Image::read_data()
  {
    if (io_nrows_offset >= 0) // If < 0, invalid data block to read from!!
    {
#ifdef GDAL
      if (gdal_flag)
      {
        switch(data_type)
        {
          case GDT_Byte:    imageDataset->RasterIO(GF_Read,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(byte_data),ncols,io_nrows,
                                          GDT_Byte,nbands,NULL,0,0,0);
                            break;
          case GDT_UInt16:  imageDataset->RasterIO(GF_Read,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(ushort_data),ncols,io_nrows,
                                          GDT_UInt16,nbands,NULL,0,0,0);
                            break;
          case GDT_Int16:   imageDataset->RasterIO(GF_Read,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(short_data),ncols,io_nrows,
                                          GDT_Int16,nbands,NULL,0,0,0);
                            break;
          case GDT_UInt32:  imageDataset->RasterIO(GF_Read,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(uint_data),ncols,io_nrows,
                                          GDT_UInt32,nbands,NULL,0,0,0);
                            break;
          case GDT_Int32:   imageDataset->RasterIO(GF_Read,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(int_data),ncols,io_nrows,
                                          GDT_Int32,nbands,NULL,0,0,0);
                            break;
          case GDT_Float32: imageDataset->RasterIO(GF_Read,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(float_data),ncols,io_nrows,
                                          GDT_Float32,nbands,NULL,0,0,0);
                            break; 
          case GDT_Float64: imageDataset->RasterIO(GF_Read,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(double_data),ncols,io_nrows,
                                          GDT_Float64,nbands,NULL,0,0,0);
                            break;
          default:          cout << "Unknown or unsupported image data type" << endl;
                            break;
        }
      }
      else
      {
#else
        if (write_flag)
        {
          cout << "WARNING(image) - Tried to read from an image data file opened in WRITE ONLY mode" << endl;
          print_info();
          return;
        }
#endif
#ifdef THREEDIM
        int io_size = ncols*nrows*nslices*nbands;
#else
        int io_size = ncols*io_nrows;
#endif
        int band, position;
        for (band = 0; band < nbands; band++)
        {
#ifdef THREEDIM
          position = 0;
#else
          position = ncols*io_nrows_offset + band*ncols*nrows;
#endif
          switch(dtype)
          {
            case UInt8:   data_fs.seekg(position,ios_base::beg); 
                          data_fs.read(reinterpret_cast<char *>(&byte_data[band*ncols*io_nrows]),io_size);
                          break;
            case UInt16:  data_fs.seekg(2*position,ios_base::beg); 
                          data_fs.read(reinterpret_cast<char *>(&ushort_data[band*ncols*io_nrows]),2*io_size);
                          break;
            case UInt32:  data_fs.seekg(4*position,ios_base::beg); 
                          data_fs.read(reinterpret_cast<char *>(&uint_data[band*ncols*io_nrows]),4*io_size);
                          break;
            case Float32: data_fs.seekg(4*position,ios_base::beg); 
                          data_fs.read(reinterpret_cast<char *>(&float_data[band*ncols*io_nrows]),4*io_size);
                          break;
            default:      cout << "RHSEG style data type is invalid (i5)" << endl;
                          break;
          }
        }
#ifdef GDAL
      }
#endif
    }

    return;
  }

  void Image::write_data()
  {
    if (read_flag)
    {
      cout << "WARNING(image) - Tried to write to an image data file opened in READ ONLY mode" << endl;
      return;
    }

    if (io_nrows_offset >= 0) // No data to flush if < 0!!
    {
#ifdef GDAL
      if (gdal_flag)
      {
        switch(data_type)
        {
          case GDT_Byte:    imageDataset->RasterIO(GF_Write,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(byte_data),ncols,io_nrows,
                                          GDT_Byte,nbands,NULL,0,0,0);
                            break;
          case GDT_UInt16:  imageDataset->RasterIO(GF_Write,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(ushort_data),ncols,io_nrows,
                                          GDT_UInt16,nbands,NULL,0,0,0);
                            break;
          case GDT_Int16:   imageDataset->RasterIO(GF_Write,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(short_data),ncols,io_nrows,
                                          GDT_Int16,nbands,NULL,0,0,0);
                            break;
          case GDT_UInt32:  imageDataset->RasterIO(GF_Write,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(uint_data),ncols,io_nrows,
                                          GDT_UInt32,nbands,NULL,0,0,0);
                            break;
          case GDT_Int32:   imageDataset->RasterIO(GF_Write,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(int_data),ncols,io_nrows,
                                          GDT_Int32,nbands,NULL,0,0,0);
                            break;
          case GDT_Float32: imageDataset->RasterIO(GF_Write,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(float_data),ncols,io_nrows,
                                          GDT_Float32,nbands,NULL,0,0,0);
                            break; 
          case GDT_Float64: imageDataset->RasterIO(GF_Write,0,io_nrows_offset,ncols,io_nrows,
                                          reinterpret_cast<char *>(double_data),ncols,io_nrows,
                                          GDT_Float64,nbands,NULL,0,0,0);
                            break;
          default:          cout << "Unknown or unsupported image data type" << endl;
                            break;
        }
      }
      else
      {
#endif
#ifdef THREEDIM
        int io_size = ncols*nrows*nslices*nbands;
#else
        int io_size = ncols*io_nrows;
        if (io_nrows == nrows)
          io_size = ncols*nrows*nbands;
#endif
        switch(dtype)
        {
            case UInt8:   data_fs.write(reinterpret_cast<char *>(&byte_data[0]),io_size);
                          break;
            case UInt16:  data_fs.write(reinterpret_cast<char *>(&ushort_data[0]),2*io_size);
                          break;
            case UInt32:  data_fs.write(reinterpret_cast<char *>(&uint_data[0]),4*io_size);
                          break;
            case Float32: data_fs.write(reinterpret_cast<char *>(&float_data[0]),4*io_size);
                          break;
            default:      cout << "RHSEG style data type is invalid (i6)" << endl;
                          break;
        }
#ifdef GDAL
      }
#endif
    }

    return;
  }

// Called by shape_to_image.
  void Image::plot_bool_line(const int& col1, const int& col2, const int& row1, const int& row2,
                             const int& nbcols, vector<bool>& bool_array)
  {
    int col, row, bool_index;

    if ((col1 == col2) && (row1 == row2))
    {
      bool_index = col1 + row1*nbcols;
      bool_array[bool_index] = true;
      return;
    }

    if (col1 == col2)
    {
      if (row2 > row1)
      {
        for (row = row1; row <= row2; row++)
        {
          bool_index = col1 + row*nbcols;
          bool_array[bool_index] = true;
        }
      }
      else
      {
        for (row = row2; row <= row1; row++)
        {
          bool_index = col1 + row*nbcols;
          bool_array[bool_index] = true;
        }
      }
    }

    if (row1 == row2)
    {
      if (col2 > col1)
      {
        for (col = col1; col <= col2; col++)
        {
          bool_index = col + row1*nbcols;
          bool_array[bool_index] = true;
        }
      }
      else
      {
        for (col = col2; col <= col1; col++)
        {
          bool_index = col + row1*nbcols;
          bool_array[bool_index] = true;
        }
      }
    }

    double x, y, x1, x2, y1, y2;
    x1 = col1;
    x2 = col2;
    y1 = row1;
    y2 = row2;
    if (abs(col1-col2) <= abs(row1-row2))
    {
      if (row2 > row1)
      {
        for (row = row1; row <= row2; row++)
        {
          y = row;
          x = x1 + (x2-x1)*(y-y1)/(y2-y1);
          col = (int) offset_for_rounding(x);
          bool_index = col + row*nbcols;
          bool_array[bool_index] = true;
        }
      }
      else
      {
        for (row = row2; row <= row1; row++)
        {
          y = row;
          x = x1 + (x2-x1)*(y-y1)/(y2-y1);
          col = (int) offset_for_rounding(x);
          bool_index = col + row*nbcols;
          bool_array[bool_index] = true;
        }
      }
    }
    else
    {
      if (col2 > col1)
      {
        for (col = col1; col <= col2; col++)
        {
          x = col;
          y = y1 + (y2-y1)*(x-x1)/(x2-x1);
          row = (int) offset_for_rounding(y);
          bool_index = col + row*nbcols;
          bool_array[bool_index] = true;
        }
      }
      else
      {
        for (col = col2; col <= col1; col++)
        {
          x = col;
          y = y1 + (y2-y1)*(x-x1)/(x2-x1);
          row = (int) offset_for_rounding(y);
          bool_index = col + row*nbcols;
          bool_array[bool_index] = true;
        }
      }
    }

    return;
  }

  void Image::interpolate(const double& continuity_ratio)
  {
    int col, row, band;
    double data_value[4], interp_value;
 
 // Interpolate odd columns of even rows
    for (band = 0; band < nbands; band++)
    {
      for (row = 0; row < nrows; row += 2)
        for (col = 1; col < ncols; col += 2)
        {
          if (data_valid((col-1),row,band) && data_valid((col+1),row,band))
          {
            data_value[0] = get_data((col-1),row,band);
            data_value[1] = get_data((col+1),row,band);
          // Gross continuity check!!
            if ((fabs((data_value[0]-data_value[1])/(data_value[0]+data_value[1])) < continuity_ratio) ||
                (fabs(data_value[0]-data_value[1]) < 100.0))
              interp_value = (data_value[0]+data_value[1])/2.0;
            else
            {
              if (no_data_value_flag[band])
                interp_value = no_data_value[band];
              else
                interp_value = 0.0;
cout << "Gross continuity check failed for band " << band << " at (row,col) = (" << row << "," << col << ")";
cout << " with delta = " << fabs(data_value[0]-data_value[1]) << " and sum = " << fabs(data_value[0]+data_value[1]) << endl;
cout << "Ratio = " << fabs((data_value[0]-data_value[1])/(data_value[0]+data_value[1])) << " and data values: ";
for (int i = 0; i < 2; i++)
  cout << data_value[i] << " ";
cout << endl;
            }
          }
          else
          {
            if (no_data_value_flag[band])
              interp_value = no_data_value[band];
            else
              interp_value = 0.0;
          }
          put_data(interp_value,col,row,band);
        }
      flush_data();
    }

  // Interpolate odd rows and even columns
    for (band = 0; band < nbands; band++)
    {
      for (row = 1; row < nrows; row += 2)
        for (col = 0; col < ncols; col += 2)
        {
          if (data_valid(col,(row-1),band) && data_valid(col,(row+1),band))
          {
            data_value[0] = get_data(col,(row-1),band);
            data_value[1] = get_data(col,(row+1),band);
          // Gross continuity check!!
            if ((fabs((data_value[0]-data_value[1])/(data_value[0]+data_value[1])) < continuity_ratio) ||
                (fabs(data_value[0]-data_value[1]) < 100.0))
              interp_value = (data_value[0]+data_value[1])/2.0;
            else
            {
              if (no_data_value_flag[band])
                interp_value = no_data_value[band];
              else
                interp_value = 0.0;
cout << "Gross continuity check failed for band " << band << " at (row,col) = (" << row << "," << col << ")";
cout << " with delta = " << fabs(data_value[0]-data_value[1]) << " and sum = " << fabs(data_value[0]+data_value[1]) << endl;
cout << "Ratio = " << fabs((data_value[0]-data_value[1])/(data_value[0]+data_value[1])) << " and data values: ";
for (int i = 0; i < 2; i++)
  cout << data_value[i] << " ";
cout << endl;
            }
          }
          else
          {
            if (no_data_value_flag[band])
              interp_value = no_data_value[band];
            else
              interp_value = 0.0;
          }
          put_data(interp_value,col,row,band);
        }
      flush_data();
    }

  // Interpolate odd rows and columns
    int index;
    double min_value, max_value;
    for (band = 0; band < nbands; band++)
    {
      for (row = 1; row < nrows; row += 2)
        for (col = 1; col < ncols; col += 2)
        {
          if (data_valid((col-1),(row-1),band) && data_valid((col+1),(row-1),band) &&
              data_valid((col-1),(row+1),band) && data_valid((col+1),(row+1),band))
          {
            data_value[0] = get_data((col-1),(row-1),band);
            data_value[1] = get_data((col+1),(row-1),band);
            data_value[2] = get_data((col-1),(row+1),band);
            data_value[3] = get_data((col+1),(row+1),band);
            min_value = max_value  = data_value[0];
            for (index = 1; index < 4; index++)
            {
              if (data_value[index] < min_value)
                min_value = data_value[index];
              if (data_value[index] > max_value)
                max_value = data_value[index];
            }
          // Gross continuity check!!
            if ((((max_value-min_value)/(max_value+min_value)) < continuity_ratio) ||
                ((max_value-min_value) < 100))
              interp_value = (data_value[0]+data_value[1]+data_value[2]+data_value[3])/4.0;
            else
            {
              if (no_data_value_flag[band])
                interp_value = no_data_value[band];
              else
                interp_value = 0.0;
cout << "Gross continuity check failed for band " << band << " at (row,col) = (" << row << "," << col << ")";
cout << " with delta = " << (max_value-min_value) << " and sum = " << max_value+min_value << endl;
cout << "Ratio = " << ((max_value-min_value)/(max_value+min_value)) << " and data values: ";
cout << "Data values: ";
for (int i = 0; i < 4; i++)
  cout << data_value[i] << " ";
cout << endl;
            }
          }
          else
          {
            if (no_data_value_flag[band])
              interp_value = no_data_value[band];
            else
              interp_value = 0.0;
          }
          put_data(interp_value,col,row,band);
        }
      flush_data();
    }
    return;
  }

} // namespace CommonTilton
