// hdf_extract.cc

#include "hdf_extract.h"
#include "params/params.h"
#include <iostream>

extern CommonTilton::Params params;
CommonTilton::Image outputImage;

namespace CommonTilton
{

  bool hdf_extract()
  {
    int band;
    GDALDataset *poDataset;

    poDataset = (GDALDataset *)GDALOpen(params.input_HDF.c_str(), GA_ReadOnly);
    if (!poDataset)
    {
      cout << "Could not open input_HDF file name = " << params.input_HDF << endl;
      return false;
    }

    bool status = false;
    GDALDataType dtype;
    int ncols, nrows;
    bool found_no_data_value_flag, no_data_value_flag = false;
    double found_no_data_value, no_data_value;
    Image nullImage;
    string projection_type;
    bool geotransform_valid = false;
    double geotransform[6];
    bool first_flag = true;
   // Loop through all subdatasets
    char **metadata = poDataset->GetMetadata("SUBDATASETS");
    if (metadata) 
    {
      // need to skip SUBDATASET_?_DESC;
      // we only need SUBDATASET_?_NAME here
      for (char **meta = metadata; *meta; meta += 2) 
      {
	const char *name = strstr(*meta, "=");
	if (!name)
        {
          cout << "Could not find SUBDATASET_#_NAME " << endl;
          return false;
        }
	else
        { 
          const char *filename = (name + 1);
          bool found_flag = false;
          for (band = 0; band < params.nbands; band++)
          {
            const char *match = strstr(filename,params.subdatasets[band].c_str());
            if (match)
              found_flag = true;
          }
          if (found_flag)
          {
            status = true;
            if (!hdf_query(filename,found_no_data_value,found_no_data_value_flag,nullImage))
              status = false;
            if (first_flag)
            {
              ncols = nullImage.get_ncols();
              nrows = nullImage.get_nrows();
              dtype = nullImage.get_data_type();
              no_data_value = found_no_data_value;
              no_data_value_flag = found_no_data_value_flag;
              if (nullImage.geotransform_valid())
              {
                geotransform_valid = true;
                projection_type = nullImage.get_projection_type();
                nullImage.get_geotransform(geotransform);
              }
              first_flag = false;
            }
            else
            {
              if (ncols != nullImage.get_ncols())
              {
                cout << "Inconsistency in image number of columns detected." << endl;
                cout << "Previous subdataset(s) had ncols = " << ncols << endl;
                cout << "Subdataset = " << filename << " has ncols = " << ncols << endl;
                cout << "hdf_extract aborted" << endl;
                return false;
              }
              if (nrows != nullImage.get_nrows())
              {
                cout << "Inconsistency in image number of rows detected." << endl;
                cout << "Previous subdataset(s) had nrows = " << nrows << endl;
                cout << "Subdataset = " << filename << " has nrows = " << nrows << endl;
                cout << "hdf_extract aborted" << endl;
                return false;
              }
              if (no_data_value_flag != found_no_data_value_flag)
              {
                cout << "Inconsistency in existance of no data fill value detected." << endl;
                if (no_data_value_flag)
                {
                  cout << "Previous subdataset(s) had no_data_value_flag = true " << endl;
                  cout << "Subdataset = " << filename << " has no_data_value_flag = false " << endl;
                }
                else
                {
                  cout << "Previous subdataset(s) had no_data_value_flag = false " << endl;
                  cout << "Subdataset = " << filename << " has no_data_value_flag = true " << endl;
                }
                cout << "hdf_extract aborted" << endl;
                return false;
              }
              if (no_data_value_flag && found_no_data_value_flag)
              {
                if (no_data_value != found_no_data_value)
                {
                  cout << "Inconsistency in no data fill value detected." << endl;
                  cout << "Previous subdataset(s) had no data fill value = " << no_data_value << endl;
                  cout << "Subdataset = " << filename << " has no data fill value = " << found_no_data_value << endl;
                  cout << "hdf_extract aborted" << endl;
                  return false;
                }
              }
              if (geotransform_valid != nullImage.geotransform_valid())
              {
                cout << "Inconsistency in existance of geotransfrom information detected." << endl;
                if (geotransform_valid)
                {
                  cout << "Previous subdataset(s) had valid geotransform information " << endl;
                  cout << "Subdataset = " << filename << " has no geotranform information " << endl;
                }
                else
                {
                  cout << "Previous subdataset(s) had no geotransform information " << endl;
                  cout << "Subdataset = " << filename << " has valid geotranform information " << endl;
                }
                cout << "hdf_extract aborted" << endl;
                return false;
              }
              if (geotransform_valid && nullImage.geotransform_valid())
              {
                string found_projection_type = nullImage.get_projection_type();
                if (strcmp(projection_type.c_str(),found_projection_type.c_str()) != 0)
                {
                  cout << "Inconsistency in projection type detected." << endl;
                  cout << "Previous subdataset(s) had projection type = " << projection_type << endl;
                  cout << "Subdataset = " << filename << " has projection_type = " << found_projection_type << endl;
                  cout << "hdf_extract aborted" << endl;
                  return false;
                }
                bool different_flag = false;
                for (int i = 0; i < 6; i++)
                  if (geotransform[i] != nullImage.get_imageGeoTransform(i))
                    different_flag = true;
                if (different_flag)
                {
                  cout << "Inconsistency in geotransform information detected." << endl;
                  cout << "Previous subdataset(s) had geotransform values: ";
                  for (int i = 0; i < 6; i++)
                    cout << geotransform[i] << " ";
                  cout << endl;
                  cout << "Subdataset = " << filename << " has eotransform values: ";
                  for (int i = 0; i < 6; i++)
                    cout << nullImage.get_imageGeoTransform(i) << " ";
                  cout << endl;
                  cout << "hdf_extract aborted" << endl;
                  return false;
                }
              }
              if (nullImage.get_data_type() > dtype)
              {
                dtype = nullImage.get_data_type();
                cout << "Upconverting all subdatasets to data type = " << GDALGetDataTypeName(dtype) << endl;
              }
            }
          }
        }
      }
    }

    if (status)
    {
      int process_band;
      outputImage.create(params.output_image, ncols, nrows, params.nbands, dtype, params.output_format);
      metadata = poDataset->GetMetadata("");
      outputImage.set_metadata(metadata,"");
      if (no_data_value_flag)
      {
        outputImage.set_no_data_value(no_data_value);
        outputImage.put_no_data_value();
      }
      if (geotransform_valid)
      {
        outputImage.set_projection_type(projection_type);
        outputImage.set_geotransform(geotransform);
        outputImage.put_geotransform();
      }
      outputImage.print_info();
     // Loop through all subdatasets
      metadata = poDataset->GetMetadata("SUBDATASETS");
      if (metadata) 
      {
      // need to skip SUBDATASET_?_DESC;
      // we only need SUBDATASET_?_NAME here
        for (char **meta = metadata; *meta; meta += 2) 
        {
	  const char *name = strstr(*meta, "=");
          const char *filename = (name + 1);
          bool found_flag = false;
          for (band = 0; band < params.nbands; band++)
          {
            const char *match = strstr(filename,params.subdatasets[band].c_str());
            if (match)
            {
              found_flag = true;
              process_band = band;
            }
          }
          if (found_flag)
          {
            hdf_process(filename,process_band);
          }
        }
      }
      outputImage.close();
    }

    GDALClose(poDataset);

    return status;
  }

  bool hdf_query(const char *filename, double& no_data_value, bool& no_data_value_flag, Image& nullImage)
  {
    GDALDataset *poDataset;

    poDataset = (GDALDataset *)GDALOpen(filename, GA_ReadOnly);
    if (!poDataset)
    {
      cout << "Could not open input_HDF file name = " << filename << endl;
      return false;
    }

//    cout << "For subdataset = " << filename << endl;

    nullImage.set_gdal_flag(true);
    int value;
    value = poDataset->GetRasterXSize();
    nullImage.set_ncols(value);
    value = poDataset->GetRasterYSize();
    nullImage.set_nrows(value);
//    cout << "(X, Y) = (" << nullImage.get_ncols() << ", " << nullImage.get_nrows() << ")" << endl;

    int count = poDataset->GetRasterCount();
    if (count != 1)
    {
      cout << "ERROR: Unexpected count value" << endl;
    }

    GDALRasterBand *rb = poDataset->GetRasterBand(1);

    int hasfill = false;
    double fill = rb->GetNoDataValue(&hasfill);
    if (hasfill)
    {
      no_data_value = fill;
      no_data_value_flag = true;
    }
    else
      no_data_value_flag = false;

    GDALDataType dtype = rb->GetRasterDataType();
//    cout << "Data type = " << GDALGetDataTypeName(dtype) << endl;
    nullImage.set_data_type(dtype);

    string projection_type;
    if (poDataset->GetProjectionRef())
    {
      projection_type = poDataset->GetProjectionRef();
      nullImage.set_projection_type(projection_type);
    }

    double geotransform[6];
    if (poDataset->GetGeoTransform(geotransform) == CE_None)
      nullImage.set_geotransform(geotransform);

    GDALClose(poDataset);

    return true;
  }

  void hdf_process(const char *filename, const int& band)
  {
    GDALDataset *poDataset, *outputDataset;

    poDataset = (GDALDataset *)GDALOpen(filename, GA_ReadOnly);
    outputDataset = outputImage.get_imageDataset();

    cout << endl << "Processing subdataset = " << filename << endl;

    int xsize = poDataset->GetRasterXSize();
    int ysize = poDataset->GetRasterYSize();
    GDALRasterBand *rb = poDataset->GetRasterBand(1);
    GDALRasterBand *wb = outputDataset->GetRasterBand(band+1);

    float *buffer = new float[xsize * ysize];
    rb->RasterIO(GF_Read, 0, 0, xsize, ysize, buffer, xsize, ysize, GDT_Float32, 0, 0);
    wb->RasterIO(GF_Write, 0, 0, xsize, ysize, buffer, xsize, ysize, GDT_Float32, 0, 0);

    GDALClose(poDataset);

    delete [] buffer;

    return;
  }


} // CommonTilton
